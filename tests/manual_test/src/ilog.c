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


static int ilog_run_if_exists(ilog_func func, const char *fmt, ...) {
    int res = 0;
    if (func) {
        va_list args;
        va_start(args, fmt);
        res = func(fmt, args);
        va_end(args);
    }
    return res;
}

int ilog_trace(const char *fmt, ...) {return  ilog_run_if_exists(log_funcs.trace, fmt); }
int ilog_debug(const char *fmt, ...) {return  ilog_run_if_exists(log_funcs.debug, fmt); }
int ilog_info(const char *fmt, ...) { return ilog_run_if_exists(log_funcs.info, fmt); }
int ilog_warn(const char *fmt, ...) { return ilog_run_if_exists(log_funcs.warn, fmt); }
int ilog_error(const char *fmt, ...) {return  ilog_run_if_exists(log_funcs.error, fmt); }
int ilog_fatal(const char *fmt, ...) {return  ilog_run_if_exists(log_funcs.fatal, fmt); }

ilog_t ilog = { 
    .trace = ilog_trace,
    .debug = ilog_debug,
    .info = ilog_info,
    .warn = ilog_warn,
    .error = ilog_error,
    .fatal = ilog_fatal
 };
