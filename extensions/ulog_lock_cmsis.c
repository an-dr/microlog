// *************************************************************************
// microlog extension: CMSIS-RTOS2 mutex lock helper (implementation)
// *************************************************************************

#include "ulog_lock_cmsis.h"
#include <stddef.h>
#include "cmsis_os2.h"
#include "ulog.h"

/** @brief Internal CMSIS-RTOS2 mutex adapter. */
static ulog_status cmsis_lock_fn(bool lock, void *arg) {
    osMutexId_t id = (osMutexId_t)arg;
    if (id == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    osStatus_t st;
    if (lock) {
        st = osMutexAcquire(id, osWaitForever);
    } else {
        st = osMutexRelease(id);
    }
    if (st != osOK) {
        return ULOG_STATUS_ERROR;
    }
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_cmsis_enable */
ulog_status ulog_lock_cmsis_enable(osMutexId_t mutex_id) {
    if (mutex_id == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    ulog_lock_set_fn(cmsis_lock_fn, (void *)mutex_id);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_cmsis_disable */
ulog_status ulog_lock_cmsis_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}
