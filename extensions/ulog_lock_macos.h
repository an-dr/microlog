// *************************************************************************
//
// microlog extension: macOS os_unfair_lock helper
//
// Available on modern macOS/iOS. Falls back to normal pthread extension if
// you prefer; this file provides direct unfair lock usage which is cheaper
// than a mutex for short critical sections.
//
// Usage:
//   #include "extensions/ulog_lock_macos.h"
//   static os_unfair_lock log_lock = OS_UNFAIR_LOCK_INIT;
//   ulog_lock_macos_unfair_enable(&log_lock);
//
// *************************************************************************
#pragma once
#include <os/lock.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enable locking with an existing os_unfair_lock.
 * @param lock Pointer to initialized unfair lock.
 */
ulog_status ulog_lock_macos_unfair_enable(os_unfair_lock *lock);

/**
 * @brief Initialize (OS_UNFAIR_LOCK_INIT) and enable an unfair lock.
 * @param lock Pointer to lock storage.
 */
ulog_status ulog_lock_macos_unfair_init_enable(os_unfair_lock *lock);

/**
 * @brief Disable logging lock (clears lock function). Does not touch lock.
 */
ulog_status ulog_lock_macos_unfair_disable(void);

// Unfair locks have no explicit destroy API; supply a semantic alias for
// consistency with other platforms.
/**
 * @brief Disable logging lock (alias; symmetry with *_destroy_disable APIs).
 */
static inline ulog_status ulog_lock_macos_unfair_destroy_disable(os_unfair_lock *lock) {
	(void)lock;  // nothing to destroy
	return ulog_lock_macos_unfair_disable();
}

#ifdef __cplusplus
}
#endif
