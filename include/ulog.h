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
#include <stdint.h>
#include <stdio.h>

/* ============================================================================
   Core Functionality
============================================================================ */
// clang-format off

/// @brief Status codes for ulog operations
typedef enum {
    ULOG_STATUS_OK           = 0,   ///< Operation completed successfully
    ULOG_STATUS_ERROR        = -1,  ///< General error occurred
    ULOG_STATUS_BAD_ARGUMENT = -2,  ///< Invalid argument provided
    ULOG_STATUS_INVALID_ARG  = -3,  ///< Invalid argument provided (alias)
} ulog_status;

/// @brief Log levels in ascending order of severity
typedef enum  ulog_level_enum { 
    ULOG_LEVEL_TRACE = 0,  ///< Most verbose level for tracing execution
    ULOG_LEVEL_DEBUG,      ///< Debug information for developers
    ULOG_LEVEL_INFO,       ///< General information messages
    ULOG_LEVEL_WARN,       ///< Warning messages for potential issues
    ULOG_LEVEL_ERROR,      ///< Error messages for failures
    ULOG_LEVEL_FATAL,      ///< Critical errors that may terminate program
} ulog_level;
#define ULOG_LEVEL_TOTAL  6  ///< Total number of log levels

#define ULOG_LEVEL_STYLE_LONG  0   /// Use long level strings (TRACE, DEBUG, etc.)
#define ULOG_LEVEL_STYLE_SHORT 1   /// Use short level strings (T, D, I, W, E, F)

typedef enum {
    ULOG_LEVEL_CFG_STYLE_DEFAULT = 0x0,
    ULOG_LEVEL_CFG_STYLE_SHORT,
    ULOG_LEVEL_CFG_STYLE_NUM
} ulog_level_cfg_style;

/// @brief Topic identifier type
typedef int ulog_topic_id;
enum {
    ULOG_TOPIC_ID_INVALID = -1,  ///< Invalid topic ID
};


/// @brief Returns the string representation of the log level
/// @param level Log level to convert
/// @return String representation of the level, or "?" for invalid levels
const char *ulog_level_to_string(ulog_level level);

/// @brief Log a TRACE level message
/// @param ... Format string and arguments (printf-style)
#define ulog_trace(...) ulog_log(ULOG_LEVEL_TRACE, __FILE__, __LINE__, NULL, __VA_ARGS__)

/// @brief Log a DEBUG level message
/// @param ... Format string and arguments (printf-style)
#define ulog_debug(...) ulog_log(ULOG_LEVEL_DEBUG, __FILE__, __LINE__, NULL, __VA_ARGS__)

/// @brief Log an INFO level message
/// @param ... Format string and arguments (printf-style)
#define ulog_info(...)  ulog_log(ULOG_LEVEL_INFO, __FILE__, __LINE__, NULL, __VA_ARGS__)

/// @brief Log a WARN level message
/// @param ... Format string and arguments (printf-style)
#define ulog_warn(...)  ulog_log(ULOG_LEVEL_WARN, __FILE__, __LINE__, NULL, __VA_ARGS__)

/// @brief Log an ERROR level message
/// @param ... Format string and arguments (printf-style)
#define ulog_error(...) ulog_log(ULOG_LEVEL_ERROR, __FILE__, __LINE__, NULL, __VA_ARGS__)

/// @brief Log a FATAL level message
/// @param ... Format string and arguments (printf-style)
#define ulog_fatal(...) ulog_log(ULOG_LEVEL_FATAL, __FILE__, __LINE__, NULL, __VA_ARGS__)

/// @brief Alias for ulog_trace
#define log_trace(...) ulog_trace(__VA_ARGS__)

/// @brief Alias for ulog_debug
#define log_debug(...) ulog_debug(__VA_ARGS__)

/// @brief Alias for ulog_info
#define log_info(...)  ulog_info(__VA_ARGS__)

/// @brief Alias for ulog_warn
#define log_warn(...)  ulog_warn(__VA_ARGS__)

/// @brief Alias for ulog_error
#define log_error(...) ulog_error(__VA_ARGS__)

/// @brief Alias for ulog_fatal
#define log_fatal(...) ulog_fatal(__VA_ARGS__)

// clang-format on

/// @brief Event structure (opaque)
typedef struct ulog_event ulog_event;

/// @brief Write event content to a buffer as a log message
/// @param ev Event to convert
/// @param out Output buffer to write to
/// @param out_size Size of the output buffer
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_BAD_ARGUMENT if invalid
/// parameters
ulog_status ulog_event_to_cstr(ulog_event *ev, char *out, size_t out_size);

/// @brief Extract the message from an event into a buffer
/// @param ev Event to extract message from
/// @param buffer Output buffer to write the message to
/// @param buffer_size Size of the output buffer
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_INVALID_ARG if invalid
/// parameters
ulog_status ulog_event_get_message(ulog_event *ev, char *buffer,
                                   size_t buffer_size);

