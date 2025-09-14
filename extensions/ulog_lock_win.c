// *************************************************************************
// microlog extension: Windows Critical Section lock helper (implementation)
// *************************************************************************

#include "ulog_lock_win.h"

// Internal lock function ----------------------------------------------------
/** @brief Internal lock adapter for Windows Critical Section. */
static ulog_status win_lock_fn(bool lock, void *arg) {
    if (arg == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    CRITICAL_SECTION *cs = (CRITICAL_SECTION *)arg;
    if (lock) {
        EnterCriticalSection(cs);
    } else {
        LeaveCriticalSection(cs);
    }
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_win_enable */
ulog_status ulog_lock_win_enable(CRITICAL_SECTION *cs) {
    if (cs == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    ulog_lock_set_fn(win_lock_fn, cs);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_win_disable */
ulog_status ulog_lock_win_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}
