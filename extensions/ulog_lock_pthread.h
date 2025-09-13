// *************************************************************************
// microlog extension: pthread mutex lock helper
// *************************************************************************
/**
 * @file ulog_lock_pthread.h
 * @brief Convenience API for enabling microlog locking with a POSIX
 *        pthread mutex.
 *
 * Compile this file only on platforms providing <pthread.h>. The helpers do
 * not allocate; they either use an already initialized mutex or initialize
 * one for you.
 */
#pragma once
#include <pthread.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Enable locking with an existing, already initialized pthread mutex.
 * @param mtx Pointer to initialized mutex.
 * @return ULOG_STATUS_OK on success, ULOG_STATUS_INVALID_ARGUMENT if mtx is NULL.
 */
ulog_status ulog_lock_pthread_enable(pthread_mutex_t *mtx);

/**
 * @brief Initialize a pthread mutex (default attributes) and enable logging lock.
 * @param mtx Pointer to (uninitialized) mutex storage.
 * @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR if initialization fails,
 *         ULOG_STATUS_INVALID_ARGUMENT if mtx is NULL.
 */
ulog_status ulog_lock_pthread_init_enable(pthread_mutex_t *mtx);

#ifdef __cplusplus
}
#endif