/// @brief Get the topic ID from an event
/// @param ev Event to get topic from
/// @return Topic ID, or ULOG_TOPIC_ID_INVALID if event is NULL
ulog_topic_id ulog_event_get_topic(ulog_event *ev);

/// @brief Get the line number from an event
/// @param ev Event to get line number from
/// @return Line number, or -1 if event is NULL
int ulog_event_get_line(ulog_event *ev);

/// @brief Get the file name from an event
/// @param ev Event to get file name from
/// @return File name string, or NULL if event is NULL
const char *ulog_event_get_file(ulog_event *ev);

/// @brief Get the log level from an event
/// @param ev Event to get log level from
/// @return Log level, or ULOG_LEVEL_TRACE if event is NULL
ulog_level ulog_event_get_level(ulog_event *ev);

/// @brief Get the timestamp from an event (requires ULOG_BUILD_TIME=1)
/// @param ev Event to get timestamp from
/// @return Pointer to struct tm with time information, or NULL if event is NULL
///         or time feature disabled
struct tm *ulog_event_get_time(ulog_event *ev);

/// @brief Main logging function - typically called through macros
/// @param level Log level for this message
/// @param file Source file name (usually __FILE__)
/// @param line Source line number (usually __LINE__)
/// @param topic Topic name string, or NULL for no topic
/// @param message Printf-style format string
/// @param ... Format arguments for the message
void ulog_log(ulog_level level, const char *file, int line, const char *topic,
              const char *message, ...);

/* ============================================================================
   Core Functionality: Thread Safety
============================================================================ */

/// @brief Lock function type for thread synchronization
/// @param lock True to acquire lock, false to release lock
/// @param lock_arg User-provided argument passed during registration
typedef void (*ulog_lock_fn)(bool lock, void *lock_arg);

/// @brief Sets the thread synchronization lock function
/// @param function Lock function to use, or NULL to disable locking
/// @param lock_arg User argument passed to the lock function
void ulog_lock_set_fn(ulog_lock_fn function, void *lock_arg);

/* ============================================================================
   Feature: Dynamic Config
============================================================================ */

/// @brief Enable or disable colored output (requires
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param enabled True to enable colors, false to disable
void ulog_color_config(bool enabled);

/// @brief Enable or disable custom prefix (requires
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param enabled True to enable prefix, false to disable
void ulog_prefix_config(bool enabled);

/// @brief Enable or disable source location in logs (requires
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param enabled True to show file:line, false to hide
void ulog_source_location_config(bool enabled);

/// @brief Enable or disable timestamps in logs (requires
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param enabled True to show timestamps, false to hide
void ulog_time_config(bool enabled);

/// @brief Configure level string format (requires ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param use_short_levels True to use short level strings (T/D/I/W/E/F), false
///                         for long (TRACE/DEBUG/...)
void ulog_level_config(bool use_short_levels);

/// @brief Enable or disable topic support (requires
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param enabled True to enable topics, false to disable
void ulog_topic_config(bool enabled);

/* ============================================================================
   Feature: Prefix
============================================================================ */

/// @brief Callback function type for generating custom log prefixes
/// (requires: ULOG_BUILD_PREFIX_SIZE>0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param ev Log event containing level, file, line, etc.
/// @param prefix Buffer to write the prefix string to
/// @param prefix_size Size of the prefix buffer
typedef void (*ulog_prefix_fn)(ulog_event *ev, char *prefix,
                               size_t prefix_size);

/// @brief Sets the custom prefix generation function (requires
///        ULOG_BUILD_PREFIX_SIZE>0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param function Callback function to generate prefix, or NULL to disable
void ulog_prefix_set_fn(ulog_prefix_fn function);

/* ============================================================================
   Feature: Output
============================================================================ */

/// @brief Output handle type for managing log destinations
typedef int ulog_output;
enum {
    ULOG_OUTPUT_INVALID = -0x1,  ///< Invalid output handle
    ULOG_OUTPUT_STDOUT  = 0x0,   ///< Standard output handle
};

/// @brief Callback function type for custom log output handlers
/// @param ev Log event to process
/// @param arg User-provided argument passed during callback registration
typedef void (*ulog_output_callback_fn)(ulog_event *ev, void *arg);

/// @brief Sets the minimum log level for a specific output
/// @param output Output handle to configure
/// @param level Minimum log level for this output
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_BAD_ARGUMENT if invalid
///         parameters, ULOG_STATUS_ERROR if output not found
ulog_status ulog_output_level_set(ulog_output output, ulog_level level);

/// @brief Sets the minimum log level for all outputs
/// @param level Minimum log level for all outputs
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_BAD_ARGUMENT if invalid level
ulog_status ulog_output_level_set_all(ulog_level level);

/// @brief Adds a custom output callback (requires ULOG_BUILD_EXTRA_OUTPUTS>0
/// or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param callback Function to handle log events
/// @param arg User argument passed to the callback function
/// @param level Minimum log level for this output
/// @return Output handle on success, ULOG_OUTPUT_INVALID on error
ulog_output ulog_output_add(ulog_output_callback_fn callback, void *arg,
                            ulog_level level);

