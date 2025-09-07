// *************************************************************************
//
// ulog v@{ULOG_VERSION}@ - A simple customizable logging library.
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

#include "ulog.h"
#include <stdlib.h>
#include <string.h>

// clang-format off
/* ====================================================================================================================
   Core Feature: Static Configuration
=======================================================================================================================

| Build Option                | Default               | Dependent Macro(s)            | Purpose                  |
| --------------------------- | --------------------- | ----------------------------- | ------------------------ |
| ULOG_BUILD_COLOR            | 0                     | ULOG_HAS_COLOR                | Compile color code paths |
| ULOG_BUILD_PREFIX_SIZE      | 0                     | ULOG_HAS_PREFIX               | Prefix buffer logic      |
| ULOG_BUILD_EXTRA_OUTPUTS    | 0                     | ULOG_HAS_EXTRA_OUTPUTS        | Extra output backends    |
| ULOG_BUILD_SOURCE_LOCATION  | 1                     | ULOG_HAS_SOURCE_LOCATION      | File\:line output        |
| ULOG_BUILD_LEVEL_STYLE      | ULOG_LEVEL_STYLE_LONG | ULOG_LEVEL_HAS_SHORT/_LONG    | Level style              |
| ULOG_BUILD_TIME             | 0                     | ULOG_HAS_TIME                 | Timestamp support        |
| ULOG_BUILD_TOPICS_NUM       | 0                     | ULOG_HAS_TOPICS               | Topic filtering logic    |
| ULOG_BUILD_DYNAMIC_CONFIG   | 0                     | ULOG_HAS_DYNAMIC_CONFIG       | Runtime toggles          |
| ULOG_BUILD_WARN_NOT_ENABLED | 1                     | ULOG_HAS_WARN_NOT_ENABLED     | Warning stubs            |

===================================================================================================================== */

#ifndef ULOG_BUILD_COLOR
    #define ULOG_HAS_COLOR 0
#else
    #define ULOG_HAS_COLOR (ULOG_BUILD_COLOR==1)
#endif


#ifndef ULOG_BUILD_PREFIX_SIZE
    #define ULOG_HAS_PREFIX 0
#else
    #define ULOG_HAS_PREFIX (ULOG_BUILD_PREFIX_SIZE > 0)
#endif


#ifndef ULOG_BUILD_TIME
    #define ULOG_HAS_TIME 0
#else
    #define ULOG_HAS_TIME (ULOG_BUILD_TIME==1)
#endif


#ifndef ULOG_BUILD_LEVEL_STYLE
    #define ULOG_HAS_LEVEL_LONG  1
    #define ULOG_HAS_LEVEL_SHORT 0
#else
    #define ULOG_HAS_LEVEL_LONG  (ULOG_BUILD_LEVEL_STYLE == ULOG_LEVEL_STYLE_LONG)
    #define ULOG_HAS_LEVEL_SHORT (ULOG_BUILD_LEVEL_STYLE == ULOG_LEVEL_STYLE_SHORT)
#endif


#ifndef ULOG_BUILD_EXTRA_OUTPUTS
    #define ULOG_HAS_EXTRA_OUTPUTS 0
#else
    #define ULOG_HAS_EXTRA_OUTPUTS (ULOG_BUILD_EXTRA_OUTPUTS > 0)
#endif


#ifndef ULOG_BUILD_SOURCE_LOCATION
    #define ULOG_HAS_SOURCE_LOCATION 1
#else
    #define ULOG_HAS_SOURCE_LOCATION (ULOG_BUILD_SOURCE_LOCATION == 1)
#endif


#ifndef ULOG_BUILD_WARN_NOT_ENABLED
    #define ULOG_HAS_WARN_NOT_ENABLED 1
#else
    #define ULOG_HAS_WARN_NOT_ENABLED (ULOG_BUILD_WARN_NOT_ENABLED==1)
#endif


#ifndef ULOG_BUILD_TOPICS_NUM
    #define ULOG_HAS_TOPICS 0
#else
    #define ULOG_HAS_TOPICS (ULOG_BUILD_TOPICS_NUM > 0 || ULOG_BUILD_TOPICS_NUM == -1)
#endif


#ifndef ULOG_BUILD_DYNAMIC_CONFIG
    #define ULOG_HAS_DYNAMIC_CONFIG 0
#else
    #define ULOG_HAS_DYNAMIC_CONFIG 1
    
    // Undef macros to avoid conflicts
    #undef ULOG_BUILD_EXTRA_OUTPUTS
    #undef ULOG_BUILD_PREFIX_SIZE
    #undef ULOG_BUILD_TOPICS_NUM
    #undef ULOG_HAS_COLOR
    #undef ULOG_HAS_EXTRA_OUTPUTS
    #undef ULOG_HAS_LEVEL_LONG
    #undef ULOG_HAS_LEVEL_SHORT
    #undef ULOG_HAS_PREFIX
    #undef ULOG_HAS_SOURCE_LOCATION
    #undef ULOG_HAS_TIME
    #undef ULOG_HAS_TOPICS
    #undef ULOG_HAS_WARN_NOT_ENABLED
    
    // Configure features based on runtime config
    #define ULOG_BUILD_EXTRA_OUTPUTS 8
    #define ULOG_BUILD_PREFIX_SIZE 64
    #define ULOG_BUILD_TOPICS_NUM -1
    #define ULOG_HAS_COLOR 1
    #define ULOG_HAS_EXTRA_OUTPUTS 1
    #define ULOG_HAS_LEVEL_LONG 1
    #define ULOG_HAS_LEVEL_SHORT 1
    #define ULOG_HAS_PREFIX 1
    #define ULOG_HAS_SOURCE_LOCATION 1
    #define ULOG_HAS_TIME 1
    #define ULOG_HAS_TOPICS 1
    #define ULOG_HAS_WARN_NOT_ENABLED 0
#endif

// clang-format on

/* ============================================================================
   Tools
============================================================================ */

