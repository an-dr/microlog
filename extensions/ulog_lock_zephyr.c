// *************************************************************************
// microlog extension: Zephyr k_mutex lock helper (implementation)
// *************************************************************************
#include "ulog_lock_zephyr.h"

/** @brief Internal Zephyr k_mutex adapter. */
static ulog_status zephyr_lock_fn(bool lock, void *arg) {
    if (arg == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    struct k_mutex *m = (struct k_mutex *)arg;
    int rc            = 0;
    if (lock) {
        rc = k_mutex_lock(m, K_FOREVER);
    } else {
        rc = k_mutex_unlock(m);
    }
    if (rc != 0) {
        return ULOG_STATUS_ERROR;
    }
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_zephyr_enable */
ulog_status ulog_lock_zephyr_enable(struct k_mutex *mtx) {
    if (mtx == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    ulog_lock_set_fn(zephyr_lock_fn, mtx);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_zephyr_init_enable */
ulog_status ulog_lock_zephyr_init_enable(struct k_mutex *mtx) {
    if (mtx == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    k_mutex_init(mtx);
    return ulog_lock_zephyr_enable(mtx);
}
