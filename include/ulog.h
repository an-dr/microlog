// *************************************************************************
//
// ulog v@{ULOG_VERSION}@  - A simple customizable logging library.
// https://github.com/an-dr/microlog
//
// *************************************************************************
//
// Original implementation by rxi: https://github.com/rxi
// Modified by Andrei Gramakov: https://agramakov.me, mail@agramakov.me
// Also modified by many beautiful contributors from GitHub
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
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
   Configuration options
===============================================================================

| Feature                     | Feature Default | Compilation Options         |
|-----------------------------|-----------------|-----------------------------|
| ULOG_FEATURE_COLOR          | ON              | ULOG_NO_COLOR               |
| ULOG_FEATURE_PREFIX         | OFF             | ULOG_PREFIX_SIZE            |
| ULOG_FEATURE_EXTRA_OUTPUTS  | OFF             | ULOG_EXTRA_OUTPUTS          |
| ULOG_FEATURE_SOURCE_LOCATION| ON              | ULOG_HIDE_SOURCE_LOCATION   |
| ULOG_FEATURE_LEVELS_LONG    | ON              | ULOG_SHORT_LEVEL_STRINGS    |
| ULOG_FEATURE_LEVELS_SHORT   | OFF             | ULOG_SHORT_LEVEL_STRINGS    |
| ULOG_FEATURE_TIME           | OFF             | ULOG_HAVE_TIME              |
| ULOG_FEATURE_TOPICS         | OFF             | ULOG_TOPICS_NUM             |
| ULOG_FEATURE_DYNAMIC_CONFIG | OFF             | ULOG_DYNAMIC_CONFIG         |

============================================================================ */
// clang-format off
#ifdef ULOG_NO_COLOR
    #define ULOG_FEATURE_COLOR false
#else
    #define ULOG_FEATURE_COLOR true
#endif


#if defined(ULOG_PREFIX_SIZE) && (ULOG_PREFIX_SIZE > 0)
    #define ULOG_FEATURE_PREFIX true
#else
    #define ULOG_FEATURE_PREFIX false
#endif


#ifdef ULOG_HAVE_TIME
    #define ULOG_FEATURE_TIME true
#else
    #define ULOG_FEATURE_TIME false
#endif



#ifdef ULOG_SHORT_LEVEL_STRINGS
    #define ULOG_FEATURE_LEVELS_LONG  false
    #define ULOG_FEATURE_LEVELS_SHORT true
#else
    #define ULOG_FEATURE_LEVELS_LONG  true
    #define ULOG_FEATURE_LEVELS_SHORT false
#endif

#if defined(ULOG_EXTRA_OUTPUTS) && (ULOG_EXTRA_OUTPUTS > 0)
    #define ULOG_FEATURE_EXTRA_OUTPUTS true
#else
    #define ULOG_FEATURE_EXTRA_OUTPUTS false
#endif


#ifdef ULOG_HIDE_SOURCE_LOCATION
    #define ULOG_FEATURE_SOURCE_LOCATION false
#else
    #define ULOG_FEATURE_SOURCE_LOCATION true
#endif





#if defined(ULOG_TOPICS_NUM) && (ULOG_TOPICS_NUM >= 0 || ULOG_TOPICS_NUM == -1)
    #define ULOG_FEATURE_TOPICS true
#else
    #define ULOG_FEATURE_TOPICS false
#endif


#ifdef ULOG_DYNAMIC_CONFIG
#define ULOG_FEATURE_DYNAMIC_CONFIG true
// Undef all ULOG_FEATURE_* macros to avoid conflicts
#undef ULOG_FEATURE_COLOR
#undef ULOG_FEATURE_PREFIX
#undef ULOG_FEATURE_EXTRA_OUTPUTS
#undef ULOG_FEATURE_SOURCE_LOCATION
#undef ULOG_FEATURE_LEVELS_SHORT
#undef ULOG_FEATURE_LEVELS_LONG
#undef ULOG_FEATURE_TIME
#undef ULOG_FEATURE_TOPICS