// If testing is enabled, we define NOT_SO_STATIC as empty to allow
// the functions to be visible outside of this file for testing purposes.
#ifdef ULOG_TESTING
#define NOT_VERY_STATIC
#else
#define NOT_VERY_STATIC static
#endif

// Check if the string is empty or not provided
static inline bool is_str_empty(const char *str) {
    return (str == NULL) || (str[0] == '\0');
}

// Macro to log a warning when a feature is not enabled
// Usage: warn_not_enabled("ULOG_BUILD_TIME")
// Output:
//   WARN src/main.c:42: 'ulog_configure_time' ignored: ULOG_BUILD_TIME disabled
#define warn_not_enabled(feature)                                              \
    ulog_warn("'%s' called with %s disabled", __func__, feature)

/* ============================================================================
   Core Feature: Print
   (`print_*`, depends on: - )
============================================================================ */

//  Private
// ================

typedef struct {
    char *data;
    unsigned int curr_pos;
    size_t size;
} print_buffer;

typedef union {
    print_buffer buffer;
    FILE *stream;
} print_target_descriptor;

typedef enum { PRINT_TARGET_BUFFER, PRINT_TARGET_STREAM } print_target_type;

typedef struct {
    print_target_type type;
    print_target_descriptor dsc;
} print_target;

static void print_to_target_valist(print_target *tgt, const char *format,
                                   va_list args) {
    if (tgt->type == PRINT_TARGET_BUFFER) {
        char *buf   = tgt->dsc.buffer.data + tgt->dsc.buffer.curr_pos;
        size_t size = tgt->dsc.buffer.size - tgt->dsc.buffer.curr_pos;
        if (size > 0U) {
            tgt->dsc.buffer.curr_pos += vsnprintf(buf, size, format, args);
        }
    } else if (tgt->type == PRINT_TARGET_STREAM) {
        FILE *stream = tgt->dsc.stream;
        vfprintf(stream, format, args);
    }
}

static void print_to_target(print_target *tgt, const char *format, ...) {
    va_list args;
    va_start(args, format);
    print_to_target_valist(tgt, format, args);
    va_end(args);
}

/* ============================================================================
   Core Feature: Events
   (`event_*`, depends on: Print)
============================================================================ */

// Private
// ================

static void log_print_message(print_target *tgt, ulog_event *ev);

/// @brief Event structure
struct ulog_event {
    const char *message;          // Message format string
    va_list message_format_args;  // Format arguments

#if ULOG_HAS_TOPICS
    ulog_topic_id topic;
#endif

#if ULOG_HAS_TIME
    struct tm *time;
#endif

#if ULOG_HAS_SOURCE_LOCATION
    const char *file;  // Event file name
    int line;          // Event line number
#endif                 // ULOG_HAS_SOURCE_LOCATION

    ulog_level level;  // Event debug level
};

ulog_status ulog_event_get_message(ulog_event *ev, char *buffer,
                                   size_t buffer_size) {
    if (ev == NULL || buffer == NULL || buffer_size == 0) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    print_target tgt = {.type       = PRINT_TARGET_BUFFER,
                        .dsc.buffer = {buffer, 0, buffer_size}};

    log_print_message(&tgt, ev);
    return ULOG_STATUS_OK;
}

#if ULOG_HAS_TOPICS
ulog_topic_id ulog_event_get_topic(ulog_event *ev) {
    if (ev == NULL) {
        return ULOG_TOPIC_ID_INVALID;  // Invalid topic
    }
    return ev->topic;
}
#endif  // ULOG_HAS_TOPICS

#if ULOG_HAS_TIME
struct tm *ulog_event_get_time(ulog_event *ev) {
    if (ev == NULL) {
        return NULL;
    }
    return ev->time;
}
#endif  // ULOG_HAS_TIME

#if ULOG_HAS_SOURCE_LOCATION
const char *ulog_event_get_file(ulog_event *ev) {
    if (ev == NULL) {
        return NULL;
    }
    return ev->file;
}

int ulog_event_get_line(ulog_event *ev) {
    if (ev == NULL) {
        return -1;
    }
    return ev->line;
}
#endif  // ULOG_HAS_SOURCE_LOCATION

ulog_level ulog_event_get_level(ulog_event *ev) {
    if (ev == NULL) {
        return ULOG_LEVEL_TRACE;  // Invalid level
    }
    return ev->level;
}

/* ============================================================================
   Core Functionality: Lock
   (`lock_*`, depends on: - )
============================================================================ */

// Private
// ================
typedef struct {
    ulog_lock_fn function;  // Lock function
    void *args;             // Argument for the lock function
} lock_data_t;

static lock_data_t lock_data = {
    .function = NULL,  // No lock function by default
    .args     = NULL,  // No lock argument by default
};

static void lock_lock(void) {
    if (lock_data.function != NULL) {
        lock_data.function(true, lock_data.args);
    }
}

static void lock_unlock(void) {
    if (lock_data.function != NULL) {
        lock_data.function(false, lock_data.args);
    }
}

// Public
// ================

/// @brief  Sets the lock function and user data
void ulog_lock_set_fn(ulog_lock_fn function, void *lock_arg) {
    lock_data.function = function;
    lock_data.args     = lock_arg;
}
/* ============================================================================
   Optional Feature: Dynamic Configuration - Color
   (`color_config_*`, depends on: - )
============================================================================ */
#if ULOG_HAS_DYNAMIC_CONFIG

typedef struct {
    bool enabled;
} color_config;

static color_config color_cfg = {
    .enabled = (bool)ULOG_HAS_COLOR,
};

// Private
// ================

bool color_config_is_enabled(void) {
    return color_cfg.enabled;
}

// Public
// ================

void ulog_color_config(bool enabled) {
    lock_lock();  // Lock the configuration
    color_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
void ulog_color_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_COLOR");
}
#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define color_config_is_enabled() (ULOG_HAS_COLOR)

#endif  // ULOG_HAS_DYNAMIC_CONFIG

