#ifndef ULOG_CORE_H
#define ULOG_CORE_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#if FEATURE_TIME
#include <time.h>
#endif

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
#if FEATURE_TOPICS
    int topic;
#endif
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
void ulog_set_level(int level);

/// @brief Sets the quiet mode
void ulog_set_quiet(bool enable);

/// @brief Write event content to a buffer as a log message
int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size);

/// @brief Logs the message
void ulog_log(int level, const char *file, int line, const char *topic, const char *message, ...);

#ifdef __cplusplus
}
#endif

#endif // ULOG_CORE_H
