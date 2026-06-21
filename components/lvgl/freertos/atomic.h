/**
 * Shim to suppress -Wunused-function warnings from FreeRTOS atomic.h.
 * The header defines static inline functions not used in every translation
 * unit. This wrapper suppresses those warnings and includes the real header
 * via #include_next.
 */

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#include_next <freertos/atomic.h>
#pragma GCC diagnostic pop
