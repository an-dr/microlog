// *************************************************************************
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#include "ulog.h"
#include "object.h"

#ifndef ULOG_DEFAULT_LOG_LEVEL
#define ULOG_DEFAULT_LOG_LEVEL LOG_TRACE
#endif

/// @brief Main logger object himself
ulog_t ulog = {
    .lock_function   = NULL,
    .lock_arg        = NULL,
    .level           = ULOG_DEFAULT_LOG_LEVEL,
    .quiet           = false,
    .callback_stdout = {0},

#if FEATURE_EXTRA_OUTPUTS
    .callbacks = {{0}},
#endif

#if FEATURE_CUSTOM_PREFIX
    .update_prefix_function = NULL,
    .custom_prefix          = {0},
#endif

};
