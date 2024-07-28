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

/// @brief Returns the string representation of the level
const char *ulog_get_level_string(int level);

/// @brief  Sets the lock function and user data
/// @param fn - Lock function
/// @param udata - User data
void ulog_set_lock(ulog_LockFn fn, void *udata);

/// @brief Sets the debug level
/// @param level - Debug level
void ulog_set_level(int level);

/// @brief Sets the quiet mode
/// @param enable - Quiet mode
void ulog_set_quiet(bool enable);

/// @brief Logs the message
/// @param level - Debug level
/// @param file - File name
/// @param line - Line number
/// @param fmt - Format string
/// @param ... - Format arguments
void ulog_log(int level, const char *file, int line, const char *fmt, ...);

#if ULOG_EXTRA_DESTINATIONS > 0
/// @brief Adds a callback
/// @param fn - Callback function
/// @param udata - User data
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_add_callback(ulog_LogFn fn, void *udata, int level);

/// @brief Add file callback
/// @param fp - File pointer
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_add_fp(FILE *fp, int level);
#endif

#ifdef ULOG_HAVE_TIME
/// @brief Returns the time. Must be implemented by the user
/// @return Time as a dec number
long unsigned ulog_get_time(void);
#endif

#ifdef __cplusplus
}
#endif

#endif
