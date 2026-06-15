/**
 * @file lv_freertos.c  (patched — copied into src/osal/ by lvgl_build_filter.py)
 *
 * Patch: lv_thread_init() allocates the FreeRTOS task stack from PSRAM on
 * ESP_PLATFORM via xTaskCreateWithCaps(), falling back to internal SRAM when
 * PSRAM is unavailable.  All other behaviour is identical to upstream LVGL 9.5.
 *
 * Why: youkorr's lvgl_9.5 fork sets LV_USE_OS=LV_OS_FREERTOS on all ESP32
 * targets so that ThorVG/Lottie mutexes work correctly.  LVGL then spawns
 * draw threads via xTaskCreate() which allocates stacks from internal SRAM.
 * With LV_DRAW_THREAD_STACK_SIZE=(48*1024) this exhausts internal SRAM on
 * ESP32-P4 before the main app task can start, causing:
 *   assert failed: esp_startup_start_app app_startup.c:86 (res == pdTRUE)
 *
 * Fix: xTaskCreateWithCaps(MALLOC_CAP_SPIRAM) routes the stack to PSRAM.
 * ThorVG render threads are not hot-path stack users; PSRAM latency is fine.
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

#include "atomic.h"

#include "../tick/lv_tick.h"
#include "../misc/lv_log.h"
#include "../core/lv_global.h"

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "freertos/idf_additions.h"
#endif

/*********************
 *      DEFINES
 *********************/

#define ulMAX_COUNT 10U

#define globals LV_GLOBAL_DEFAULT()

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void prvRunThread(void * pxArg);

static void prvMutexInit(lv_mutex_t * pxMutex);

static void prvCheckMutexInit(lv_mutex_t * pxMutex);

static void prvCondInit(lv_thread_sync_t * pxCond);

static void prvCheckCondInit(lv_thread_sync_t * pxCond);

static void prvCheckCondInitIsr(lv_thread_sync_t * pxCond);

#if !LV_USE_FREERTOS_TASK_NOTIFY
static void prvTestAndDecrement(lv_thread_sync_t * pxCond,
                                uint32_t ulLocalWaitingThreads);
#endif

/**********************
 *  STATIC VARIABLES
 **********************/

#ifdef ESP_PLATFORM
    static portMUX_TYPE critSectionMux = portMUX_INITIALIZER_UNLOCKED;
#endif

/**********************
 *      MACROS
 **********************/

#ifdef ESP_PLATFORM
    #define _enter_critical()   taskENTER_CRITICAL(&critSectionMux);
    #define _exit_critical()    taskEXIT_CRITICAL(&critSectionMux);
    #define _enter_critical_isr() taskENTER_CRITICAL_FROM_ISR();
    #define _exit_critical_isr(x) taskEXIT_CRITICAL_FROM_ISR(x);
#else
    #define _enter_critical()   taskENTER_CRITICAL();
    #define _exit_critical()    taskEXIT_CRITICAL();
    #define _enter_critical_isr() taskENTER_CRITICAL_FROM_ISR();
    #define _exit_critical_isr(x) taskEXIT_CRITICAL_FROM_ISR(x);
#endif

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_result_t lv_thread_init(lv_thread_t * pxThread,  const char * const name,
                           lv_thread_prio_t xSchedPriority,
                           void (*pvStartRoutine)(void *), size_t usStackSize,
                           void * xAttr)
{
    pxThread->pTaskArg = xAttr;
    pxThread->pvStartRoutine = pvStartRoutine;

    BaseType_t xTaskCreateStatus;

#ifdef ESP_PLATFORM
    /* Try PSRAM first so the draw-thread stack doesn't consume internal SRAM.
     * xTaskCreateWithCaps() is an ESP-IDF extension (freertos/idf_additions.h). */
    xTaskCreateStatus = xTaskCreateWithCaps(
                            prvRunThread,
                            name,
                            (configSTACK_DEPTH_TYPE)(usStackSize / sizeof(StackType_t)),
                            (void *)pxThread,
                            tskIDLE_PRIORITY + xSchedPriority,
                            &pxThread->xTaskHandle,
                            MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);

    if(xTaskCreateStatus != pdPASS) {
        LV_LOG_WARN("xTaskCreateWithCaps(SPIRAM) failed, retrying in SRAM");
        xTaskCreateStatus = xTaskCreate(
                                prvRunThread,
                                name,
                                (configSTACK_DEPTH_TYPE)(usStackSize / sizeof(StackType_t)),
                                (void *)pxThread,
                                tskIDLE_PRIORITY + xSchedPriority,
                                &pxThread->xTaskHandle);
    }
#else
    xTaskCreateStatus = xTaskCreate(
                            prvRunThread,
                            name,
                            (configSTACK_DEPTH_TYPE)(usStackSize / sizeof(StackType_t)),
                            (void *)pxThread,
                            tskIDLE_PRIORITY + xSchedPriority,
                            &pxThread->xTaskHandle);
#endif

    if(xTaskCreateStatus != pdPASS) {
        LV_LOG_ERROR("xTaskCreate failed!");
        return LV_RESULT_INVALID;
    }

    return LV_RESULT_OK;
}

