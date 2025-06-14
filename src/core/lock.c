// *************************************************************************
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#include "object.h"
#include "lock.h"

/// @brief Locks if the function provided
void lock(void) {
    if (ulog.lock_function) {
        ulog.lock_function(true, ulog.lock_arg);
    }
}

void unlock(void) {
    if (ulog.lock_function) {
        ulog.lock_function(false, ulog.lock_arg);
    }
}

/// @brief  Sets the lock function and user data
void ulog_set_lock(ulog_LockFn function, void *lock_arg) {
    ulog.lock_function = function;
    ulog.lock_arg      = lock_arg;
}
