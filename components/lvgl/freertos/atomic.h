/**
 * Shim to suppress -Wunused-function warnings from FreeRTOS atomic.h.
 * The header defines many static inline functions that are not all used
 * in every translation unit. This wrapper suppresses those warnings and
 * then includes the real header via #include_next.
 */

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include_next <freertos/atomic.h>
#pragma GCC diagnostic pop
