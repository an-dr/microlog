// *************************************************************************
//
// ulog v6.3.0 - A simple customizable logging library.
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
| FEATURE_EXTRA_OUTPUTS | OFF             | ULOG_EXTRA_OUTPUTS                |
| FEATURE_TOPICS        | OFF             | ULOG_TOPICS_NUM                   |

============================================================================ */

// The FEATURE_ and CFG_ macros are removed. 
// Their values will be stored in g_ulog_config.
// Compile-time flags like ULOG_HAVE_TIME will be used 
// to initialize g_ulog_config.

/* ============================================================================
   Core Functionality
============================================================================ */

// ULOG_HAVE_TIME still controls this include for struct tm definition
#ifdef ULOG_HAVE_TIME
#include <time.h>
#endif

// Struct to hold all ulog configurations
typedef struct {
    bool time_enabled;          // Replaces FEATURE_TIME
    bool color_enabled;         // Replaces FEATURE_COLOR
    int custom_prefix_size;     // Replaces FEATURE_CUSTOM_PREFIX and CFG_CUSTOM_PREFIX_SIZE
    bool file_string_enabled;   // Replaces FEATURE_FILE_STRING
    bool short_level_strings;   // Replaces FEATURE_SHORT_LEVELS
    bool emoji_levels;          // Replaces FEATURE_EMOJI_LEVELS
    int extra_outputs;          // Replaces FEATURE_EXTRA_OUTPUTS and CFG_EXTRA_OUTPUTS
    int topics_num;             // Replaces FEATURE_TOPICS and CFG_TOPICS_NUM
    bool topics_dynamic_alloc;  // Replaces CFG_TOPICS_DINAMIC_ALLOC
    int level;                  // Existing runtime-configurable option
    bool quiet;                 // Existing runtime-configurable option
} ulog_config_t;

enum { LOG_TRACE,
       LOG_DEBUG,
       LOG_INFO,
       LOG_WARN,
       LOG_ERROR,
       LOG_FATAL };

#define log_trace(...) ulog_log(LOG_TRACE, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_debug(...) ulog_log(LOG_DEBUG, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_info(...) ulog_log(LOG_INFO, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_warn(...) ulog_log(LOG_WARN, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_error(...) ulog_log(LOG_ERROR, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_fatal(...) ulog_log(LOG_FATAL, __FILE__, __LINE__, NULL, __VA_ARGS__)


/// @brief Event structure
typedef struct {
    const char *message;          // Message format string
    va_list message_format_args;  // Format arguments

// ULOG_TOPICS_NUM will determine if this field is present
#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0
    int topic;
#endif

// ULOG_HAVE_TIME will determine if this field is present
#ifdef ULOG_HAVE_TIME
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

/// @brief Initializes the ulog library with a specific configuration.
/// If NULL is passed, default configuration with compile-time overrides is used.
/// @param config - Pointer to a ulog_config_t struct, or NULL.
void ulog_init(const ulog_config_t *config);

/// @brief Write event content to a buffer as a log message
/// @param ev - Event
/// @param out_buf - Output buffer
/// @param out_buf_size - Output buffer size
/// @return 0 if success, -1 if failed
int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size);

/// @brief Logs the message
/// @param level - Debug level
/// @param file - File name
/// @param line - Line number
/// @param topic - Topic name
/// @param message - Message format string
/// @param ... - Format arguments
void ulog_log(int level, const char *file, int line, const char *topic, const char *message, ...);

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

// The function ulog_set_prefix_fn is now always declared.
// Runtime checks g_ulog_config.custom_prefix_size will control its behavior.
typedef void (*ulog_PrefixFn)(ulog_Event *ev, char *prefix, size_t prefix_size);

/// @brief Sets the prefix function
/// @param function - Prefix function
void ulog_set_prefix_fn(ulog_PrefixFn function);

/* ============================================================================
   Feature: Extra Outputs
============================================================================ */

// Functions ulog_add_callback and ulog_add_fp are now always declared.
// Runtime checks g_ulog_config.extra_outputs will control their behavior.

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

/* ============================================================================
   Feature: Log Topics
============================================================================ */

#define TOPIC_NOT_FOUND 0x7FFFFFFF // This can remain as it's a constant for return values
#define logt_trace(TOPIC_NAME, ...) ulog_log(LOG_TRACE, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_debug(TOPIC_NAME, ...) ulog_log(LOG_DEBUG, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_info(TOPIC_NAME, ...) ulog_log(LOG_INFO, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_warn(TOPIC_NAME, ...) ulog_log(LOG_WARN, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_error(TOPIC_NAME, ...) ulog_log(LOG_ERROR, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)
#define logt_fatal(TOPIC_NAME, ...) ulog_log(LOG_FATAL, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

// Topic related functions are now always declared.
// Runtime checks g_ulog_config.topics_num will control their behavior.

/// @brief Adds a topic
/// @param topic_name
/// @param enable
/// @return Topic ID if success, -1 if failed
int ulog_add_topic(const char *topic_name, bool enable);

/// @brief Sets the debug level of a given topic
/// @param topic_name
/// @param level
/// @return 0 if success, -1 if failed
int ulog_set_topic_level(const char *topic_name, int level);

/// @brief Gets the topic ID
/// @param topic_name
/// @return  Topic ID if success, -1 if failed, TOPIC_NOT_FOUND if not found
int ulog_get_topic_id(const char *topic_name);

/// @brief Enables the topic
/// @param topic_name - Topic name
/// @return 0 if success, -1 if failed
int ulog_enable_topic(const char *topic_name);

/// @brief Disables the topic
/// @param topic_name - Topic name
/// @return 0 if success, -1 if failed
int ulog_disable_topic(const char *topic_name);

/// @brief Enables all topics
int ulog_enable_all_topics(void);

/// @brief Disables all topics
int ulog_disable_all_topics(void);

#ifdef __cplusplus
}
#endif
