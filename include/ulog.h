/**
 * Copyright (c) 2020 rxi
 * Additions Copyright (c) 2024 Andrei Gramakov - mail@agramakov.me
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `ulog.c` for details.
 */

#ifndef ULOG_H
#define ULOG_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#if HAVE_ULOG_CFG_H
#include "ulog_cfg.h"
#endif

typedef struct {
    va_list ap;
    const char *fmt;
    const char *file;
    void *udata;
    int line;
    int level;
} ulog_Event;

typedef void (*ulog_LogFn)(ulog_Event *ev);
typedef void (*ulog_LockFn)(bool lock, void *udata);

enum { LOG_TRACE,
       LOG_DEBUG,
       LOG_INFO,
       LOG_WARN,
       LOG_ERROR,
       LOG_FATAL };

#define log_trace(...) ulog_log(LOG_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#define log_debug(...) ulog_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) ulog_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) ulog_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) ulog_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) ulog_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif

const char *ulog_level_string(int level);
void ulog_set_lock(ulog_LockFn fn, void *udata);
void ulog_set_level(int level);
void ulog_set_quiet(bool enable);
int ulog_add_callback(ulog_LogFn fn, void *udata, int level);
int ulog_add_fp(FILE *fp, int level);
void ulog_log(int level, const char *file, int line, const char *fmt, ...);

#ifdef ULOG_HAVE_TIME
long unsigned ulog_get_time(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