/* ============================================================================
   Optional Feature: Color
   (`color_*`, depends on: Print, Color Config)
============================================================================ */
#if ULOG_HAS_COLOR

//  Private
// ================
static const char *color_levels[] = {
    "\x1b[37m",  // TRACE : White #000
    "\x1b[36m",  // DEBUG : Cyan #0ff
    "\x1b[32m",  // INFO : Green #0f0
    "\x1b[33m",  // WARN : Yellow #ff0
    "\x1b[31m",  // ERROR : Red #f00
    "\x1b[35m"   // FATAL : Magenta #f0f
};
#define COLOR_TERMINATOR "\x1b[0m"

static void color_print_start(print_target *tgt, ulog_event *ev) {
    if (!color_config_is_enabled()) {
        return;  // Color is disabled, do not print color codes
    }
    print_to_target(tgt, "%s", color_levels[ev->level]);  // color start
}

static void color_print_end(print_target *tgt) {
    if (!color_config_is_enabled()) {
        return;  // Color is disabled, do not print color codes
    }
    print_to_target(tgt, "%s", COLOR_TERMINATOR);  // color end
}

#else  // ULOG_HAS_COLOR

// Disabled Private
// ================

#define color_print_start(tgt, ev) (void)(tgt), (void)(ev)
#define color_print_end(tgt) (void)(tgt)

#endif  // ULOG_HAS_COLOR

/* ============================================================================
   Optional Feature: Dynamic Configuration - Prefix
   (`prefix_config_*`, depends on: - )
============================================================================ */
#if ULOG_HAS_DYNAMIC_CONFIG

typedef struct {
    bool enabled;
} prefix_config;

static prefix_config prefix_cfg = {
    .enabled = (bool)ULOG_HAS_PREFIX,
};

// Private
// ================

bool prefix_config_is_enabled(void) {
    return prefix_cfg.enabled;
}

// Public
// ================

void ulog_prefix_config(bool enabled) {
    lock_lock();  // Lock the configuration
    prefix_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
void ulog_prefix_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_DYNAMIC_CONFIG");
}
#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define prefix_config_is_enabled() (ULOG_HAS_PREFIX)
#endif  // ULOG_HAS_DYNAMIC_CONFIG

/* ============================================================================
   Optional Feature: Prefix
   (`prefix_*`, depends on: Print, Prefix Config)
============================================================================ */
#if ULOG_HAS_PREFIX

// Private
// ================
typedef struct {
    ulog_prefix_fn function;
    char prefix[ULOG_BUILD_PREFIX_SIZE];
} prefix_data_t;

static prefix_data_t prefix_data = {
    .function = NULL,
    .prefix   = {0},
};

static void prefix_update(ulog_event *ev) {
    if (prefix_data.function == NULL || !prefix_config_is_enabled()) {
        return;
    }
    prefix_data.function(ev, prefix_data.prefix, ULOG_BUILD_PREFIX_SIZE);
}

static void prefix_print(print_target *tgt, ulog_event *ev) {
    if (prefix_data.function == NULL || !prefix_config_is_enabled()) {
        return;
    }
    print_to_target(tgt, "%s", prefix_data.prefix);
}

// Public
// ================

void ulog_prefix_set_fn(ulog_prefix_fn function) {
    if (function == NULL) {
        return;  // Ignore NULL function
    }
    prefix_data.function = function;
}

#else  // ULOG_HAS_PREFIX

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
void ulog_prefix_set_fn(ulog_prefix_fn function) {
    (void)(function);
    warn_not_enabled("ULOG_BUILD_PREFIX_SIZE");
}
#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define prefix_print(tgt, ev) (void)(tgt), (void)(ev)
#define prefix_update(ev) (void)(ev)
#endif  // ULOG_HAS_PREFIX

/* ============================================================================
   Optional Feature: Dynamic Configuration - Time
   (`time_config_*`, depends on: - )
============================================================================ */
#if ULOG_HAS_DYNAMIC_CONFIG

typedef struct {
    bool enabled;
} time_config;

static time_config time_cfg = {
    .enabled = ULOG_HAS_TIME,
};

// Private
// ================

bool time_config_is_enabled(void) {
    return time_cfg.enabled;
}

// Public
// ================

void ulog_time_config(bool enabled) {
    lock_lock();  // Lock the configuration
    time_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
void ulog_time_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_TIME");
}
#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define time_config_is_enabled() (ULOG_HAS_TIME)
#endif  // ULOG_HAS_DYNAMIC_CONFIG

/* ============================================================================
   Optional Feature: Time
   (`time_*`, depends on: Time, Print)
============================================================================ */
#if ULOG_HAS_TIME

#include <time.h>

#define TIME_SHORT_BUF_SIZE 10  // HH:MM:SS(8) + 1 space + null
#define TIME_FULL_BUF_SIZE 21   // YYYY-MM-DD HH:MM:SS(19) + 1 space + null

// Private
// ================
static bool time_print_if_invalid(print_target *tgt, ulog_event *ev) {
    if (ev->time == NULL) {
        print_to_target(tgt, "INVALID_TIME");
        return true;  // Time is invalid, print error message
    }
    return false;  // Time is valid
}

/// @brief Fills the event time with the current local time
/// @param ev - Event to fill. Assumed not NULL
static void time_fill_current_time(ulog_event *ev) {
    time_t current_time = time(NULL);  // Get current time
    // Use localtime because existing tests inspect local clock fields.
    ev->time = localtime(&current_time);
}

static void time_print_short(print_target *tgt, ulog_event *ev,
                             bool append_space) {
    if (!time_config_is_enabled() || time_print_if_invalid(tgt, ev)) {
        return;  // If time is not valid or disabled, stop printing
    }
    char buf[TIME_SHORT_BUF_SIZE] = {0};
    const char *format            = append_space ? "%H:%M:%S " : "%H:%M:%S";
    strftime(buf, TIME_SHORT_BUF_SIZE, format, ev->time);
    print_to_target(tgt, "%s", buf);
}

