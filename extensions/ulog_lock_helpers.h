// *************************************************************************
// microlog aggregated lock helpers header
// *************************************************************************
// Include this header to get declarations for all available lock helper APIs.
// Only include the platform sections relevant to your build system (guarded
// by platform-specific macros). This does NOT automatically include platform
// native headers unless their own helper header does so.
// *************************************************************************
#pragma once
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

// Generic disable (alias for ulog_lock_set_fn(NULL,NULL))
static inline void ulog_lock_disable(void) { ulog_lock_set_fn(NULL, NULL); }

// POSIX pthread
#if defined(__unix__) || defined(__APPLE__)
#include "ulog_lock_pthread.h"
#endif

// Windows
#if defined(_WIN32)
#include "ulog_lock_win.h"
#endif

// FreeRTOS
#ifdef ULOG_LOCK_WITH_FREERTOS
#include "ulog_lock_freertos.h"
#endif

// ThreadX
#ifdef ULOG_LOCK_WITH_THREADX
#include "ulog_lock_threadx.h"
#endif

// Zephyr
#ifdef ULOG_LOCK_WITH_ZEPHYR
#include "ulog_lock_zephyr.h"
#endif

// CMSIS RTOS2
#ifdef ULOG_LOCK_WITH_CMSIS
#include "ulog_lock_cmsis.h"
#endif

// macOS unfair lock
#if defined(__APPLE__)
#include "ulog_lock_macos.h"
#endif

#ifdef __cplusplus
}
#endif