lv_result_t lv_thread_delete(lv_thread_t * pxThread)
{
    vTaskDelete(pxThread->xTaskHandle);

    return LV_RESULT_OK;
}

lv_result_t lv_mutex_init(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);

    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);

    BaseType_t xMutexTakeStatus = xSemaphoreTakeRecursive(pxMutex->xMutex, portMAX_DELAY);
    if(xMutexTakeStatus != pdTRUE) {
        LV_LOG_ERROR("xSemaphoreTake failed!");
        return LV_RESULT_INVALID;
    }

    return LV_RESULT_OK;
}

lv_result_t lv_mutex_lock_isr(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);

    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    BaseType_t xMutexTakeStatus = xSemaphoreTakeFromISR(pxMutex->xMutex, &xHigherPriorityTaskWoken);
    if(xMutexTakeStatus != pdTRUE) {
        LV_LOG_ERROR("xSemaphoreTake failed!");
        return LV_RESULT_INVALID;
    }

    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);

    return LV_RESULT_OK;
}

lv_result_t lv_mutex_unlock(lv_mutex_t * pxMutex)
{
    prvCheckMutexInit(pxMutex);

    BaseType_t xMutexGiveStatus = xSemaphoreGiveRecursive(pxMutex->xMutex);
    if(xMutexGiveStatus != pdTRUE) {
        LV_LOG_ERROR("xSemaphoreGive failed!");
        return LV_RESULT_INVALID;
    }

    return LV_RESULT_OK;
}

