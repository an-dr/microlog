#pragma once

#ifdef ARDUINO

#include <Arduino.h>
#include "ulog.h" // This will be the existing ulog.h

// Define a default Print object (e.g., Serial)
#ifndef ULOG_ARDUINO_PRINT
#define ULOG_ARDUINO_PRINT Serial
#endif

// Initialize the logger with a Print object
void ulog_arduino_init(Print* client = &ULOG_ARDUINO_PRINT);

// Arduino-specific log function
void ulog_arduino(int level, const char *file, int line, const char *topic, const char *fmt, ...);

// Macros for Arduino logging, similar to existing log_xxx macros
// These will call ulog_arduino
#define log_arduino_trace(...) ulog_arduino(LOG_TRACE, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_arduino_debug(...) ulog_arduino(LOG_DEBUG, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_arduino_info(...)  ulog_arduino(LOG_INFO,  __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_arduino_warn(...)  ulog_arduino(LOG_WARN,  __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_arduino_error(...) ulog_arduino(LOG_ERROR, __FILE__, __LINE__, NULL, __VA_ARGS__)
#define log_arduino_fatal(...) ulog_arduino(LOG_FATAL, __FILE__, __LINE__, NULL, __VA_ARGS__)

// Topic-based logging macros for Arduino
#define logt_arduino_trace(TOPIC, ...) ulog_arduino(LOG_TRACE, __FILE__, __LINE__, TOPIC, __VA_ARGS__)
#define logt_arduino_debug(TOPIC, ...) ulog_arduino(LOG_DEBUG, __FILE__, __LINE__, TOPIC, __VA_ARGS__)
#define logt_arduino_info(TOPIC, ...)  ulog_arduino(LOG_INFO,  __FILE__, __LINE__, TOPIC, __VA_ARGS__)
#define logt_arduino_warn(TOPIC, ...)  ulog_arduino(LOG_WARN,  __FILE__, __LINE__, TOPIC, __VA_ARGS__)
#define logt_arduino_error(TOPIC, ...) ulog_arduino(LOG_ERROR, __FILE__, __LINE__, TOPIC, __VA_ARGS__)
#define logt_arduino_fatal(TOPIC, ...) ulog_arduino(LOG_FATAL, __FILE__, __LINE__, TOPIC, __VA_ARGS__)

#endif // ARDUINO
