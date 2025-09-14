// *************************************************************************
//
// microlog extension: ThreadX mutex lock helper
//
// Usage:
//    #include "ulog_lock_threadx.h"
//    ...
//    TX_MUTEX m;
//    tx_mutex_create(&m, "ulog", TX_INHERIT);
//    ulog_lock_threadx_enable(&m);
//    ulog_info("ThreadX lock active");
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
 * @brief Disable logging lock (clears lock function). Does not delete mutex.
 */
ulog_status ulog_lock_threadx_disable(void);

#ifdef __cplusplus
}
#endif
