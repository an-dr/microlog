// *************************************************************************
//
// microlog extension: FreeRTOS mutex lock helper (implementation)
// *************************************************************************

#include "ulog_lock_freertos.h"

static SemaphoreHandle_t freertos_mutex = NULL;  // internal optional mutex

// Internal lock function ----------------------------------------------------
/** @brief Internal FreeRTOS mutex adapter. */
static ulog_status freertos_lock_fn(bool lock, void *arg) {
    SemaphoreHandle_t m = (SemaphoreHandle_t)arg;
    if (m == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    BaseType_t ok = pdFALSE;
    if (lock) {
        ok = xSemaphoreTake(m, portMAX_DELAY);
    } else {
        ok = xSemaphoreGive(m);
    }
    if (ok != pdTRUE) {
        return ULOG_STATUS_ERROR;
    }
    return ULOG_STATUS_OK;
}

// Public API ----------------------------------------------------------------

/** @copydoc ulog_lock_freertos_enable */
ulog_status ulog_lock_freertos_enable(SemaphoreHandle_t mutex) {
    if (mutex == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    ulog_lock_set_fn(freertos_lock_fn, (void *)mutex);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_freertos_create_and_enable */
ulog_status ulog_lock_freertos_create_and_enable(void) {
    if (freertos_mutex == NULL) {
        freertos_mutex = xSemaphoreCreateMutex();
        if (freertos_mutex == NULL) {
            return ULOG_STATUS_ERROR;
        }
    }
    return ulog_lock_freertos_enable(freertos_mutex);
}

/** @copydoc ulog_lock_freertos_get_handle */
SemaphoreHandle_t ulog_lock_freertos_get_handle(void) {
    return freertos_mutex;
}

/** @copydoc ulog_lock_freertos_disable */
ulog_status ulog_lock_freertos_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_freertos_delete_and_disable */
ulog_status ulog_lock_freertos_delete_and_disable(void) {
    ulog_lock_freertos_disable();
    if (freertos_mutex != NULL) {
        vSemaphoreDelete(freertos_mutex);
        freertos_mutex = NULL;
    }
    return ULOG_STATUS_OK;
}