#if ULOG_HAS_EXTRA_OUTPUTS
static void time_print_full(print_target *tgt, ulog_event *ev,
                            bool append_space) {
    if (!time_config_is_enabled() || time_print_if_invalid(tgt, ev)) {
        return;  // If time is not valid or disabled, stop printing
    }
    char buf[TIME_FULL_BUF_SIZE] = {0};
    const char *format =
        append_space ? "%Y-%m-%d %H:%M:%S " : "%Y-%m-%d %H:%M:%S";
    strftime(buf, TIME_FULL_BUF_SIZE, format, ev->time);
    print_to_target(tgt, "%s", buf);
}
#else
#define time_print_full(tgt, ev, append_space) (void)(0)
#endif  // ULOG_HAS_EXTRA_OUTPUTS

#else  // ULOG_HAS_TIME

// Disabled Private
// ================

#define time_print_short(tgt, ev, append_space) (void)(0)
#define time_print_full(tgt, ev, append_space) (void)(0)
#define time_fill_current_time(ev) (void)(ev)
#endif  // ULOG_HAS_TIME

/* ============================================================================
   Optional Feature: Dynamic Configuration - Level
   (`level_config_*`, depends on: - )
============================================================================ */
#if ULOG_HAS_DYNAMIC_CONFIG

typedef struct {
    ulog_level_config_style style;  // Use short level strings
} level_config;

static level_config level_cfg = {
    .style = ULOG_LEVEL_CONFIG_STYLE_DEFAULT,
};

// Private
// ================

bool level_config_is_short(void) {
    return level_cfg.style == ULOG_LEVEL_CONFIG_STYLE_SHORT;
}

// Public
// ================

