// *************************************************************************
// microlog extension: macOS os_unfair_lock helper (implementation)
// *************************************************************************
#include "ulog_lock_macos.h"

/** @brief Internal macOS unfair lock adapter. */
static ulog_status macos_unfair_lock_fn(bool lock, void *arg) {
    if (arg == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    os_unfair_lock *l = (os_unfair_lock *)arg;
    if (lock) {
        os_unfair_lock_lock(l);
    } else {
        os_unfair_lock_unlock(l);
    }
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_macos_unfair_enable */
ulog_status ulog_lock_macos_unfair_enable(os_unfair_lock *lock) {
    if (lock == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    ulog_lock_set_fn(macos_unfair_lock_fn, lock);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_macos_unfair_init_enable */
ulog_status ulog_lock_macos_unfair_init_enable(os_unfair_lock *lock) {
    if (lock == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    *lock = OS_UNFAIR_LOCK_INIT;  // explicit init (usually already set)
    return ulog_lock_macos_unfair_enable(lock);
}

/** @copydoc ulog_lock_macos_unfair_disable */
ulog_status ulog_lock_macos_unfair_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}
