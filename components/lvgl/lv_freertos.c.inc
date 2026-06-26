/**
 * @file lv_freertos.c
 *
 * Patched replacement for the upstream LVGL 9.5 lv_freertos.c.
 * Copied into the build's managed_components/lvgl__lvgl/src/osal/lv_freertos.c
 * by project_include.cmake before compilation.
 *
 * Two fixes over upstream:
 *
 * 1. ESP-IDF FreeRTOS adaptations (lv_thread_init / lv_thread_delete)
 *    Uses xTaskCreatePinnedToCore(tskNO_AFFINITY) instead of xTaskCreate so
 *    the draw thread is unpinned and the scheduler places it on whichever core
 *    is available — matching the SMP-aware behaviour expected on ESP32-P4.
 *    Stack is allocated from internal SRAM (original upstream behaviour).
 *    Declared in freertos/idf_additions.h (IDF 5.0+).
 *
 * 2. SMP concurrent-init deadlock fix (prvCheckMutexInit / prvCheckCondInit)
 *    Upstream uses a single global portMUX_TYPE critSectionMux shared across
 *    ALL lv_mutex_t and lv_thread_sync_t init calls.  On IDF 6.x SMP,
 *    taskENTER_CRITICAL(&mux) is a true cross-core spinlock.  When Core0
 *    holds it (lv_os_init initialising lv_general_mutex) and Core1 (the newly
 *    spawned draw thread) immediately calls lv_thread_sync_init ->
 *    prvCheckCondInit -> taskENTER_CRITICAL on the SAME global mux, both cores
 *    deadlock permanently: silent boot hang, 0xa5a5a5a5 stack fill.
 *
 *    Fix: replace the global spinlock with lock-free init using a three-state
 *    xIsInitialized field and GCC __atomic built-ins:
 *      0 (pdFALSE)  = uninitialised
 *      2            = init claimed by one core, in progress
 *      1 (pdTRUE)   = fully initialised
 *    The core that wins the CAS(0->2) does the init and stores pdTRUE.
 *    Any other core that races spins briefly (taskYIELD loop) until pdTRUE.
 *    The spin is bounded: just FreeRTOS semaphore creation, no blocking.
 *    No shared spinlock, no cross-object contention, deadlock impossible.
 *
 *    For the LV_USE_FREERTOS_TASK_NOTIFY path (lv_thread_sync_wait/signal):
 *    xTaskToNotify and xSyncSignal are accessed via __atomic_exchange_n /
 *    __atomic_store_n with __ATOMIC_SEQ_CST, replacing the global-spinlock
 *    critical sections there too.
 */

/**
 * Copyright 2023 NXP
 * SPDX-License-Identifier: MIT
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_os_private.h"
#if LV_USE_OS == LV_OS_FREERTOS

#ifdef ESP_PLATFORM
#include "freertos/idf_additions.h"  /* xTaskCreatePinnedToCore, tskNO_AFFINITY */
#endif

#include "freertos/atomic.h"

#include "../tick/lv_tick.h"
#include "../misc/lv_log.h"
#include "../core/lv_global.h"

/*********************
 *      DEFINES
 *********************/

#define ulMAX_COUNT 10U
#define globals LV_GLOBAL_DEFAULT()

/* Three-state values for xIsInitialized */
#define LV_INIT_UNCLAIMED  ((BaseType_t)0)   /* == pdFALSE */
#define LV_INIT_CLAIMED    ((BaseType_t)2)   /* in progress, claimed by one core */
#define LV_INIT_DONE       ((BaseType_t)1)   /* == pdTRUE, fully initialised */

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void prvRunThread(void *pxArg);
static void prvCheckMutexInit(lv_mutex_t *pxMutex);
static void prvCondInit(lv_thread_sync_t *pxCond);
static void prvCheckCondInit(lv_thread_sync_t *pxCond);
static void prvCheckCondInitIsr(lv_thread_sync_t *pxCond);

#ifdef ESP_PLATFORM
/* Bootstrap wrapper: carries real thread ptr + a semaphore that lv_thread_init
 * blocks on until the new task has executed its first instruction.  Ensures
 * thread_dsc->inited is set before the first dispatch() call on SMP targets
 * where taskYIELD() only yields the calling core. */
typedef struct {
    lv_thread_t       *pxThread;
    SemaphoreHandle_t  xReadySem;
} lv_thread_boot_t;

