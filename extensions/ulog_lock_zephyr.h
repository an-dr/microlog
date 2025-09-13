// *************************************************************************
//
// microlog extension: Zephyr k_mutex lock helper
//
// Enable by defining ULOG_LOCK_WITH_ZEPHYR and compiling this file.
// Use existing struct k_mutex or let helper initialize it.
//
// Usage:
//   #include "extensions/ulog_lock_zephyr.h"
//   static struct k_mutex log_mutex;
//   ulog_lock_zephyr_init_enable(&log_mutex);
//   ulog_info("Zephyr lock active");
//
// *************************************************************************
#pragma once
#include <zephyr/kernel.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Enable logging lock using an existing Zephyr k_mutex.
 * @param mtx Initialized kernel mutex.
 */
ulog_status ulog_lock_zephyr_enable(struct k_mutex *mtx);

/**
 * @brief Initialize a k_mutex (k_mutex_init) and enable locking.
 * @param mtx Pointer to mutex storage.
 */
ulog_status ulog_lock_zephyr_init_enable(struct k_mutex *mtx);

#ifdef __cplusplus
}
#endif
