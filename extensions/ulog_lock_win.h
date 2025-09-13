// *************************************************************************
//
// microlog extension: Windows Critical Section lock helper
//
// Provides helpers to attach a `CRITICAL_SECTION` to microlog and complementary
// disable counterpart. You own the critical section object; destroy it only
// after disabling logging and ensuring no threads are inside logging calls.
//
// Usage:
//   static CRITICAL_SECTION log_cs;
//   ulog_lock_win_init_and_enable(&log_cs);
//   ... logging ...
//   ulog_lock_win_disable(); // detach when done
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
ulog_status ulog_lock_win_init_and_enable(CRITICAL_SECTION *cs);

/**
 * @brief Disable logging lock (clears lock function). Does not delete section.
 */
ulog_status ulog_lock_win_disable(void);

/**
 * @brief Delete a CRITICAL_SECTION and disable locking.
 * @param cs Pointer to initialized section.
 */
ulog_status ulog_lock_win_delete_and_disable(CRITICAL_SECTION *cs);
#ifdef __cplusplus
}
#endif