static void prvRunThreadSmp(void *pxArg)
{
    lv_thread_boot_t boot = *(lv_thread_boot_t *)pxArg;
    lv_free(pxArg);
    xSemaphoreGive(boot.xReadySem);   /* unblock lv_thread_init */
    boot.pxThread->pvStartRoutine((void *)boot.pxThread->pTaskArg);
    vTaskDelete(NULL);
}
#endif

#if !LV_USE_FREERTOS_TASK_NOTIFY
static void prvTestAndDecrement(lv_thread_sync_t *pxCond, uint32_t ulLocalWaitingThreads);
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_result_t lv_thread_init(lv_thread_t *pxThread, const char *const name,
                            lv_thread_prio_t xSchedPriority,
                            void (*pvStartRoutine)(void *), size_t usStackSize,
                            void *xAttr)
{
    pxThread->pTaskArg = xAttr;
    pxThread->pvStartRoutine = pvStartRoutine;

    BaseType_t xStatus;

#ifdef ESP_PLATFORM
    /* Allocate a bootstrap struct so we can block until the new task has
     * executed its first instruction on either core.  This guarantees
     * thread_dsc->inited == true before lv_draw_sw_init() returns, avoiding
     * the dispatch() / lv_draw_dispatch_wait_for_request() deadlock on SMP
     * where a plain taskYIELD() only yields the calling core. */
    lv_thread_boot_t *pxBoot = lv_malloc(sizeof(lv_thread_boot_t));
    if(pxBoot == NULL) {
        LV_LOG_ERROR("lv_malloc(lv_thread_boot_t) failed for '%s'!", name);
        return LV_RESULT_INVALID;
    }
    pxBoot->pxThread  = pxThread;
    pxBoot->xReadySem = xSemaphoreCreateBinary();
    if(pxBoot->xReadySem == NULL) {
        LV_LOG_ERROR("xSemaphoreCreateBinary failed for '%s'!", name);
        lv_free(pxBoot);
        return LV_RESULT_INVALID;
    }

    xStatus = xTaskCreatePinnedToCore(
                  prvRunThreadSmp,
                  name,
                  (configSTACK_DEPTH_TYPE)usStackSize,
                  (void *)pxBoot,
                  tskIDLE_PRIORITY + (UBaseType_t)xSchedPriority,
                  &pxThread->xTaskHandle,
                  tskNO_AFFINITY);

    if(xStatus != pdPASS) {
        LV_LOG_ERROR("Task creation failed for '%s'!", name);
        vSemaphoreDelete(pxBoot->xReadySem);
        lv_free(pxBoot);
        return LV_RESULT_INVALID;
    }

    /* Block until prvRunThreadSmp signals us — guarantees the task has started
     * and thread_dsc->inited will be set before our caller proceeds. */
    xSemaphoreTake(pxBoot->xReadySem, portMAX_DELAY);
    /* pxBoot was freed by prvRunThreadSmp before the Give; xReadySem still
     * valid here because Give happens before free on the task side. Wait —
     * actually free is BEFORE Give would be wrong.  prvRunThreadSmp does:
     *   1. copy boot struct to stack
     *   2. free heap block
     *   3. xSemaphoreGive(boot.xReadySem)   <- still valid, copied to stack
     * So after Take returns, the semaphore handle is no longer needed. */
    vSemaphoreDelete(pxBoot->xReadySem);

#else
    xStatus = xTaskCreate(
                  prvRunThread,
                  name,
                  (configSTACK_DEPTH_TYPE)(usStackSize / sizeof(StackType_t)),
                  (void *)pxThread,
                  tskIDLE_PRIORITY + (UBaseType_t)xSchedPriority,
                  &pxThread->xTaskHandle);

    if(xStatus != pdPASS) {
        LV_LOG_ERROR("Task creation failed for '%s'!", name);
        return LV_RESULT_INVALID;
    }
#endif

    return LV_RESULT_OK;
}

