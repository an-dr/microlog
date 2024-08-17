/**
 * Copyright (c) 2020 rxi
 * Additions Copyright (c) 2024 Andrei Gramakov - mail@agramakov.me
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `ulog.c` for details.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

/* ============================================================================
   Configuration options
===============================================================================

| Feature               | Feature Default | Controlling Options               |
|-----------------------|-----------------|-----------------------------------|
| FEATURE_TIME          | OFF             | ULOG_HAVE_TIME                    |
| FEATURE_COLOR         | ON              | ULOG_NO_COLOR                     |
| FEATURE_CUSTOM_PREFIX | OFF             | ULOG_CUSTOM_PREFIX_SIZE           |
| FEATURE_FILE_STRING   | ON              | ULOG_HIDE_FILE_STRING             |
| FEATURE_SHORT_LEVELS  | OFF             | ULOG_SHORT_LEVEL_STRINGS          |
| FEATURE_EMOJI_LEVELS  | OFF             | ULOG_USE_EMOJI                    |
| FEATURE_STDOUT        | ON              | ULOG_NO_STDOUT                    |
| FEATURE_EXTRA_DESTS   | OFF             | ULOG_EXTRA_DESTINATIONS           |

============================================================================ */

#ifdef ULOG_HAVE_TIME
#define FEATURE_TIME 1
#endif

#ifndef ULOG_NO_COLOR
#define FEATURE_COLOR 1
#endif

#if ULOG_CUSTOM_PREFIX_SIZE > 0
#define FEATURE_CUSTOM_PREFIX 1
#define CFG_CUSTOM_PREFIX_SIZE ULOG_CUSTOM_PREFIX_SIZE
#endif

#ifndef ULOG_HIDE_FILE_STRING
#define FEATURE_FILE_STRING 1
#endif

#ifndef ULOG_SHORT_LEVEL_STRINGS
#define FEATURE_SHORT_LEVELS 1
#endif

#ifndef ULOG_NO_STDOUT
#define FEATURE_STDOUT 1
#endif

#if ULOG_EXTRA_DESTINATIONS > 0
#define FEATURE_EXTRA_DESTS 1
#define CFG_EXTRA_DESTS ULOG_EXTRA_DESTINATIONS
#endif

#ifndef ULOG_USE_EMOJI

#ifdef ULOG_SHORT_LEVEL_STRINGS
#warning "ULOG_USE_EMOJI overrides ULOG_SHORT_LEVEL_STRINGS!"
#undef FEATURE_SHORT_LEVELS
#endif

#define FEATURE_EMOJI_LEVELS 1
#endif // ULOG_USE_EMOJI

/* ============================================================================
   Core Functionality
============================================================================ */

#if FEATURE_TIME
#include <time.h>
#endif

enum { LOG_TRACE,
       LOG_DEBUG,
       LOG_INFO,
       LOG_WARN,
       LOG_ERROR,
       LOG_FATAL };

#define log_debug(...) ulog_log(LOG_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define log_info(...) ulog_log(LOG_INFO, __FILE__, __LINE__, __VA_ARGS__)
#define log_warn(...) ulog_log(LOG_WARN, __FILE__, __LINE__, __VA_ARGS__)
#define log_error(...) ulog_log(LOG_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#define log_fatal(...) ulog_log(LOG_FATAL, __FILE__, __LINE__, __VA_ARGS__)


/// @brief Event structure
typedef struct {
    const char *message;          // Message format string
    va_list message_format_args;  // Format arguments

#if FEATURE_TIME
    struct tm *time;
#endif

    const char *file;  // Event file name
    int line;          // Event line number
    int level;         // Event debug level
} ulog_Event;

typedef void (*ulog_LogFn)(ulog_Event *ev, void *arg);

/// @brief Returns the string representation of the level
const char *ulog_get_level_string(int level);

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
/// @param message - Message format string
/// @param ... - Format arguments
void ulog_log(int level, const char *file, int line, const char *message, ...);

/* ============================================================================
   Core Functionality: Thread Safety
============================================================================ */

typedef void (*ulog_LockFn)(bool lock, void *lock_arg);

/// @brief  Sets the lock function and user data
/// @param function - Lock function
/// @param lock_arg - User data
void ulog_set_lock(ulog_LockFn function, void *lock_arg);

/* ============================================================================
   Feature: Custom Prefix
============================================================================ */
#if FEATURE_CUSTOM_PREFIX

typedef void (*ulog_PrefixFn)(ulog_Event *ev, char *prefix, size_t prefix_size);

/// @brief Sets the prefix function
/// @param function - Prefix function
void ulog_set_prefix_fn(ulog_PrefixFn function);

#endif  // FEATURE_CUSTOM_PREFIX

/* ============================================================================
   Feature: Extra Destinations
============================================================================ */
#if FEATURE_EXTRA_DESTS

/// @brief Adds a callback
/// @param function - Callback function
/// @param arg - Optional argument that will be added to the event to be
///              processed by the callback
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_add_callback(ulog_LogFn function, void *arg, int level);

/// @brief Add file callback
/// @param fp - File pointer
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_add_fp(FILE *fp, int level);

#endif  // FEATURE_EXTRA_DESTS

#ifdef __cplusplus
}
#endif