void ulog_level_config(ulog_level_config_style style) {
    lock_lock();  // Lock the configuration
    level_cfg.style = style;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

void ulog_level_config(ulog_level_config_style style) {
    (void)(style);
    warn_not_enabled("ULOG_BUILD_DYNAMIC_CONFIG");
}

#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define level_config_is_short() (ULOG_HAS_LEVEL_SHORT)
#endif  // ULOG_HAS_DYNAMIC_CONFIG

/* ============================================================================
   Core Feature: Levels
   (`level_*`, depends on: Levels Config, Print)
============================================================================ */

// Private
// ================
#define LEVEL_MIN_VALUE 0
#define LEVEL_STYLE_NUM                                                        \
    (ULOG_HAS_LEVEL_SHORT + ULOG_HAS_LEVEL_LONG)  // TODO: fragile

/// @brief Level strings
static const char *level_strings[LEVEL_STYLE_NUM][ULOG_LEVEL_TOTAL] = {

#if ULOG_HAS_LEVEL_LONG
    {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"},
#endif

#if ULOG_HAS_LEVEL_SHORT
    {"T", "D", "I", "W", "E", "F"},
#endif

};

static bool level_is_allowed(ulog_level msg_level, ulog_level log_verbosity) {
    if (msg_level < log_verbosity || msg_level < LEVEL_MIN_VALUE) {
        return false;  // Level is higher than the configured level, not allowed
    }
    return true;  // Level is allowed
}

static void level_print(print_target *tgt, ulog_event *ev) {
#if ULOG_HAS_LEVEL_LONG && ULOG_HAS_LEVEL_SHORT
    if (level_config_is_short()) {
        print_to_target(
            tgt, "%-1s ",
            level_strings[ULOG_LEVEL_CONFIG_STYLE_SHORT][ev->level]);
    } else {
        print_to_target(
            tgt, "%-5s ",
            level_strings[ULOG_LEVEL_CONFIG_STYLE_DEFAULT][ev->level]);
    }
#elif ULOG_HAS_LEVEL_SHORT
    print_to_target(tgt, "%-1s ",
                    level_strings[ULOG_LEVEL_CONFIG_STYLE_DEFAULT][ev->level]);
#else
    print_to_target(tgt, "%-5s ",
                    level_strings[ULOG_LEVEL_CONFIG_STYLE_DEFAULT][ev->level]);
#endif  // ULOG_HAS_LEVEL_SHORT
}

// Public
// ================

/// @brief Returns the string representation of the level
const char *ulog_level_to_string(ulog_level level) {
    if (level < LEVEL_MIN_VALUE || level >= ULOG_LEVEL_TOTAL) {
        return "?";  // Return a default string for invalid levels
    }
    return level_strings[ULOG_LEVEL_CONFIG_STYLE_DEFAULT][level];
}

/* ============================================================================
   Core Feature: Outputs
   (`output_*`, depends on: Print, Log, Level)
============================================================================ */

//  Private
// ================
#if ULOG_HAS_EXTRA_OUTPUTS
#define OUTPUT_EXTRA_NUM ULOG_BUILD_EXTRA_OUTPUTS
#else
#define OUTPUT_EXTRA_NUM 0
#endif  // ULOG_HAS_EXTRA_OUTPUTS

#define OUTPUT_STDOUT_DEFAULT_LEVEL ULOG_LEVEL_TRACE
#define OUTPUT_TOTAL_NUM (1 + OUTPUT_EXTRA_NUM)  // stdout + extra

// Prototypes
static void output_stdout_callback(ulog_event *ev, void *arg);
static void log_print_event(print_target *tgt, ulog_event *ev, bool full_time,
                            bool color, bool new_line);

typedef struct {
    ulog_output_callback_fn callback;
    void *arg;
    ulog_level level;
} output;

typedef struct {
    output outputs[OUTPUT_TOTAL_NUM];  // order num = id. 0 is for stdout
} output_data_t;

static output_data_t output_data = {
    .outputs = {{output_stdout_callback, NULL, OUTPUT_STDOUT_DEFAULT_LEVEL}}};

static void output_handle_single(ulog_event *ev, output *output) {
    if (output->callback == NULL) {
        return;  // Output has been removed, skip it
    }

    if (level_is_allowed(ev->level, output->level)) {

        // Create event copy to avoid va_list issues
        ulog_event ev_copy = {0};
        memcpy(&ev_copy, ev, sizeof(ulog_event));

        // Initialize the va_list for the copied event
        // Note: We use a copy of the va_list to avoid issues with passing it
        // directly as on some platforms using the same va_list multiple times
        // can lead to undefined behavior.
        va_copy(ev_copy.message_format_args, ev->message_format_args);
        output->callback(&ev_copy, output->arg);
        va_end(ev_copy.message_format_args);
    }
}

static void output_handle_by_id(ulog_event *ev, ulog_output_id output_id) {
    // Validate output ID bounds
    if (output_id < 0 || output_id >= OUTPUT_TOTAL_NUM) {
        return;  // Invalid output ID
    }
    output_handle_single(ev, &output_data.outputs[output_id]);
}

static void output_handle_all(ulog_event *ev) {
    // Processing the message for outputs
    for (int i = 0;
         (i < OUTPUT_TOTAL_NUM) && (output_data.outputs[i].callback != NULL);
         i++) {
        output_handle_single(ev, &output_data.outputs[i]);
    }
}

static void output_stdout_callback(ulog_event *ev, void *arg) {
    (void)(arg);  // Unused
    print_target tgt = {.type = PRINT_TARGET_STREAM, .dsc.stream = stdout};
    log_print_event(&tgt, ev, false, true, true);
}

// Public
// ================

ulog_status ulog_output_level_set(ulog_output_id output, ulog_level level) {
    if (level < LEVEL_MIN_VALUE || level >= ULOG_LEVEL_TOTAL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    if (output < ULOG_OUTPUT_STDOUT || output >= OUTPUT_TOTAL_NUM) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    if (output_data.outputs[output].callback == NULL) {
        return ULOG_STATUS_NOT_FOUND;  // Output exists but no callback assigned
    }
    output_data.outputs[output].level = level;
    return ULOG_STATUS_OK;
}

ulog_status ulog_output_level_set_all(ulog_level level) {
    if (level < LEVEL_MIN_VALUE || level >= ULOG_LEVEL_TOTAL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    for (int i = 0; i < OUTPUT_TOTAL_NUM; i++) {
        output_data.outputs[i].level = level;
    }
    return ULOG_STATUS_OK;
}

/* ============================================================================
   Optional Feature: Extra Outputs
   (`output_*` depends on: Outputs)
============================================================================ */
#if ULOG_HAS_EXTRA_OUTPUTS

//  Private
// ================
static void output_file_callback(ulog_event *ev, void *arg) {
    print_target tgt = {.type = PRINT_TARGET_STREAM, .dsc.stream = (FILE *)arg};
    log_print_event(&tgt, ev, true, false, true);
}

// Public
// ================

ulog_output_id ulog_output_add(ulog_output_callback_fn callback, void *arg,
                               ulog_level level) {
    lock_lock();  // Lock the configuration
    for (int i = 0; i < OUTPUT_TOTAL_NUM; i++) {
        if (output_data.outputs[i].callback == NULL) {
            output_data.outputs[i] = (output){callback, arg, level};
            lock_unlock();  // Unlock the configuration
            return i;
        }
    }
    lock_unlock();  // Unlock the configuration
    return ULOG_OUTPUT_INVALID;
}

/// @brief Add file callback
ulog_output_id ulog_output_add_file(FILE *file, ulog_level level) {
    return ulog_output_add(output_file_callback, file, level);
}

/// @brief Remove an output from the logging system
ulog_status ulog_output_remove(ulog_output_id output) {
    if (output < 0 || output >= OUTPUT_TOTAL_NUM) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    if (output == ULOG_OUTPUT_STDOUT) {
        return ULOG_STATUS_ERROR;  // Cannot remove stdout output
    }

    lock_lock();
    if (output_data.outputs[output].callback == NULL) {
        lock_unlock();
        return ULOG_STATUS_NOT_FOUND;  // Output not found or already removed
    }

    // Mark output as removed by setting callback to NULL
    output_data.outputs[output].callback = NULL;
    output_data.outputs[output].arg      = NULL;
    output_data.outputs[output].level    = ULOG_LEVEL_TRACE;

    lock_unlock();
    return ULOG_STATUS_OK;
}

#else  // ULOG_HAS_EXTRA_OUTPUTS

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

ulog_output_id ulog_output_add(ulog_output_callback_fn callback, void *arg,
                               ulog_level level) {
    (void)(callback);
    (void)(arg);
    (void)(level);
    warn_not_enabled("ULOG_BUILD_EXTRA_OUTPUTS");
    return ULOG_OUTPUT_INVALID;
}

ulog_output_id ulog_output_add_file(FILE *file, ulog_level level) {
    (void)(file);
    (void)(level);
    warn_not_enabled("ULOG_BUILD_EXTRA_OUTPUTS");
    return ULOG_OUTPUT_INVALID;
}

ulog_status ulog_output_remove(ulog_output_id output) {
    (void)(output);
    warn_not_enabled("ULOG_BUILD_EXTRA_OUTPUTS");
    return ULOG_STATUS_ERROR;
}

#endif  // ULOG_HAS_WARN_NOT_ENABLED

#endif  // ULOG_HAS_EXTRA_OUTPUTS

/* ============================================================================
   Optional Feature: Dynamic Configuration - Topics
   (`topic_config_*`, depends on: - )
============================================================================ */
#if ULOG_HAS_DYNAMIC_CONFIG

typedef struct {
    bool enabled;
} topic_config;

static topic_config topic_cfg = {
    .enabled = (bool)ULOG_HAS_TOPICS,
};

// Private
// ================

bool topic_config_is_enabled(void) {
    return topic_cfg.enabled;
}

// Public
// ================

void ulog_topic_config(bool enabled) {
    lock_lock();  // Lock the configuration
    topic_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

void ulog_topic_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_DYNAMIC_CONFIG");
}

#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define topic_config_is_enabled() (ULOG_HAS_TOPICS)
#endif  // ULOG_HAS_DYNAMIC_CONFIG

/* ============================================================================
   Optional Feature: Topics
   (`topic_*`, depends on: Topics Config, Print, Levels)
============================================================================ */

#if ULOG_HAS_TOPICS

// Private
// ================
#define TOPIC_DYNAMIC (ULOG_BUILD_TOPICS_NUM < 0)
#define TOPIC_STATIC_NUM ULOG_BUILD_TOPICS_NUM
#define TOPIC_LEVEL_DEFAULT ULOG_LEVEL_TRACE

typedef struct {
    ulog_topic_id id;
    const char *name;
    bool enabled;
    ulog_level level;
    ulog_output_id output;

#if TOPIC_DYNAMIC
    void *next;  // Pointer to the next topic pointer (Topic **)
#endif

} topic_t;

typedef struct {
    bool new_topic_enabled;  // Whether new topics are enabled by default

#if TOPIC_DYNAMIC
    topic_t *topics;
#else
    topic_t topics[TOPIC_STATIC_NUM];
#endif

} topic_data_t;

static topic_data_t topic_data = {
    .new_topic_enabled = false,  // New topics are disabled by default

#if TOPIC_DYNAMIC
    .topics = NULL,  // No topics allocated by default
#else
    .topics = {{0}},  // Initialize static topics array to zero
#endif
};

// === Implementation specific functions for topics ===========================

/// @brief Converts a topic string to its ID
/// @param str - Topic string
/// @return Topic ID or ULOG_TOPIC_ID_INVALID if not found
static ulog_topic_id topic_str_to_id(const char *str);

/// @brief Gets the topic by ID
/// @param topic - Topic ID
/// @return Pointer to the topic if found, NULL otherwise
static topic_t *topic_get(ulog_topic_id topic);

/// @brief Enable all topics
/// @return ulog_status
static ulog_status topic_enable_all();

/// @brief Disable all topics
/// @return ulog_status
static ulog_status topic_disable_all();

/// @brief Add a new topic
/// @param topic_name - Topic name
/// @param output - Output id
/// @param enable - Whether the topic is enabled after creation
static ulog_topic_id topic_add(const char *topic_name, ulog_output_id output,
                               bool enable);

// === Common Topic Functions =================================================

static void topic_print(print_target *tgt, ulog_event *ev) {
    if (!topic_config_is_enabled()) {
        return;  // Topics are disabled, do nothing
    }

    topic_t *t = topic_get(ev->topic);
    if (t != NULL) {
        print_to_target(tgt, "[%s] ", t->name);
    }
}

/// @brief Sets the topic level
/// @param topic - Topic ID
/// @param level - Log level to set
/// @return ULOG_STATUS_OK if success, ULOG_STATUS_NOT_FOUND if topic not found
static ulog_status topic_set_level(int topic, ulog_level level) {
    if (level < LEVEL_MIN_VALUE || level >= ULOG_LEVEL_TOTAL) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        t->level = level;
        return ULOG_STATUS_OK;
    }
    return ULOG_STATUS_NOT_FOUND;
}

/// @brief Checks if the topic is loggable
/// @param t - Pointer to the topic, NULL is allowed
/// @param level - Log level to check against
/// @return true if loggable, false otherwise
static bool topic_is_loggable(topic_t *t, ulog_level level) {
    if (t == NULL) {
        return false;  // Topic not found, cannot log
    }
    if (!t->enabled || !level_is_allowed(level, t->level)) {
        return false;  // Topic is disabled, cannot log
    }
    return true;
}

/// @brief Enables the topic
/// @param topic - Topic ID
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_NOT_FOUND if topic not found
static ulog_status topic_enable(int topic) {
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        t->enabled = true;
        return ULOG_STATUS_OK;
    }
    return ULOG_STATUS_NOT_FOUND;
}

/// @brief Disables the topic
/// @param topic - Topic ID
/// @return ULOG_STATUS_OK on success, ULOG_STATUS_NOT_FOUND if topic not found
static ulog_status topic_disable(int topic) {
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        t->enabled = false;
        return ULOG_STATUS_OK;
    }
    return ULOG_STATUS_NOT_FOUND;
}

/// @brief Processes the topic
/// @param topic - Topic name
/// @param level - Log level
/// @param is_log_allowed - (Output) log allowed
/// @param topic_id - (Output) topic ID
/// @param output - (Output) topic output ID
static void topic_process(const char *topic, ulog_level level,
                          bool *is_log_allowed, int *topic_id,
                          ulog_output_id *output) {
    if (is_log_allowed == NULL || topic_id == NULL || output == NULL) {
        return;  // Invalid arguments, do nothing
    }

    topic_t *t = topic_get(topic_str_to_id(topic));

    *is_log_allowed = topic_is_loggable(t, level);
    if (!*is_log_allowed) {
        return;  // Topic is not loggable, stop processing
    }
    *topic_id = t->id;      // Set topic ID
    *output   = t->output;  // Set topic output
}

// Public
// ================

ulog_status ulog_topic_level_set(const char *topic_name, ulog_level level) {
    ulog_topic_id topic_id = ulog_topic_get_id(topic_name);
    if (topic_id == ULOG_TOPIC_ID_INVALID) {
        return ULOG_STATUS_NOT_FOUND;  // Topic not found, do nothing
    }
    return topic_set_level(topic_id, level);
}

ulog_status ulog_topic_enable(const char *topic_name) {
    ulog_topic_id topic_id = ulog_topic_get_id(topic_name);
    if (topic_id == ULOG_TOPIC_ID_INVALID) {
        return ULOG_STATUS_NOT_FOUND;  // Topic not found, do nothing
    }
    return topic_enable(topic_id);
}

ulog_status ulog_topic_disable(const char *topic_name) {
    ulog_topic_id topic_id = ulog_topic_get_id(topic_name);
    if (topic_id == ULOG_TOPIC_ID_INVALID) {
        return ULOG_STATUS_NOT_FOUND;  // Topic not found, do nothing
    }
    return topic_disable(topic_id);
}

ulog_status ulog_topic_enable_all(void) {
    topic_data.new_topic_enabled = true;
    return topic_enable_all();
}

ulog_status ulog_topic_disable_all(void) {
    topic_data.new_topic_enabled = false;
    return topic_disable_all();
}

ulog_topic_id ulog_topic_get_id(const char *topic_name) {
    return topic_str_to_id(topic_name);
}

ulog_topic_id ulog_topic_add(const char *topic_name, ulog_output_id output,
                             bool enable) {
    if (is_str_empty(topic_name)) {
        return ULOG_TOPIC_ID_INVALID;  // Invalid topic name, do nothing
    }
    return topic_add(topic_name, output, enable);
}

#else  // ULOG_HAS_TOPICS

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

ulog_status ulog_topic_level_set(const char *topic_name, ulog_level level) {
    (void)(topic_name);
    (void)(level);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_ERROR;
}

ulog_status ulog_topic_enable(const char *topic_name) {
    (void)(topic_name);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_ERROR;
}

ulog_status ulog_topic_disable(const char *topic_name) {
    (void)(topic_name);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_ERROR;
}

ulog_status ulog_topic_enable_all(void) {
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_ERROR;
}

ulog_status ulog_topic_disable_all(void) {
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_ERROR;
}

ulog_topic_id ulog_topic_get_id(const char *topic_name) {
    (void)(topic_name);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_TOPIC_ID_INVALID;
}

ulog_topic_id ulog_topic_add(const char *topic_name, ulog_output_id output,
                             bool enable) {
    (void)(topic_name);
    (void)(output);
    (void)(enable);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_TOPIC_ID_INVALID;
}

#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define topic_print(tgt, ev) (void)(tgt), (void)(ev)
#define topic_process(topic, level, is_log_allowed, topic_id, output)          \
    (void)(topic), (void)(level), (void)(is_log_allowed), (void)(topic_id),    \
        (void)(output)

#endif  // ULOG_HAS_TOPICS

/* ============================================================================
   Optional Feature: Topics - Static Allocation
   (`topic_*`, depends on: Topics)
============================================================================ */
#if ULOG_HAS_TOPICS && TOPIC_DYNAMIC == false
// Private
// ================

ulog_status topic_enable_all(void) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, no more to enable
        }
        topic_data.topics[i].enabled = true;
    }
    return ULOG_STATUS_OK;
}

