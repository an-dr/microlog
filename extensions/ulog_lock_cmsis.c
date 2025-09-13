// *************************************************************************
// microlog extension: CMSIS-RTOS2 mutex lock helper (implementation)
// *************************************************************************
#include "ulog_lock_cmsis.h"

static osMutexId_t cmsis_mutex = NULL;  // optional internal mutex

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

/** @copydoc ulog_lock_cmsis_create_enable */
ulog_status ulog_lock_cmsis_create_enable(void) {
    if (cmsis_mutex == NULL) {
        cmsis_mutex = osMutexNew(NULL);
        if (cmsis_mutex == NULL) {
            return ULOG_STATUS_ERROR;
        }
    }
    return ulog_lock_cmsis_enable(cmsis_mutex);
}

/** @copydoc ulog_lock_cmsis_get_mutex */
osMutexId_t ulog_lock_cmsis_get_mutex(void) {
    return cmsis_mutex;
}

/** @copydoc ulog_lock_cmsis_disable */
ulog_status ulog_lock_cmsis_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_cmsis_delete_disable */
ulog_status ulog_lock_cmsis_delete_disable(void) {
    ulog_lock_cmsis_disable();
    if (cmsis_mutex != NULL) {
        if (osMutexDelete(cmsis_mutex) != osOK) {
            return ULOG_STATUS_ERROR;
        }
        cmsis_mutex = NULL;
    }
    return ULOG_STATUS_OK;
}
