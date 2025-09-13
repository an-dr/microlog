// *************************************************************************
//
// microlog extension: FreeRTOS mutex lock helper
//
// Requires defining ULOG_LOCK_WITH_FREERTOS and linking against FreeRTOS.
// Provides functions to either create an internal mutex or use an existing
// one.
//
// Usage:
//   #define ULOG_LOCK_WITH_FREERTOS
//   #include "extensions/ulog_lock_freertos.h"
//   ulog_lock_freertos_create_and_enable();
//   ulog_info("FreeRTOS lock active");
//
// Or reuse existing mutex:
//   SemaphoreHandle_t m = xSemaphoreCreateMutex();
//   ulog_lock_freertos_enable(m);
//
// *************************************************************************
#pragma once
#include "FreeRTOS.h"
#include "semphr.h"
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enable locking using an existing FreeRTOS mutex.
 * @param mutex Valid recursive or normal mutex handle.
 * @return ULOG_STATUS_OK or ULOG_STATUS_INVALID_ARGUMENT.
 */
ulog_status ulog_lock_freertos_enable(SemaphoreHandle_t mutex);

/**
 * @brief Create (if needed) an internal mutex with xSemaphoreCreateMutex and enable it.
 * @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR on allocation failure.
 */
ulog_status ulog_lock_freertos_create_and_enable(void);

/**
 * @brief Get handle of internally created mutex (NULL if not created).
 */
SemaphoreHandle_t ulog_lock_freertos_get_handle(void);

/**
 * @brief Disable logging lock (clears lock function). Keeps mutex allocated.
 */
ulog_status ulog_lock_freertos_disable(void);

/**
 * @brief Delete internally created mutex (if any) and disable locking.
 *        Safe only if mutex was created by create_enable().
 */
ulog_status ulog_lock_freertos_delete_and_disable(void);

#ifdef __cplusplus
}
#endif
