/**
 * @file lvgl_fps_benchmark_wrapper.cpp
 * ESPHome compiles .cpp files from the component dir; this wrapper pulls
 * the .c sampler source so it gets linked.
 */

#include "esphome/core/defines.h"

#ifdef USE_LVGL_FPS_BENCHMARK

extern "C" {
#include "lvgl_fps_benchmark.c"
}

#endif