ulog_status topic_disable_all(void) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, no more to disable
        }
        topic_data.topics[i].enabled = false;
    }
    return ULOG_STATUS_OK;
}

ulog_topic_id topic_str_to_id(const char *str) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, not found
        }
        if (strcmp(topic_data.topics[i].name, str) == 0) {
            return topic_data.topics[i].id;
        }
    }
    return ULOG_TOPIC_ID_INVALID;  // Not found
}

static topic_t *topic_get(ulog_topic_id topic) {
    if (topic < TOPIC_STATIC_NUM && topic >= 0) {
        return &topic_data.topics[topic];
    }
    return NULL;
}

static ulog_topic_id topic_add(const char *topic_name, ulog_output_id output,
                               bool enable) {
    if (is_str_empty(topic_name)) {
        return ULOG_TOPIC_ID_INVALID;
    }
    lock_lock();  // Lock the configuration
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        // If there is an empty slot
        if (is_str_empty(topic_data.topics[i].name)) {
            topic_data.topics[i].id      = i;
            topic_data.topics[i].name    = topic_name;
            topic_data.topics[i].enabled = enable;
            topic_data.topics[i].level   = TOPIC_LEVEL_DEFAULT;
            topic_data.topics[i].output  = output;
            lock_unlock();  // Unlock the configuration
            return i;
        }
        // If the topic already exists
        else if (strcmp(topic_data.topics[i].name, topic_name) == 0) {
            lock_unlock();  // Unlock the configuration
            return i;
        }
    }
    lock_unlock();                 // Unlock the configuration
    return ULOG_TOPIC_ID_INVALID;  // No space for new topics
}
#endif  // ULOG_HAS_TOPICS && TOPIC_DYNAMIC == false

