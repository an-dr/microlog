// *************************************************************************
//
// microlog extension: pthread mutex lock helper
//
// Provides convenience functions to wire a POSIX pthread mutex into microlog's
// locking callback. Mirrors the enable/disable pattern used by other
// extensions (e.g. `ulog_syslog_enable/disable`).
//
// Lifetime: You own the mutex. Use `ulog_lock_pthread_init_enable()` to
// initialize + enable, then optionally `ulog_lock_pthread_destroy_disable()`
// when finished *after* ensuring no other threads are inside logging.
//
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

/**
 * @brief Disable logging lock (clears lock function). Does not destroy mutex.
 * @return ULOG_STATUS_OK always.
 */
ulog_status ulog_lock_pthread_disable(void);

/**
 * @brief Destroy a pthread mutex and disable logging lock.
 * @param mtx Pointer to initialized (and currently unlocked) mutex.
 * @return ULOG_STATUS_OK on success, ULOG_STATUS_INVALID_ARGUMENT if mtx is NULL,
 *         ULOG_STATUS_ERROR if pthread_mutex_destroy fails.
 */
ulog_status ulog_lock_pthread_destroy_disable(pthread_mutex_t *mtx);

#ifdef __cplusplus
}
#endif