// Configure features based on runtime config
#define ULOG_FEATURE_COLOR true
#define ULOG_FEATURE_PREFIX true
#define ULOG_PREFIX_SIZE 64
#define ULOG_FEATURE_EXTRA_OUTPUTS true
#define ULOG_EXTRA_OUTPUTS 8
#define ULOG_FEATURE_SOURCE_LOCATION true
#define ULOG_FEATURE_LEVELS_LONG true
#define ULOG_FEATURE_LEVELS_SHORT true
#define ULOG_FEATURE_TIME true
#define ULOG_FEATURE_TOPICS true
#define ULOG_TOPICS_NUM -1

#else
#define ULOG_FEATURE_DYNAMIC_CONFIG false
#endif

// clang-format on

/* ============================================================================
   Core Functionality
============================================================================ */
// clang-format off

typedef enum {
    ULOG_STATUS_OK           = 0,
    ULOG_STATUS_ERROR        = -1,
    ULOG_STATUS_BAD_ARGUMENT = -2,
} ulog_status;

typedef enum  ulog_level_enum { 
    ULOG_LEVEL_TRACE = 0,
    ULOG_LEVEL_DEBUG,
    ULOG_LEVEL_INFO,
    ULOG_LEVEL_WARN,
    ULOG_LEVEL_ERROR,
    ULOG_LEVEL_FATAL,
} ulog_level;
#define ULOG_LEVELS_TOTAL  6
       
#define ulog_trace(...) ulog_log(ULOG_LEVEL_TRACE, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define ulog_debug(...) ulog_log(ULOG_LEVEL_DEBUG, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define ulog_info(...)  ulog_log(ULOG_LEVEL_INFO, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define ulog_warn(...)  ulog_log(ULOG_LEVEL_WARN, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define ulog_error(...) ulog_log(ULOG_LEVEL_ERROR, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define ulog_fatal(...) ulog_log(ULOG_LEVEL_FATAL, __FILE__, __LINE__, NULL, __VA_ARGS__)
// Aliases
#define log_trace(...) ulog_trace(__VA_ARGS__)
#define log_debug(...) ulog_debug(__VA_ARGS__)
#define log_info(...)  ulog_info(__VA_ARGS__)
#define log_warn(...)  ulog_warn(__VA_ARGS__)
#define log_error(...) ulog_error(__VA_ARGS__)
#define log_fatal(...) ulog_fatal(__VA_ARGS__)

// clang-format on

/// @brief Event structure
typedef struct {
    const char *message;          // Message format string
    va_list message_format_args;  // Format arguments

#if ULOG_FEATURE_TOPICS
    int topic;  // TODO: ulog_topic_id
#endif

#if ULOG_FEATURE_TIME
    struct tm *time;
#endif

#if ULOG_FEATURE_SOURCE_LOCATION
    const char *file;  // Event file name
    int line;          // Event line number
#endif                 // ULOG_FEATURE_SOURCE_LOCATION

    ulog_level level;  // Event debug level
} ulog_event;

/// @brief Returns the string representation of the level
const char *ulog_level_to_string(ulog_level level);

/// @brief Write event content to a buffer as a log message
/// @param ev - Event
/// @param out_buf - Output buffer
/// @param out_buf_size - Output buffer size
/// @return ulog_status
ulog_status ulog_event_to_cstr(ulog_event *ev, char *out, size_t out_size);

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
void ulog_lock_set_fn(ulog_lock_fn function, void *lock_arg);

/* ============================================================================
   Feature: Runtime Config
============================================================================ */
#if ULOG_FEATURE_DYNAMIC_CONFIG

void ulog_color_config(bool enabled);
void ulog_prefix_config(bool enabled);
void ulog_source_location_config(bool enabled);
void ulog_time_config(bool enabled);
void ulog_level_config(bool use_short_levels);
void ulog_topic_config(bool enabled);

#endif  // ULOG_FEATURE_DYNAMIC_CONFIG

/* ============================================================================
   Feature: Prefix
============================================================================ */
#if ULOG_FEATURE_PREFIX

typedef void (*ulog_prefix_fn)(ulog_event *ev, char *prefix,
                               size_t prefix_size);

/// @brief Sets the prefix function
/// @param function - Prefix function
void ulog_prefix_set_fn(ulog_prefix_fn function);