lv_result_t lv_mutex_delete(lv_mutex_t * pxMutex)
{
    if(pxMutex->xIsInitialized == pdFALSE)
        return LV_RESULT_INVALID;
    vSemaphoreDelete(pxMutex->xMutex);
    pxMutex->xIsInitialized = pdFALSE;

    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_init(lv_thread_sync_t * pxCond)
{
    prvCheckCondInit(pxCond);

    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_wait(lv_thread_sync_t * pxCond)
{
    lv_result_t lvRes = LV_RESULT_OK;

    prvCheckCondInit(pxCond);

#if LV_USE_FREERTOS_TASK_NOTIFY
    TaskHandle_t xCurrentTaskHandle = xTaskGetCurrentTaskHandle();

    _enter_critical();
    BaseType_t xSyncSygnal = pxCond->xSyncSignal;
    pxCond->xSyncSignal = pdFALSE;
    if(xSyncSygnal == pdFALSE) {
        pxCond->xTaskToNotify = xCurrentTaskHandle;
    }
    _exit_critical();

    if(xSyncSygnal == pdFALSE) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    }
#else
    uint32_t ulLocalWaitingThreads;

    xSemaphoreTake(pxCond->xSyncMutex, portMAX_DELAY);

    while(!pxCond->xSyncSignal) {
        ulLocalWaitingThreads = Atomic_Increment_u32(&pxCond->ulWaitingThreads);

        BaseType_t xMutexStatus = xSemaphoreGive(pxCond->xSyncMutex);

        if(xMutexStatus == pdTRUE) {
            BaseType_t xCondWaitStatus = xSemaphoreTake(
                                             pxCond->xCondWaitSemaphore,
                                             portMAX_DELAY);

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

lv_result_t lv_thread_sync_signal(lv_thread_sync_t * pxCond)
{
    prvCheckCondInit(pxCond);

#if LV_USE_FREERTOS_TASK_NOTIFY
    _enter_critical();
    TaskHandle_t xTaskToNotify = pxCond->xTaskToNotify;
    pxCond->xTaskToNotify = NULL;
    if(xTaskToNotify == NULL) {
        pxCond->xSyncSignal = pdTRUE;
    }
    _exit_critical();

    if(xTaskToNotify != NULL) {
        xTaskNotifyGive(xTaskToNotify);
    }
#else
    xSemaphoreTake(pxCond->xSyncMutex, portMAX_DELAY);

    pxCond->xSyncSignal = pdTRUE;

    uint32_t ulLocalWaitingThreads = pxCond->ulWaitingThreads;

    while(ulLocalWaitingThreads > 0) {
        if(ATOMIC_COMPARE_AND_SWAP_SUCCESS == Atomic_CompareAndSwap_u32(
               &pxCond->ulWaitingThreads,
               0,
               ulLocalWaitingThreads)) {
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

lv_result_t lv_thread_sync_delete(lv_thread_sync_t * pxCond)
{
#if !LV_USE_FREERTOS_TASK_NOTIFY
    vSemaphoreDelete(pxCond->xCondWaitSemaphore);
    vSemaphoreDelete(pxCond->xSyncMutex);
    pxCond->ulWaitingThreads = 0;
#endif
    pxCond->xSyncSignal = pdFALSE;
    pxCond->xIsInitialized = pdFALSE;

    return LV_RESULT_OK;
}

lv_result_t lv_thread_sync_signal_isr(lv_thread_sync_t * pxCond)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    prvCheckCondInitIsr(pxCond);

#if LV_USE_FREERTOS_TASK_NOTIFY
    uint32_t mask = _enter_critical_isr();
    TaskHandle_t xTaskToNotify = pxCond->xTaskToNotify;
    pxCond->xTaskToNotify = NULL;
    if(xTaskToNotify == NULL) {
        pxCond->xSyncSignal = pdTRUE;
    }
    _exit_critical_isr(mask);

    if(xTaskToNotify != NULL) {
        vTaskNotifyGiveFromISR(xTaskToNotify, &xHigherPriorityTaskWoken);
        portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    }
#else
    uint32_t mask = _enter_critical_isr();

    pxCond->xSyncSignal = pdTRUE;
    BaseType_t xAnyHigherPriorityTaskWoken = pdFALSE;

    for(uint32_t i = 0; i < pxCond->ulWaitingThreads; i++) {
        xSemaphoreGiveFromISR(pxCond->xCondWaitSemaphore, &xAnyHigherPriorityTaskWoken);
        xHigherPriorityTaskWoken |= xAnyHigherPriorityTaskWoken;
    }

    _exit_critical_isr(mask);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
#endif

    return LV_RESULT_OK;
}


void lv_freertos_task_switch_in(const char * name)
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

    uint32_t pct = (globals->freertos_idle_time_sum * 100) / (globals->freertos_idle_time_sum +
                                                              globals->freertos_non_idle_time_sum);

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

static void prvRunThread(void * pxArg)
{
    lv_thread_t * pxThread = (lv_thread_t *)pxArg;

    pxThread->pvStartRoutine((void *)pxThread->pTaskArg);

    vTaskDelete(NULL);
}

static void prvMutexInit(lv_mutex_t * pxMutex)
{
    pxMutex->xMutex = xSemaphoreCreateRecursiveMutex();

    if(pxMutex->xMutex == NULL) {
        LV_LOG_ERROR("xSemaphoreCreateMutex failed!");
        return;
    }

    pxMutex->xIsInitialized = pdTRUE;
}

static void prvCheckMutexInit(lv_mutex_t * pxMutex)
{
    if(pxMutex->xIsInitialized == pdFALSE) {
        _enter_critical();

        if(pxMutex->xIsInitialized == pdFALSE) {
            prvMutexInit(pxMutex);
        }

        _exit_critical();
    }
}

static void prvCondInit(lv_thread_sync_t * pxCond)
{
    pxCond->xIsInitialized = pdTRUE;
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

static void prvCheckCondInit(lv_thread_sync_t * pxCond)
{
    if(pxCond->xIsInitialized == pdFALSE) {
        _enter_critical();

        if(pxCond->xIsInitialized == pdFALSE) {
            prvCondInit(pxCond);
        }

        _exit_critical();
    }
}

static void prvCheckCondInitIsr(lv_thread_sync_t * pxCond)
{
    if(pxCond->xIsInitialized == pdFALSE) {
        uint32_t mask = _enter_critical_isr();

        if(pxCond->xIsInitialized == pdFALSE) {
            prvCondInit(pxCond);
        }

        _exit_critical_isr(mask);
    }
}

#if !LV_USE_FREERTOS_TASK_NOTIFY
static void prvTestAndDecrement(lv_thread_sync_t * pxCond,
                                uint32_t ulLocalWaitingThreads)
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

#endif /*LV_USE_OS == LV_OS_FREERTOS*/
