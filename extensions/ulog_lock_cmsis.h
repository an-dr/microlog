// *************************************************************************
//
// microlog extension: CMSIS-RTOS2 mutex lock helper
//
// Enable by defining ULOG_LOCK_WITH_CMSIS and compiling this file.
// Uses cmsis_os2.h API (RTOS2). You can provide existing osMutexId_t or let
// helper create one.
//
// Usage:
//   #include "extensions/ulog_lock_cmsis.h"
//   ulog_lock_cmsis_create_enable();
//   ulog_info("CMSIS lock active");
//
// *************************************************************************
#pragma once
#include "cmsis_os2.h"
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enable locking with an existing CMSIS-RTOS2 mutex id.
 * @param mutex_id Handle returned by osMutexNew or equivalent.
 */
ulog_status ulog_lock_cmsis_enable(osMutexId_t mutex_id);

/**
 * @brief Create (once) an internal CMSIS mutex and enable it.
 */
ulog_status ulog_lock_cmsis_create_enable(void);

/**
 * @brief Get internally created mutex id (NULL if not created).
 */
osMutexId_t ulog_lock_cmsis_get_mutex(void);

/**
 * @brief Disable logging lock (clears lock function). Keeps mutex.
 */
ulog_status ulog_lock_cmsis_disable(void);

/**
 * @brief Delete internally created mutex (if owned) and disable locking.
 */
ulog_status ulog_lock_cmsis_delete_disable(void);

#ifdef __cplusplus
}
#endif
