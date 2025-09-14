// *************************************************************************
//
// microlog extension: CMSIS-RTOS2 mutex lock helper
//
// Usage:
//    #include "ulog_lock_cmsis.h"
//    ...
//    osMutexId_t mutex_id = osMutexNew(NULL);
//    ulog_lock_cmsis_enable(mutex_id);
//    ulog_info("CMSIS lock active");
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
 * @brief Disable logging lock (clears lock function). Keeps mutex.
 */
ulog_status ulog_lock_cmsis_disable(void);

#ifdef __cplusplus
}
#endif
