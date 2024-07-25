// *************************************************************************
//
// Copyright (c) $CURRENT_YEAR Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#include "ilog.h"
#include <stdarg.h>

// #define ILOG_GET_ARGS va_list args; va_start(args , fmt)
// #define ILOG_FINALIZE va_end(args)

ilog_t log_funcs = { 0 };


static void ilog_run_if_exists(ilog_func func, const char *fmt, ...) {
    if (func) {
        va_list args;
        va_start(args, fmt);
        func(fmt, args);
        va_end(args);
    }
}

void ilog_trace(const char *fmt, ...) { ilog_run_if_exists(log_funcs.trace, fmt); }
void ilog_debug(const char *fmt, ...) { ilog_run_if_exists(log_funcs.debug, fmt); }
void ilog_info(const char *fmt, ...) { ilog_run_if_exists(log_funcs.info, fmt); }
void ilog_warn(const char *fmt, ...) { ilog_run_if_exists(log_funcs.warn, fmt); }
void ilog_error(const char *fmt, ...) { ilog_run_if_exists(log_funcs.error, fmt); }
void ilog_fatal(const char *fmt, ...) { ilog_run_if_exists(log_funcs.fatal, fmt); }

ilog_t ilog = { 
    .trace = ilog_trace,
    .debug = ilog_debug,
    .info = ilog_info,
    .warn = ilog_warn,
    .error = ilog_error,
    .fatal = ilog_fatal
 };
