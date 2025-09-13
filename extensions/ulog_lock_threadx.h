// *************************************************************************
//
// microlog extension: ThreadX mutex lock helper
//
// Enable by defining ULOG_LOCK_WITH_THREADX and compiling this file.
// Provides helpers to use an existing TX_MUTEX or create one.
//
// Usage:
//   #define ULOG_LOCK_WITH_THREADX
//   #include "extensions/ulog_lock_threadx.h"
//   // Create + enable
//   static TX_MUTEX log_mutex;               // zeroed BSS is fine
//   ulog_lock_threadx_create_enable(&log_mutex);
//   ulog_info("ThreadX lock active");
//
//   // Or if you already created it:
//   ulog_lock_threadx_enable(&log_mutex);
//
// *************************************************************************
#pragma once
#include "tx_api.h"
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enable locking with an existing ThreadX mutex.
 * @param mtx Pointer to initialized TX_MUTEX.
 * @return ULOG_STATUS_OK / ULOG_STATUS_INVALID_ARGUMENT.
 */
ulog_status ulog_lock_threadx_enable(TX_MUTEX *mtx);

/**
 * @brief Create a ThreadX mutex (tx_mutex_create) and enable locking.
 * @param mtx Pointer to TX_MUTEX storage.
 * @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR on create failure.
 */
ulog_status ulog_lock_threadx_create_enable(TX_MUTEX *mtx);

#ifdef __cplusplus
}
#endif
