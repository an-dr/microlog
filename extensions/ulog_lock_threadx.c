// *************************************************************************
// microlog extension: ThreadX mutex lock helper (implementation)
// *************************************************************************
#include "ulog_lock_threadx.h"
#include <stddef.h>
#include "ulog.h"

/** @brief Internal ThreadX mutex adapter. */
static ulog_status threadx_lock_fn(bool lock, void *arg) {
    if (arg == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    TX_MUTEX *m = (TX_MUTEX *)arg;
    UINT rc     = TX_SUCCESS;
    if (lock) {
        rc = tx_mutex_get(m, TX_WAIT_FOREVER);
    } else {
        rc = tx_mutex_put(m);
    }
    if (rc != TX_SUCCESS) {
        return ULOG_STATUS_ERROR;
    }
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_threadx_enable */
ulog_status ulog_lock_threadx_enable(TX_MUTEX *mtx) {
    if (mtx == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    ulog_lock_set_fn(threadx_lock_fn, mtx);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_threadx_disable */
ulog_status ulog_lock_threadx_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}
