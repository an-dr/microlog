// *************************************************************************
//
// microlog extension: Windows Critical Section lock helper
//
// Usage:
//    #include "ulog_lock_win.h"
//    ...
//    static CRITICAL_SECTION log_cs;
//    InitializeCriticalSection(&log_cs);
//    ulog_lock_win_enable(&log_cs);
//    ulog_info("Windows lock active");
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
 * @brief Disable logging lock (clears lock function). Does not delete section.
 */
ulog_status ulog_lock_win_disable(void);

#ifdef __cplusplus
}
#endif
