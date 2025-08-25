// *************************************************************************
//
// ulog v@{ULOG_VERSION}@  - A simple customizable logging library.
// https://github.com/an-dr/microlog
//
// *************************************************************************
//
// Original implementation by rxi: https://github.com/rxi
// Modified by Andrei Gramakov: https://agramakov.me, mail@agramakov.me
//
// Copyright (c) 2024 Andrei Gramakov. All rights reserved.
//
// This file is licensed under the terms of the MIT license.
// For a copy, see: https://opensource.org/licenses/MIT
//
// *************************************************************************

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

/* ============================================================================
   Core Functionality
============================================================================ */
// clang-format off
typedef enum  ulog_level_enum { 
    ULOG_LEVEL_TRACE = 0,
    ULOG_LEVEL_DEBUG,
    ULOG_LEVEL_INFO,
    ULOG_LEVEL_WARN,
    ULOG_LEVEL_ERROR,
    ULOG_LEVEL_FATAL
} ulog_level;
       
#define log_trace(...) ulog_log(ULOG_LEVEL_TRACE, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_debug(...) ulog_log(ULOG_LEVEL_DEBUG, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_info(...) ulog_log(ULOG_LEVEL_INFO, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_warn(...) ulog_log(ULOG_LEVEL_WARN, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_error(...) ulog_log(ULOG_LEVEL_ERROR, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_fatal(...) ulog_log(ULOG_LEVEL_FATAL, __FILE__, __LINE__, NULL, __VA_ARGS__)
// clang-format on

/// @brief Event structure (opaque)
typedef struct ulog_event ulog_event;

typedef void (*ulog_log_fn)(ulog_event *ev, void *arg);

/// @brief Returns the string representation of the level
const char *ulog_level_to_string(int level);

/// @brief Sets the debug level
/// @param level - Debug level
void ulog_level_set(int level);

/// @brief Disables logging to stdout
/// @param enable - Quiet mode
void ulog_set_quiet(bool enable); // TODO: ulog_disable_stdout/enable_stdout?

/// @brief Write event content to a buffer as a log message
/// @param ev - Event
/// @param out_buf - Output buffer
/// @param out_buf_size - Output buffer size
/// @return 0 if success, -1 if failed
int ulog_event_to_cstr(ulog_event *ev, char *out, size_t out_size);

/// @brief Logs the message
/// @param level - Debug level
/// @param file - File name
/// @param line - Line number
/// @param topic - Topic name
/// @param message - Message format string
/// @param ... - Format arguments
void ulog_log(ulog_level level, const char *file, int line, const char *topic,
              const char *message, ...);

/* ============================================================================
   Core Functionality: Thread Safety
============================================================================ */

typedef void (*ulog_lock_fn)(bool lock, void *lock_arg);

/// @brief  Sets the lock function and user data
/// @param function - Lock function
/// @param lock_arg - User data
void ulog_lock_set(ulog_lock_fn function, void *lock_arg);

/* ============================================================================
   Feature: Runtime Config
============================================================================ */

void ulog_configure_color(bool enabled);
void ulog_configure_prefix(bool enabled);
void ulog_configure_file_string(bool enabled);
void ulog_configure_time(bool enabled);
void ulog_configure_levels(bool use_short_levels);
void ulog_configure_topics(bool enabled);

/* ============================================================================
   Feature: Custom Prefix
============================================================================ */

typedef void (*ulog_prefix_fn)(ulog_event *ev, char *prefix,
                               size_t prefix_size);

/// @brief Sets the prefix function
/// @param function - Prefix function
void ulog_prefix_set_fn(ulog_prefix_fn function);

/* ============================================================================
   Feature: User Callbacks
============================================================================ */

/// @brief Adds a callback
/// @param function - Callback function
/// @param arg - Optional argument that will be added to the event to be
///              processed by the callback
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_user_callback_add(ulog_log_fn function, void *arg, int level);

/// @brief Add file callback
/// @param fp - File pointer
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_user_callback_add_fp(FILE *fp, int level);

/* ============================================================================
   Feature: Log Topics
============================================================================ */

// clang-format off
#define TOPIC_NOT_FOUND 0x7FFFFFFF
#define logt_trace(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_TRACE, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_debug(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_DEBUG, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_info(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_INFO, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_warn(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_WARN, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_error(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_ERROR, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_fatal(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_FATAL, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
// clang-format on

/// @brief Adds a topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @param enable
/// @return Topic ID if success, -1 if failed
int ulog_topic_add(const char *topic_name, bool enable);

/// @brief Sets the debug level of a given topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @param level - Debug level
/// @return 0 if success, -1 if failed
int ulog_topic_set_level(const char *topic_name, ulog_level level);

/// @brief Gets the topic ID
/// @param topic_name - Topic name. "" and NULL are not valid
/// @return  Topic ID if success, -1 if failed, TOPIC_NOT_FOUND if not found
int ulog_topic_get_id(const char *topic_name);

/// @brief Enables the topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @return 0 if success, -1 if failed
int ulog_topic_enable(const char *topic_name);

/// @brief Disables the topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @return 0 if success, -1 if failed
int ulog_topic_disable(const char *topic_name);

/// @brief Enables all topics
int ulog_topic_enable_all(void);

/// @brief Disables all topics
int ulog_topic_disable_all(void);

#ifdef __cplusplus
}
#endif
