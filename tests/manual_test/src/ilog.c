// *************************************************************************
//
// Copyright (c) $CURRENT_YEAR Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#include "ulog.h"

#define ILOG_GET_ARGS va_list args; va_start(args , fmt)
#define ILOG_FINALIZE va_end(args)

void ilog_debug(const char *fmt, ...){
    ILOG_GET_ARGS;
    log_debug(fmt, args);
    ILOG_FINALIZE;
}

void ilog_info(const char *fmt, ...){
    ILOG_GET_ARGS;
    log_info(fmt, args);
    ILOG_FINALIZE;
}

