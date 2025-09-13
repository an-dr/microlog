// *************************************************************************
//
// microlog extension: Windows Critical Section lock helper
//
// Usage:
//   #include "extensions/ulog_lock_win.h"
//   static CRITICAL_SECTION log_cs;
//   ulog_lock_win_init_enable(&log_cs);
//   ulog_info("Windows lock active");
//
// Or if you already initialized it elsewhere:
//   ulog_lock_win_enable(&log_cs);
//
// *************************************************************************
#pragma once
#include <windows.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Enable locking with an existing initialized CRITICAL_SECTION.
 * @param cs Pointer to initialized critical section.
 * @return ULOG_STATUS_OK or ULOG_STATUS_INVALID_ARGUMENT.
 */
ulog_status ulog_lock_win_enable(CRITICAL_SECTION *cs);

/**
 * @brief Initialize a CRITICAL_SECTION (InitializeCriticalSection) and enable locking.
 * @param cs Pointer to critical section storage.
 * @return ULOG_STATUS_OK on success, ULOG_STATUS_INVALID_ARGUMENT if cs is NULL.
 */
ulog_status ulog_lock_win_init_enable(CRITICAL_SECTION *cs);
#ifdef __cplusplus
}
#endif
