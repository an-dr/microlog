// *************************************************************************
//
// microlog extension: pthread mutex lock helper
//
// Usage:
//    #include "ulog_lock_pthread.h"
//    ...
//    pthread_mutex_t m;
//    pthread_mutex_init(&m, NULL);
//    ulog_lock_pthread_enable(&m);
//    ulog_info("pthread lock active");
//
// *************************************************************************

#pragma once
#include <pthread.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Enable locking with an existing, already initialized pthread mutex.
 * @param mtx Pointer to initialized mutex.
 * @return ULOG_STATUS_OK on success, ULOG_STATUS_INVALID_ARGUMENT if mtx is
 * NULL.
 */
ulog_status ulog_lock_pthread_enable(pthread_mutex_t *mtx);

/**
 * @brief Disable logging lock (clears lock function). Does not destroy mutex.
 * @return ULOG_STATUS_OK always.
 */
ulog_status ulog_lock_pthread_disable(void);

#ifdef __cplusplus
}
#endif
