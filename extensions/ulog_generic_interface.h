// ***************************************************************************
//
// microlog extension: Generic Interface (Header-Only)
//
// This provides a very simple logging interface with minimal dependencies.
// It maps to the underlying ulog functions. You can use this interface in
// your code to allow easy switching to other logging libraries in the future.
//
// Usage:
//   #include "ulog_generic_interface.h"
//   log_message(LOG_INFO, "This is an info message");
//   log_topic(LOG_WARN, "NET", "This is a warning with topic");
//   log_message(LOG_ERROR, "This is an error: %d", error_code);
//
// Output (default configuration):
//   INFO This is an info message
//   WARN [NET] This is a warning with topic
//   ERROR This is an error: 42
// ***************************************************************************

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "ulog.h"

/// @brief Log levels
typedef enum {
    LOG_TRACE = 0,
    LOG_DEBUG = 1,
    LOG_INFO  = 2,
    LOG_WARN  = 3,
    LOG_ERROR = 4,
    LOG_FATAL = 5
} log_level;

/// @brief Log a message at the specified level.
/// @param level Log level (LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL).
/// @param ... printf-style format string and arguments.
#define log_message(level, ...) ulog(level, __VA_ARGS__)

/// @brief Log a message with a topic at the specified level.
/// @param level Log level (LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL).
/// @param topic A string topic (e.g. "NET", "UI") to categorize the message.
/// @param ... printf-style format string and arguments.
#define log_topic(level, topic, ...) ulog_t(level, topic, __VA_ARGS__)

#ifdef __cplusplus
}
#endif
