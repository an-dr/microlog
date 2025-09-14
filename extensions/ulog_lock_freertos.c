// *************************************************************************
//
// microlog extension: FreeRTOS mutex lock helper (implementation)
// *************************************************************************

#include "ulog_lock_freertos.h"
#include "FreeRTOS.h"
#include "semphr.h"

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

/** @copydoc ulog_lock_freertos_enable */
ulog_status ulog_lock_freertos_enable(SemaphoreHandle_t mutex) {
    if (mutex == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    ulog_lock_set_fn(freertos_lock_fn, (void *)mutex);
    return ULOG_STATUS_OK;
}

/** @copydoc ulog_lock_freertos_disable */
ulog_status ulog_lock_freertos_disable(void) {
    ulog_lock_set_fn(NULL, NULL);
    return ULOG_STATUS_OK;
}