/// @brief Adds a file output (requires ULOG_BUILD_EXTRA_OUTPUTS>0 or
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param file File pointer to write logs to (must remain valid)
/// @param level Minimum log level for this file output
/// @return Output handle on success, ULOG_OUTPUT_INVALID on error
ulog_output ulog_output_add_file(FILE *file, ulog_level level);

/* ============================================================================
   Feature: Log Topics
============================================================================ */

// clang-format off
/// @brief Log a TRACE level message with topic (requires ULOG_BUILD_TOPICS!=0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param TOPIC_NAME Topic name string
/// @param ... Format string and arguments (printf-style)
#define ulog_topic_trace(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_TRACE, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

/// @brief Log a DEBUG level message with topic (requires ULOG_BUILD_TOPICS!=0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param TOPIC_NAME Topic name string
/// @param ... Format string and arguments (printf-style)
#define ulog_topic_debug(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_DEBUG, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

/// @brief Log an INFO level message with topic (requires ULOG_BUILD_TOPICS!=0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param TOPIC_NAME Topic name string
/// @param ... Format string and arguments (printf-style)
#define ulog_topic_info(TOPIC_NAME, ...)  ulog_log(ULOG_LEVEL_INFO, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

/// @brief Log a WARN level message with topic (requires ULOG_BUILD_TOPICS!=0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param TOPIC_NAME Topic name string
/// @param ... Format string and arguments (printf-style)
#define ulog_topic_warn(TOPIC_NAME, ...)  ulog_log(ULOG_LEVEL_WARN, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

/// @brief Log an ERROR level message with topic (requires ULOG_BUILD_TOPICS!=0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param TOPIC_NAME Topic name string
/// @param ... Format string and arguments (printf-style)
#define ulog_topic_error(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_ERROR, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

/// @brief Log a FATAL level message with topic (requires ULOG_BUILD_TOPICS!=0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param TOPIC_NAME Topic name string
/// @param ... Format string and arguments (printf-style)
#define ulog_topic_fatal(TOPIC_NAME, ...) ulog_log(ULOG_LEVEL_FATAL, __FILE__, __LINE__, TOPIC_NAME, __VA_ARGS__)

// Aliases
/// @brief Alias for ulog_topic_trace
#define logt_trace(...) ulog_topic_trace(__VA_ARGS__)

/// @brief Alias for ulog_topic_debug
#define logt_debug(...) ulog_topic_debug(__VA_ARGS__)

/// @brief Alias for ulog_topic_info
#define logt_info(...)  ulog_topic_info(__VA_ARGS__)

/// @brief Alias for ulog_topic_warn
#define logt_warn(...)  ulog_topic_warn(__VA_ARGS__)

/// @brief Alias for ulog_topic_error
#define logt_error(...) ulog_topic_error(__VA_ARGS__)

/// @brief Alias for ulog_topic_fatal
#define logt_fatal(...) ulog_topic_fatal(__VA_ARGS__)
// clang-format on

/// @brief Adds a topic  (requires ULOG_BUILD_TOPICS!=0 or
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param topic_name Topic name string (empty or NULL names are invalid)
/// @param enable Whether to enable the topic immediately
/// @return Topic ID on success, ULOG_TOPIC_ID_INVALID on failure
ulog_topic_id ulog_topic_add(const char *topic_name, bool enable);

/// @brief Sets the minimum log level for a topic  (requires
/// ULOG_BUILD_TOPICS!=0 or ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param topic_name Topic name string (empty or NULL names are invalid)
/// @param level Minimum log level for this topic
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR if topic not found
ulog_status ulog_topic_level_set(const char *topic_name, ulog_level level);

/// @brief Gets the ID of a topic by name  (requires ULOG_BUILD_TOPICS!=0 or
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param topic_name Topic name string (empty or NULL names are invalid)
/// @return Topic ID on success, ULOG_TOPIC_ID_INVALID if not found
ulog_topic_id ulog_topic_get_id(const char *topic_name);

/// @brief Enables a topic for logging  (requires ULOG_BUILD_TOPICS!=0 or
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param topic_name Topic name string (empty or NULL names are invalid)
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR if topic not found
ulog_status ulog_topic_enable(const char *topic_name);

/// @brief Disables a topic from logging  (requires ULOG_BUILD_TOPICS!=0 or
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @param topic_name Topic name string (empty or NULL names are invalid)
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR if topic not found
ulog_status ulog_topic_disable(const char *topic_name);

/// @brief Enables all existing topics  (requires ULOG_BUILD_TOPICS!=0 or
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR on failure
ulog_status ulog_topic_enable_all(void);

/// @brief Disables all existing topics  (requires ULOG_BUILD_TOPICS!=0 or
/// ULOG_BUILD_DYNAMIC_CONFIG=1)
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_ERROR on failure
ulog_status ulog_topic_disable_all(void);

#ifdef __cplusplus
}
#endif
