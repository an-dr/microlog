// *************************************************************************
//
// Copyright (c) 2024 Andrei Gramakov. All rights reserved.
//
// This file is licensed under the terms of the MIT license.
// For a copy, see: https://opensource.org/licenses/MIT
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#pragma once

typedef void(*ilog_func)(const char *fmt, ...);

typedef struct {
    ilog_func trace;
    ilog_func debug;
    ilog_func info;
    ilog_func warn;
    ilog_func error;
    ilog_func fatal;
} ilog_t;

extern ilog_t ilog;
extern ilog_t log_funcs;

// inline void ilog_trace(const char *fmt, ...);
// inline void ilog_debug(const char *fmt, ...);
// inline void ilog_info(const char *fmt, ...);
// inline void ilog_warn(const char *fmt, ...);
// inline void ilog_error(const char *fmt, ...);
// inline void ilog_fatal(const char *fmt, ...);

// __attribute__((weak)) void ilog_trace(const char *fmt, ...){}
// __attribute__((weak)) void ilog_debug(const char *fmt, ...){}
// __attribute__((weak)) void ilog_info(const char *fmt, ...){}
// __attribute__((weak)) void ilog_warn(const char *fmt, ...){}
// __attribute__((weak)) void ilog_error(const char *fmt, ...){}
// __attribute__((weak)) void ilog_fatal(const char *fmt, ...){}

