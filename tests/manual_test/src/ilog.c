// *************************************************************************
//
// Copyright (c) $CURRENT_YEAR Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#include "ulog.h"

#define ILOG_IMPL_START(a) (va_list (a); va_start((a), fmt))
#define ILOG_IMPL_END(a) (va_end(a))

void ilog_debug(const char *fmt, ...){
    ILOG_IMPL_START(args);
    log_debug(fmt, args);
    va_end(args);
}

void ilog_info(const char *fmt, ...){
    va_list args; va_start(args, fmt);
    log_info(fmt, args);
    va_end(args);
}