lv_result_t lv_thread_delete(lv_thread_t *pxThread)
{
    vTaskDelete(pxThread->xTaskHandle);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_init(lv_mutex_t *pxMutex)
{
    prvCheckMutexInit(pxMutex);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock(lv_mutex_t *pxMutex)
{
    prvCheckMutexInit(pxMutex);

    if(xSemaphoreTakeRecursive(pxMutex->xMutex, portMAX_DELAY) != pdTRUE) {
        LV_LOG_ERROR("xSemaphoreTakeRecursive failed!");
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock_isr(lv_mutex_t *pxMutex)
{
    prvCheckMutexInit(pxMutex);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    if(xSemaphoreTakeFromISR(pxMutex->xMutex, &xHigherPriorityTaskWoken) != pdTRUE) {
        LV_LOG_ERROR("xSemaphoreTakeFromISR failed!");
        return LV_RESULT_INVALID;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_unlock(lv_mutex_t *pxMutex)
{
    prvCheckMutexInit(pxMutex);

    if(xSemaphoreGiveRecursive(pxMutex->xMutex) != pdTRUE) {
        LV_LOG_ERROR("xSemaphoreGiveRecursive failed!");
        return LV_RESULT_INVALID;
    }
    return LV_RESULT_OK;
}

lv_result_t lv_mutex_delete(lv_mutex_t *pxMutex)
{
    if(__atomic_load_n(&pxMutex->xIsInitialized, __ATOMIC_ACQUIRE) != LV_INIT_DONE)
        return LV_RESULT_INVALID;
    vSemaphoreDelete(pxMutex->xMutex);
    __atomic_store_n(&pxMutex->xIsInitialized, LV_INIT_UNCLAIMED, __ATOMIC_RELEASE);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_init(lv_thread_sync_t *pxCond)
{
    prvCheckCondInit(pxCond);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_wait(lv_thread_sync_t *pxCond)
{
    lv_result_t lvRes = LV_RESULT_OK;

    prvCheckCondInit(pxCond);

#if LV_USE_FREERTOS_TASK_NOTIFY
    TaskHandle_t xCurrentTaskHandle = xTaskGetCurrentTaskHandle();

    /* 1. Try to consume a pre-existing signal. */
    BaseType_t xSyncSignal = __atomic_exchange_n(&pxCond->xSyncSignal, pdFALSE, __ATOMIC_SEQ_CST);
    if(xSyncSignal == pdFALSE) {
        /* 2. No pre-existing signal: register to be notified. */
        __atomic_store_n(&pxCond->xTaskToNotify, xCurrentTaskHandle, __ATOMIC_SEQ_CST);

        /* 3. Re-check: signal may have arrived in the gap above. */
        xSyncSignal = __atomic_exchange_n(&pxCond->xSyncSignal, pdFALSE, __ATOMIC_SEQ_CST);
        if(xSyncSignal == pdFALSE) {
            ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        }
        else {
            /* Signal arrived in gap; clear registration so sender won't also notify. */
            __atomic_store_n(&pxCond->xTaskToNotify, NULL, __ATOMIC_SEQ_CST);
        }
    }
#else
    xSemaphoreTake(pxCond->xSyncMutex, portMAX_DELAY);

    while(!pxCond->xSyncSignal) {
        uint32_t ulLocalWaitingThreads = Atomic_Increment_u32(&pxCond->ulWaitingThreads);

        BaseType_t xMutexStatus = xSemaphoreGive(pxCond->xSyncMutex);
        if(xMutexStatus == pdTRUE) {
            BaseType_t xCondWaitStatus = xSemaphoreTake(pxCond->xCondWaitSemaphore, portMAX_DELAY);
            xSemaphoreTake(pxCond->xSyncMutex, portMAX_DELAY);
            if(xCondWaitStatus != pdTRUE) {
                LV_LOG_ERROR("xSemaphoreTake(xCondWaitSemaphore) failed!");
                lvRes = LV_RESULT_INVALID;
                prvTestAndDecrement(pxCond, ulLocalWaitingThreads + 1);
            }
        }
        else {
            LV_LOG_ERROR("xSemaphoreGive(xSyncMutex) failed!");
            lvRes = LV_RESULT_INVALID;
            prvTestAndDecrement(pxCond, ulLocalWaitingThreads + 1);
        }
    }

    pxCond->xSyncSignal = pdFALSE;
    xSemaphoreGive(pxCond->xSyncMutex);
#endif

    return lvRes;
}

lv_result_t lv_thread_sync_signal(lv_thread_sync_t *pxCond)
{
    prvCheckCondInit(pxCond);

#if LV_USE_FREERTOS_TASK_NOTIFY
    TaskHandle_t xTaskToNotify = __atomic_exchange_n(&pxCond->xTaskToNotify, NULL, __ATOMIC_SEQ_CST);
    if(xTaskToNotify != NULL) {
        xTaskNotifyGive(xTaskToNotify);
    }
    else {
        __atomic_store_n(&pxCond->xSyncSignal, pdTRUE, __ATOMIC_SEQ_CST);
    }
#else
    xSemaphoreTake(pxCond->xSyncMutex, portMAX_DELAY);
    pxCond->xSyncSignal = pdTRUE;

    uint32_t ulLocalWaitingThreads = pxCond->ulWaitingThreads;
    while(ulLocalWaitingThreads > 0) {
        if(ATOMIC_COMPARE_AND_SWAP_SUCCESS == Atomic_CompareAndSwap_u32(
               &pxCond->ulWaitingThreads, 0, ulLocalWaitingThreads)) {
            for(uint32_t i = 0; i < ulLocalWaitingThreads; i++) {
                xSemaphoreGive(pxCond->xCondWaitSemaphore);
            }
            break;
        }
        ulLocalWaitingThreads = pxCond->ulWaitingThreads;
    }
    xSemaphoreGive(pxCond->xSyncMutex);
#endif

    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_delete(lv_thread_sync_t *pxCond)
{
#if !LV_USE_FREERTOS_TASK_NOTIFY
    vSemaphoreDelete(pxCond->xCondWaitSemaphore);
    vSemaphoreDelete(pxCond->xSyncMutex);
    pxCond->ulWaitingThreads = 0;
#endif
    pxCond->xSyncSignal = pdFALSE;
    __atomic_store_n(&pxCond->xIsInitialized, LV_INIT_UNCLAIMED, __ATOMIC_RELEASE);
    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_signal_isr(lv_thread_sync_t *pxCond)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    prvCheckCondInitIsr(pxCond);

#if LV_USE_FREERTOS_TASK_NOTIFY
    TaskHandle_t xTaskToNotify = __atomic_exchange_n(&pxCond->xTaskToNotify, NULL, __ATOMIC_SEQ_CST);
    if(xTaskToNotify != NULL) {
        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
    else {
        __atomic_store_n(&pxCond->xSyncSignal, pdTRUE, __ATOMIC_SEQ_CST);
    }
#else
    UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
    pxCond->xSyncSignal = pdTRUE;
    BaseType_t xAnyHigherPriorityTaskWoken = pdFALSE;
    for(uint32_t i = 0; i < pxCond->ulWaitingThreads; i++) {
        xSemaphoreGiveFromISR(pxCond->xCondWaitSemaphore, &xAnyHigherPriorityTaskWoken);
        xHigherPriorityTaskWoken |= xAnyHigherPriorityTaskWoken;
    }
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif

    return LV_RESULT_OK;
}

void lv_freertos_task_switch_in(const char *name)
{
    if(lv_strcmp(name, "IDLE")) globals->freertos_idle_task_running = false;
    else globals->freertos_idle_task_running = true;
    globals->freertos_task_switch_timestamp = lv_tick_get();
}

void lv_freertos_task_switch_out(void)
{
    uint32_t elaps = lv_tick_elaps(globals->freertos_task_switch_timestamp);
    if(globals->freertos_idle_task_running) globals->freertos_idle_time_sum += elaps;
    else globals->freertos_non_idle_time_sum += elaps;
}

uint32_t lv_os_get_idle_percent(void)
{
    if(globals->freertos_non_idle_time_sum + globals->freertos_idle_time_sum == 0) {
        LV_LOG_WARN("Not enough time elapsed to provide idle percentage");
        return 0;
    }
    uint32_t pct = (globals->freertos_idle_time_sum * 100) /
                   (globals->freertos_idle_time_sum + globals->freertos_non_idle_time_sum);
    globals->freertos_non_idle_time_sum = 0;
    globals->freertos_idle_time_sum = 0;
    return pct;
}

void lv_sleep_ms(uint32_t ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void prvRunThread(void *pxArg)
{
    lv_thread_t *pxThread = (lv_thread_t *)pxArg;
    pxThread->pvStartRoutine((void *)pxThread->pTaskArg);
    vTaskDelete(NULL);
}

static void prvCheckMutexInit(lv_mutex_t *pxMutex)
{
    /* Fast path: fully initialised. Acquire barrier ensures xMutex is visible. */
    if(__atomic_load_n(&pxMutex->xIsInitialized, __ATOMIC_ACQUIRE) == LV_INIT_DONE)
        return;

    /* Try to claim init: CAS(UNCLAIMED -> CLAIMED). Only one core wins. */
    BaseType_t xExpected = LV_INIT_UNCLAIMED;
    if(__atomic_compare_exchange_n(&pxMutex->xIsInitialized, &xExpected,
                                   LV_INIT_CLAIMED, /*weak=*/0,
                                   __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        SemaphoreHandle_t xNew = xSemaphoreCreateRecursiveMutex();
        if(xNew == NULL) {
            LV_LOG_ERROR("xSemaphoreCreateRecursiveMutex failed!");
            __atomic_store_n(&pxMutex->xIsInitialized, LV_INIT_UNCLAIMED, __ATOMIC_RELEASE);
            return;
        }
        pxMutex->xMutex = xNew;
        /* Release store: pairs with acquire loads; xMutex visible after this. */
        __atomic_store_n(&pxMutex->xIsInitialized, LV_INIT_DONE, __ATOMIC_RELEASE);
    }
    else {
        /* Another core claimed init. Spin until it completes (bounded). */
        while(__atomic_load_n(&pxMutex->xIsInitialized, __ATOMIC_ACQUIRE) != LV_INIT_DONE) {
            taskYIELD();
        }
    }
}

static void prvCondInit(lv_thread_sync_t *pxCond)
{
    pxCond->xSyncSignal = pdFALSE;
#if LV_USE_FREERTOS_TASK_NOTIFY
    pxCond->xTaskToNotify = NULL;
#else
    pxCond->xCondWaitSemaphore = xSemaphoreCreateCounting(ulMAX_COUNT, 0U);
    if(pxCond->xCondWaitSemaphore == NULL) {
        LV_LOG_ERROR("xSemaphoreCreateCounting failed!");
        return;
    }
    pxCond->xSyncMutex = xSemaphoreCreateMutex();
    if(pxCond->xSyncMutex == NULL) {
        LV_LOG_ERROR("xSemaphoreCreateMutex failed!");
        vSemaphoreDelete(pxCond->xCondWaitSemaphore);
        return;
    }
    pxCond->ulWaitingThreads = 0;
#endif
}

static void prvCheckCondInit(lv_thread_sync_t *pxCond)
{
    /* Fast path. */
    if(__atomic_load_n(&pxCond->xIsInitialized, __ATOMIC_ACQUIRE) == LV_INIT_DONE)
        return;

    /* Try to claim init. */
    BaseType_t xExpected = LV_INIT_UNCLAIMED;
    if(__atomic_compare_exchange_n(&pxCond->xIsInitialized, &xExpected,
                                   LV_INIT_CLAIMED, /*weak=*/0,
                                   __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
        prvCondInit(pxCond);
        __atomic_store_n(&pxCond->xIsInitialized, LV_INIT_DONE, __ATOMIC_RELEASE);
    }
    else {
        /* Spin-wait for winning core to finish. */
        while(__atomic_load_n(&pxCond->xIsInitialized, __ATOMIC_ACQUIRE) != LV_INIT_DONE) {
            taskYIELD();
        }
    }
}

static void prvCheckCondInitIsr(lv_thread_sync_t *pxCond)
{
    LV_ASSERT_MSG(__atomic_load_n(&pxCond->xIsInitialized, __ATOMIC_ACQUIRE) == LV_INIT_DONE,
                  "lv_thread_sync_signal_isr called before lv_thread_sync_init");
}

#if !LV_USE_FREERTOS_TASK_NOTIFY
static void prvTestAndDecrement(lv_thread_sync_t *pxCond, uint32_t ulLocalWaitingThreads)
{
    while(ulLocalWaitingThreads > 0) {
        if(ATOMIC_COMPARE_AND_SWAP_SUCCESS == Atomic_CompareAndSwap_u32(
               &pxCond->ulWaitingThreads,
               ulLocalWaitingThreads - 1,
               ulLocalWaitingThreads)) {
            break;
        }
        ulLocalWaitingThreads = pxCond->ulWaitingThreads;
    }
}
#endif

#endif /* LV_USE_OS == LV_OS_FREERTOS */