/* ============================================================================
   Optional Feature: Topics - Dynamic Allocation
   (`topic_*`, depends on: Topics)
============================================================================ */

#if ULOG_HAS_TOPICS && TOPIC_DYNAMIC == true
// Private
// ================

static topic_t *topic_get_first(void) {
    return topic_data.topics;
}

static topic_t *topic_get_next(topic_t *t) {
    return t->next;
}

static topic_t *topic_get_last(void) {
    topic_t *last = topic_data.topics;
    if (last == NULL) {
        return NULL;  // No topics
    }
    while (last->next != NULL) {
        last = last->next;
    }
    return last;
}

static ulog_status topic_enable_all(void) {
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        t->enabled = true;
    }
    return ULOG_STATUS_OK;
}

static ulog_status topic_disable_all(void) {
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        t->enabled = false;
    }
    return ULOG_STATUS_OK;
}

ulog_topic_id topic_str_to_id(const char *str) {
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        if (!is_str_empty(t->name) && strcmp(t->name, str) == 0) {
            return t->id;
        }
    }
    return ULOG_TOPIC_ID_INVALID;
}

static topic_t *topic_get(int topic) {
    if (topic < 0) {
        return NULL;  // Invalid topic ID
    }
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        if (t->id == topic) {
            return t;
        }
    }
    return NULL;
}

static void *topic_allocate(int id, const char *topic_name,
                            ulog_output_id output, bool enable) {
    if (is_str_empty(topic_name)) {
        return NULL;  // Invalid topic name, do not allocate
    }
    if (id < 0) {
        return NULL;  // Invalid ID, do not allocate
    }
    topic_t *t = malloc(sizeof(topic_t));
    if (t != NULL) {
        t->id      = id;
        t->name    = topic_name;
        t->enabled = enable;
        t->level   = TOPIC_LEVEL_DEFAULT;
        t->output  = output;
        t->next    = NULL;
    }
    return t;
}

