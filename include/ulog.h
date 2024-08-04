/**
 * Copyright (c) 2020 rxi
 * Additions Copyright (c) 2024 Andrei Gramakov - mail@agramakov.me
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `ulog.c` for details.
 */

#ifndef ULOG_H
#define ULOG_H

// =====================
// Default configuration
// =====================

#ifndef ULOG_EXTRA_DESTINATIONS
#define ULOG_EXTRA_DESTINATIONS 0
#endif

#ifndef ULOG_CUSTOM_PREFIX_SIZE
#define ULOG_CUSTOM_PREFIX_SIZE 0
#endif

#if !defined(ULOG_HAVE_TIME) || ULOG_HAVE_TIME == 0
#undef ULOG_HAVE_TIME 
#endif

#if !defined(ULOG_NO_COLOR) || ULOG_NO_COLOR == 0
#undef ULOG_NO_COLOR
#endif

#if !defined(ULOG_CUSTOM_PREFIX_SIZE)
#define ULOG_CUSTOM_PREFIX_SIZE 0
#endif

#if !defined(ULOG_HIDE_FILE_STRING) || ULOG_HIDE_FILE_STRING == 0
#undef ULOG_HIDE_FILE_STRING
#endif

#if !defined(ULOG_SHORT_LEVEL_STRINGS) || ULOG_SHORT_LEVEL_STRINGS == 0
#undef ULOG_SHORT_LEVEL_STRINGS
#endif

#if !defined(ULOG_NO_STDOUT) || ULOG_NO_STDOUT == 0
#undef ULOG_NO_STDOUT
#endif

// ============
// Declarations
// ============

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef ULOG_HAVE_TIME
#include <time.h>
#endif

typedef struct {
    const char *message;          // Message format string
    va_list message_format_args;  // Format arguments

#ifdef ULOG_HAVE_TIME
    struct tm *time;
#endif // ULOG_HAVE_TIME

    const char *file;  // Event file name
    int line;          // Event line number
    int level;         // Event debug level
} ulog_Event;

typedef void (*ulog_LogFn)(ulog_Event *ev, void *arg);
typedef void (*ulog_LockFn)(bool lock, void *lock_arg);
typedef void (*ulog_PrefixFn)(ulog_Event *ev, char *prefix, size_t prefix_size);

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
/// @param function - Lock function
/// @param lock_arg - User data
void ulog_set_lock(ulog_LockFn function, void *lock_arg);

/// @brief Sets the debug level
/// @param level - Debug level
void ulog_set_level(int level);

/// @brief Sets the quiet mode
/// @param enable - Quiet mode
void ulog_set_quiet(bool enable);

#if ULOG_CUSTOM_PREFIX_SIZE > 0
/// @brief Sets the prefix function
/// @param function - Prefix function
void ulog_set_prefix_fn(ulog_PrefixFn function);
#endif

/// @brief Logs the message
/// @param level - Debug level
/// @param file - File name
/// @param line - Line number
/// @param message - Message format string
/// @param ... - Format arguments
void ulog_log(int level, const char *file, int line, const char *message, ...);

#if ULOG_EXTRA_DESTINATIONS > 0
/// @brief Adds a callback
/// @param function - Callback function
/// @param arg - Optional argument that will be added to the event to be processed by the callback
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_add_callback(ulog_LogFn function, void *arg, int level);

/// @brief Add file callback
/// @param fp - File pointer
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_add_fp(FILE *fp, int level);
#endif

#ifdef __cplusplus
}
#endif

#endif