#endif  // ULOG_FEATURE_PREFIX

/* ============================================================================
   Feature: Output
============================================================================ */

typedef int ulog_output;
enum {
    ULOG_OUTPUT_INVALID = -0x1,
    ULOG_OUTPUT_STDOUT  = 0x0,
};

typedef void (*ulog_output_callback_fn)(ulog_event *ev, void *arg);

/// @brief Sets the debug level
/// @param level - Debug level
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR if callback is not
///         added, or ULOG_STATUS_BAD_ARGUMENT
ulog_status ulog_output_level_set(ulog_output output, ulog_level level);

/// @brief Sets the debug level
/// @param level - Debug level
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR if callback is not
///         added, or ULOG_STATUS_BAD_ARGUMENT
ulog_status ulog_output_level_set_all(ulog_level level);

#if ULOG_FEATURE_EXTRA_OUTPUTS

/// @brief Adds a callback
/// @param callback - Callback function
/// @param arg - Optional argument that will be added to the event to be
///              processed by the callback
/// @param level - Debug level
/// @return ulog_output, on error - ULOG_OUTPUT_INVALID
ulog_output ulog_output_add(ulog_output_callback_fn callback, void *arg,
                            ulog_level level);

/// @brief Add file callback
/// @param file - File pointer
/// @param level - Debug level
/// @return ulog_output, on error - ULOG_OUTPUT_INVALID
ulog_output ulog_output_add_file(FILE *file, ulog_level level);

#endif  // ULOG_FEATURE_EXTRA_OUTPUTS

/* ============================================================================
   Feature: Log Topics
============================================================================ */
#if ULOG_FEATURE_TOPICS

// clang-format off
#define ulog_topic_trace(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_TRACE, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define ulog_topic_debug(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_DEBUG, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define ulog_topic_info(TOPIC_NAME, ...)  ulog_log(ULOG_LEVEL_INFO, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define ulog_topic_warn(TOPIC_NAME, ...)  ulog_log(ULOG_LEVEL_WARN, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define ulog_topic_error(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_ERROR, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define ulog_topic_fatal(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_FATAL, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
// Aliases
#define logt_trace(...) ulog_topic_trace(__VA_ARGS__)
#define logt_debug(...) ulog_topic_debug(__VA_ARGS__)
#define logt_info(...)  ulog_topic_info(__VA_ARGS__)
#define logt_warn(...)  ulog_topic_warn(__VA_ARGS__)
#define logt_error(...) ulog_topic_error(__VA_ARGS__)
#define logt_fatal(...) ulog_topic_fatal(__VA_ARGS__)
// clang-format on

typedef int ulog_topic_id;
enum {
    ULOG_TOPIC_ID_INVALID = -1,
};

/// @brief Adds a topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @param enable
/// @return Topic ID if success, ULOG_TOPIC_ID_INVALID if failed
ulog_topic_id ulog_topic_add(const char *topic_name, bool enable);

/// @brief Sets the debug level of a given topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @param level - Debug level
/// @return ULOG_STATUS_OK if success, ULOG_STATUS_ERROR if topic not found
ulog_status ulog_topic_level_set(const char *topic_name, ulog_level level);

/// @brief Gets the topic ID
/// @param topic_name - Topic name. "" and NULL are not valid
/// @return  Topic ID if success, ULOG_TOPIC_ID_INVALID if failed
ulog_topic_id ulog_topic_get_id(const char *topic_name);

/// @brief Enables the topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @return ULOG_STATUS_OK if success, ULOG_STATUS_ERROR if failed
ulog_status ulog_topic_enable(const char *topic_name);

/// @brief Disables the topic
/// @param topic_name - Topic name. "" and NULL are not valid
/// @return ULOG_STATUS_OK if success, ULOG_STATUS_ERROR if failed
ulog_status ulog_topic_disable(const char *topic_name);

/// @brief Enables all topics
ulog_status ulog_topic_enable_all(void);

/// @brief Disables all topics
ulog_status ulog_topic_disable_all(void);

#endif  // ULOG_FEATURE_TOPICS

#ifdef __cplusplus
}
#endif
