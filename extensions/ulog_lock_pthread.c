// *************************************************************************
//
// microlog extension: pthread mutex lock helper (implementation)
//
// This file provides convenience functions to enable microlog locking using a
// POSIX pthread mutex. The implementation mirrors coding style used in
// `ulog.c`: explicit braces, early returns, and clear variable naming.
//
// *************************************************************************

#include "ulog_lock_pthread.h"

// --------------------------------------------------------------------------
// Internal lock function
// --------------------------------------------------------------------------
/**
 * @brief Internal adapter; wraps pthread mutex operations in ulog_lock_fn signature.
 */
static ulog_status pthread_lock_fn(bool lock, void *arg) {
    if (arg == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    pthread_mutex_t *mtx = (pthread_mutex_t *)arg;
    int rc               = -1;

    if (lock) {
        rc = pthread_mutex_lock(mtx);
    } else {
        rc = pthread_mutex_unlock(mtx);
    }

    if (rc != 0) {
        return ULOG_STATUS_ERROR;
    }
    return ULOG_STATUS_OK;
}

// --------------------------------------------------------------------------
// Public API
// --------------------------------------------------------------------------

/**
 * @copydoc ulog_lock_pthread_enable
 */
ulog_status ulog_lock_pthread_enable(pthread_mutex_t *mtx) {
    if (mtx == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    ulog_lock_set_fn(pthread_lock_fn, mtx);
    return ULOG_STATUS_OK;
}

/**
 * @copydoc ulog_lock_pthread_init_enable
 */
ulog_status ulog_lock_pthread_init_enable(pthread_mutex_t *mtx) {
    if (mtx == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    if (pthread_mutex_init(mtx, NULL) != 0) {
        return ULOG_STATUS_ERROR;
    }
    return ulog_lock_pthread_enable(mtx);
}

/** @copydoc ulog_lock_pthread_disable */
ulog_status ulog_lock_pthread_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_pthread_destroy_disable */
ulog_status ulog_lock_pthread_destroy_disable(pthread_mutex_t *mtx) {
    if (mtx == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    int rc = pthread_mutex_destroy(mtx);
    ulog_lock_pthread_disable();
    if (rc != 0) {
        return ULOG_STATUS_ERROR;
    }
    return ULOG_STATUS_OK;
}