static ulog_topic_id topic_add(const char *topic_name, ulog_output_id output,
                               bool enable) {
    if (is_str_empty(topic_name)) {
        return ULOG_TOPIC_ID_INVALID;
    }

    // if exists
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        if (!is_str_empty(t->name) && strcmp(t->name, topic_name) == 0) {
            return t->id;
        }
    }

    // If the beginning is empty
    lock_lock();
    topic_t *t = topic_get_last();
    if (t == NULL) {
        topic_data.topics =
            (topic_t *)topic_allocate(0, topic_name, output, enable);
        if (topic_data.topics != NULL) {
            lock_unlock();
            return 0;
        }
        lock_unlock();
        return ULOG_TOPIC_ID_INVALID;
    }

    // If the beginning is not empty
    t->next = topic_allocate(t->id + 1, topic_name, output, enable);
    if (t->next) {
        lock_unlock();
        return t->id + 1;
    }
    lock_unlock();
    return ULOG_TOPIC_ID_INVALID;
}

#endif  // ULOG_HAS_TOPICS && TOPIC_DYNAMIC == true

/* ============================================================================
   Optional Feature: Dynamic Configuration - Source Location
   (`src_loc_config_*`, depends on: - )
============================================================================ */
#if ULOG_HAS_DYNAMIC_CONFIG

typedef struct {
    bool enabled;
} src_loc_config;

static src_loc_config src_loc_cfg = {
    .enabled = (bool)ULOG_HAS_SOURCE_LOCATION,
};

// Private
// ================

bool src_loc_config_is_enabled(void) {
    return src_loc_cfg.enabled;
}

// Public
// ================

void ulog_source_location_config(bool enabled) {
    lock_lock();  // Lock the configuration
    src_loc_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

void ulog_source_location_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_SOURCE_LOCATION");
}

#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define src_loc_config_is_enabled() (ULOG_HAS_SOURCE_LOCATION)
#endif  // ULOG_HAS_DYNAMIC_CONFIG

/* ============================================================================
   Core Feature: Log
   (`log_*`, depends on: Print, Level, Outputs, Extra Outputs, Prefix, Topics,
                         Time, Color, Locking, Source Location)
============================================================================ */

// Private
// ================

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void log_print_message(print_target *tgt, ulog_event *ev) {

    if (src_loc_config_is_enabled()) {
        print_to_target(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
    }

    if (!is_str_empty(ev->message)) {
        print_to_target_valist(tgt, ev->message,
                               ev->message_format_args);  // message
    } else {
        print_to_target(tgt, "NULL");  // message
    }
}

/// @brief Writes a formatted message
/// @details The message is formatted as follows:
///
/// [Time][Prefix][Topic]Level [File: ]Message
/// or
/// [Time ][Topic ]Level [File: ]Message
///
/// where [Entry] is an optional part
///
/// @param tgt - Target
/// @param ev - Event
/// @param full_time - Full time or short time
/// @param color - Color or no color
/// @param new_line - New line in the end or no new line
static void log_print_event(print_target *tgt, ulog_event *ev, bool full_time,
                            bool color, bool new_line) {

    color ? color_print_start(tgt, ev) : (void)0;

    bool append_space = true;
    (void)append_space;  // May be unused if no prefix and time
#if ULOG_HAS_PREFIX
    if (prefix_data.function != NULL) {
        append_space = false;  // Prefix does not need leading space
    }
#endif

    full_time ? time_print_full(tgt, ev, append_space)
              : time_print_short(tgt, ev, append_space);

    prefix_print(tgt, ev);
    level_print(tgt, ev);
    topic_print(tgt, ev);
    log_print_message(tgt, ev);

    color ? color_print_end(tgt) : (void)0;
    new_line ? print_to_target(tgt, "\n") : (void)0;
}

void log_fill_event(ulog_event *ev, const char *message, ulog_level level,
                    const char *file, int line, int topic_id) {
    if (ev == NULL) {
        return;  // Invalid event, do nothing
    }

    ev->message = message;
    ev->level   = level;

#if ULOG_HAS_SOURCE_LOCATION
    ev->file = file;
    ev->line = line;
#else
    (void)(file), (void)(line);  // Unused if file string is disabled
#endif  // ULOG_HAS_SOURCE_LOCATION

#if ULOG_HAS_TOPICS
    ev->topic = topic_id;
#else
    (void)(topic_id);  // Unused if topics are disabled
#endif

#if ULOG_HAS_TIME
    ev->time = NULL;  // Time will be filled later
#endif

    time_fill_current_time(ev);  // Fill time with current value
}

// Public
// ================

ulog_status ulog_event_to_cstr(ulog_event *ev, char *out, size_t out_size) {
    if (out == NULL || out_size == 0) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    print_target tgt = {.type       = PRINT_TARGET_BUFFER,
                        .dsc.buffer = {out, 0, out_size}};
    log_print_event(&tgt, ev, false, false, false);
    return ULOG_STATUS_OK;
}

void ulog_log(ulog_level level, const char *file, int line, const char *topic,
              const char *message, ...) {

    lock_lock();

    // Try to get topic ID, outputs and check if logging is allowed for this
    // topic
    ulog_output_id output = ULOG_OUTPUT_ALL;
    int topic_id          = -1;
    if (!is_str_empty(topic)) {
        bool is_log_allowed = false;
        topic_process(topic, level, &is_log_allowed, &topic_id, &output);
        if (!is_log_allowed) {
            lock_unlock();
            return;  // Topic is not enabled or level is lower than topic level
        }
    }

    ulog_event ev = {0};
    log_fill_event(&ev, message, level, file, line, topic_id);
    va_start(ev.message_format_args, message);

    prefix_update(&ev);

    // Handle output routing
    if (output == ULOG_OUTPUT_ALL) {
        output_handle_all(&ev);
    } else {
        output_handle_by_id(&ev, output);
    }

    va_end(ev.message_format_args);

    lock_unlock();
}
