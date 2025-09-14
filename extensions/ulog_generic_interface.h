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
//   log_msg(INFO, "This is an info message");
//   log_tag(WARN, "NET", "This is a warning with tag");
//   log_msg(ERROR, "This is an error: %d", error_code);
//
// Output (default configuration):
//   INFO This is an info message
//   WARN [NET] This is a warning with tag
//   ERROR This is an error: 42
// ***************************************************************************

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "ulog.h"

/// @brief Log levels
typedef enum {
    TRACE = 0,
    DEBUG = 1,
    INFO  = 2,
    WARN  = 3,
    ERROR = 4,
    FATAL = 5
} log_level;

/// @brief Log a message at the specified level.
/// @param level Log level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL).
/// @param ... printf-style format string and arguments.
#define log_msg(level, ...) ulog(level, __VA_ARGS__)

/// @brief Log a message with a tag at the specified level.
/// @param level Log level (TRACE, DEBUG, INFO, WARN, ERROR, FATAL).
/// @param tag A string tag (e.g. "NET", "UI") to categorize the message.
/// @param ... printf-style format string and arguments.
#define log_tag(level, tag, ...) ulog_t(level, tag, __VA_ARGS__)

#ifdef __cplusplus
}
#endif
