// *************************************************************************
//
// ulog v@ULOG_VERSION@ - A simple customizable logging library.
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

| Build Option                      | Default   | Dependent Macro(s)            | Purpose                  |
| --------------------------------- | --------- | ----------------------------- | ------------------------ |
| ULOG_BUILD_COLOR                  | 0         | ULOG_HAS_COLOR                | Compile color code paths |
| ULOG_BUILD_PREFIX_SIZE            | 0         | ULOG_HAS_PREFIX               | Prefix buffer logic      |
| ULOG_BUILD_EXTRA_OUTPUTS          | 0         | ULOG_HAS_EXTRA_OUTPUTS        | Extra output backends    |
| ULOG_BUILD_SOURCE_LOCATION        | 1         | ULOG_HAS_SOURCE_LOCATION      | File\:line output        |
| ULOG_BUILD_LEVEL_SHORT            | 0         | ULOG_LEVEL_HAS_SHORT/_LONG    | Short level style        |
| ULOG_BUILD_TIME                   | 0         | ULOG_HAS_TIME                 | Timestamp support        |
| ULOG_BUILD_TOPICS_NUM             | 0         | ULOG_HAS_TOPICS               | Topic filtering logic    |
| ULOG_BUILD_DYNAMIC_CONFIG         | 0         | ULOG_HAS_DYNAMIC_CONFIG       | Runtime toggles          |
| ULOG_BUILD_WARN_NOT_ENABLED       | 1         | ULOG_HAS_WARN_NOT_ENABLED     | Warning stubs            |
| ULOG_BUILD_STATIC_CONFIG_HEADER   | 0         | -                             | Configuration header     |

===================================================================================================================== */

#ifdef ULOG_BUILD_STATIC_CONFIG_HEADER

    // If ULOG_BUILD_STATIC_CONFIG_HEADER is defined, no other ULOG_BUILD_* macros should be defined to avoid conflicts
    #ifdef ULOG_BUILD_COLOR
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_COLOR"
    #endif
    #ifdef ULOG_BUILD_PREFIX_SIZE
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_PREFIX_SIZE"
    #endif
    #ifdef ULOG_BUILD_EXTRA_OUTPUTS
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_EXTRA_OUTPUTS"
    #endif
    #ifdef ULOG_BUILD_SOURCE_LOCATION
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_SOURCE_LOCATION"
    #endif
    #ifdef ULOG_BUILD_LEVEL_SHORT
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_LEVEL_SHORT"
    #endif
    #ifdef ULOG_BUILD_TIME
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_TIME"
    #endif
    #ifdef ULOG_BUILD_TOPICS_NUM
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_TOPICS_NUM"
    #endif
    #ifdef ULOG_BUILD_DYNAMIC_CONFIG
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_DYNAMIC_CONFIG"
    #endif
    #ifdef ULOG_BUILD_WARN_NOT_ENABLED
        #error "ULOG_BUILD_STATIC_CONFIG_HEADER cannot be used with ULOG_BUILD_WARN_NOT_ENABLED"
    #endif

    // The user provided configuration header
    #include "ulog_static_config.h"
#endif

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


#ifndef ULOG_BUILD_LEVEL_SHORT
    #define ULOG_HAS_LEVEL_LONG  1
    #define ULOG_HAS_LEVEL_SHORT 0
#else
    #define ULOG_HAS_LEVEL_LONG  !ULOG_BUILD_LEVEL_SHORT
    #define ULOG_HAS_LEVEL_SHORT ULOG_BUILD_LEVEL_SHORT
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

/* ============================================================================
   Core Feature: Warn Not Enabled
   (`warn_not_enabled`, depends on: - )
============================================================================ */
#if ULOG_HAS_WARN_NOT_ENABLED

// Macro to log a warning when a feature is not enabled
// Usage: warn_not_enabled("ULOG_BUILD_TIME")
// Output:
//   WARN src/main.c:42: 'ulog_configure_time' ignored: ULOG_BUILD_TIME disabled
#define warn_not_enabled(feature)                                              \
    warn_non_enabled_full(__func__, feature, __FILE__, __LINE__)

#define warn_non_enabled_full(func, feature, file, line)                       \
    ulog_log(ULOG_LEVEL_WARN, file, line, NULL,                                \
             "'%s' called with %s disabled", func, feature)

#endif  // ULOG_HAS_WARN_NOT_ENABLED
/* ============================================================================
   Core Feature: Print
   (`print_*`, depends on: - )
============================================================================ */

//  Private
// ================

typedef struct {
    char *data;
    size_t curr_pos;
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
        print_buffer *buf = &tgt->dsc.buffer;

        if (buf->curr_pos >= buf->size) {
            return;  // No space available
        }

        size_t remaining = buf->size - buf->curr_pos;
        char *write_pos  = buf->data + buf->curr_pos;

        int written = vsnprintf(write_pos, remaining, format, args);
        if (written < 0) {
            return;  // Encoding error
        }

        // Update position, capping at buffer end
        if ((size_t)written >= remaining) {
            buf->curr_pos = buf->size;
        } else {
            buf->curr_pos += written;
        }

    } else if (tgt->type == PRINT_TARGET_STREAM) {
        vfprintf(tgt->dsc.stream, format, args);
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

    // Create a copy of the event to avoid va_list issues
    ulog_event ev_copy = *ev;
    va_copy(ev_copy.message_format_args, ev->message_format_args);

    log_print_message(&tgt, &ev_copy);

    va_end(ev_copy.message_format_args);
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

static ulog_status lock_lock(void) {
    if (lock_data.function != NULL) {
        return lock_data.function(true, lock_data.args);
    }
    return ULOG_STATUS_OK;
}

static ulog_status lock_unlock(void) {
    if (lock_data.function != NULL) {
        return lock_data.function(false, lock_data.args);
    }
    return ULOG_STATUS_OK;
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

ulog_status ulog_color_config(bool enabled) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;  // Failed to acquire lock
    }
    color_cfg.enabled = enabled;
    return lock_unlock();
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
ulog_status ulog_color_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_COLOR");
    return ULOG_STATUS_DISABLED;
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

// From https://en.wikipedia.org/wiki/ANSI_escape_code#8-bit
// ANSI color codes from least to most attention-grabbing
static const char *color_levels[] = {
    "\x1b[m",            // LEVEL_0: Color reset (default)
    "\x1b[36m",          // LEVEL_1: Color (Cyan)
    "\x1b[32m",          // LEVEL_2: Color (Green)
    "\x1b[33m",          // LEVEL_3: Color (Yellow)
    "\x1b[31m",          // LEVEL_4: Color (Red)
    "\x1b[31m\x1b[47m",  // LEVEL_5: Color (Red on White)
    "\x1b[43m\x1b[31m",  // LEVEL_6: Color (Yellow on Red)
    "\x1b[41m\x1b[97m",  // LEVEL_7: Color (White on Red)
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

ulog_status ulog_prefix_config(bool enabled) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;
    }
    prefix_cfg.enabled = enabled;
    return lock_unlock();
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
ulog_status ulog_prefix_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_DYNAMIC_CONFIG");
    return ULOG_STATUS_DISABLED;
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

static void prefix_print(print_target *tgt) {
    if (prefix_data.function == NULL || !prefix_config_is_enabled()) {
        return;
    }
    print_to_target(tgt, "%s", prefix_data.prefix);
}

// Public
// ================

ulog_status ulog_prefix_set_fn(ulog_prefix_fn function) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;
    }
    if (function == NULL) {
        return ULOG_STATUS_INVALID_ARGUMENT;  // Ignore NULL function
    }
    prefix_data.function = function;
    return lock_unlock();
}

#else  // ULOG_HAS_PREFIX

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
ulog_status ulog_prefix_set_fn(ulog_prefix_fn function) {
    (void)(function);
    warn_not_enabled("ULOG_BUILD_PREFIX_SIZE");
    return ULOG_STATUS_DISABLED;
}
#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define prefix_print(tgt) (void)(tgt)
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

ulog_status ulog_time_config(bool enabled) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;
    }
    time_cfg.enabled = enabled;
    return lock_unlock();
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
ulog_status ulog_time_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_TIME");
    return ULOG_STATUS_DISABLED;
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
   Core Feature: Levels
   (`level_*`, depends on: Levels Config, Print)
============================================================================ */

// Private
// ================
// clang-format off
#define LEVEL_MIN_VALUE 0

#define LEVEL_NAMES_SHORT {"T", "D", "I", "W", "E", "F", NULL, NULL}
#define LEVEL_NAMES_LONG  {"TRACE", "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL", NULL, NULL}

#if ULOG_HAS_LEVEL_SHORT && !ULOG_HAS_LEVEL_LONG
    #define LEVEL_NAMES_DEFAULT LEVEL_NAMES_SHORT
#else // ULOG_HAS_LEVEL_LONG or both
    #define LEVEL_NAMES_DEFAULT LEVEL_NAMES_LONG
#endif
// clang-format on

typedef struct {
    const ulog_level_descriptor *dsc;
} level_data_t;

const ulog_level_descriptor level_names_default = {
    .max_level = ULOG_LEVEL_FATAL,
    .names     = LEVEL_NAMES_DEFAULT,
};

level_data_t level_data = {
    .dsc = &level_names_default,
};

static bool level_is_allowed(ulog_level msg_level, ulog_level log_verbosity) {
    if (msg_level < log_verbosity || msg_level < LEVEL_MIN_VALUE) {
        return false;  // Level is higher than the configured level, not allowed
    }

    return true;  // Level is allowed
}

static bool level_is_valid(ulog_level level) {
    return (level >= LEVEL_MIN_VALUE && level <= level_data.dsc->max_level);
}

static void level_print(print_target *tgt, ulog_event *ev) {
    print_to_target(tgt, "%s ", level_data.dsc->names[ev->level]);
}

// Public
// ================

/// @brief Returns the string representation of the level
const char *ulog_level_to_string(ulog_level level) {
    if (level < LEVEL_MIN_VALUE || level >= level_data.dsc->max_level) {
        return "?";  // Return a default string for invalid levels
    }

    return level_data.dsc->names[level];
}

ulog_status ulog_level_set_new_levels(const ulog_level_descriptor *new_levels) {
    if (new_levels == NULL || new_levels->names[0] == NULL ||
        new_levels->max_level <= LEVEL_MIN_VALUE) {
        return ULOG_STATUS_INVALID_ARGUMENT;  // Invalid argument
    }
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;  // Failed to acquire lock
    }

    level_data.dsc = new_levels;
    return lock_unlock();
}

ulog_status ulog_level_reset_levels(void) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;  // Failed to acquire lock
    }

    level_data.dsc = &level_names_default;
    return lock_unlock();
}

/* ============================================================================
   Optional Feature: Dynamic Configuration - Level
   (`level_config_*`, depends on: - )
============================================================================ */
#if ULOG_HAS_DYNAMIC_CONFIG

// Private
// ================

typedef struct {
    bool short_style;  // Use short level strings
} level_config;

static level_config level_cfg = {
    .short_style = false,
};

const ulog_level_descriptor level_names_default_short = {
    .max_level = ULOG_LEVEL_FATAL,
    .names     = LEVEL_NAMES_SHORT,
};

bool level_config_is_short(void) {
    return level_cfg.short_style;
}

// Public
// ================

ulog_status ulog_level_config(ulog_level_config_style style) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;
    }
    level_cfg.short_style = (style == ULOG_LEVEL_CONFIG_STYLE_SHORT);
    if (level_cfg.short_style) {
        level_data.dsc = &level_names_default_short;
    } else {
        level_data.dsc = &level_names_default;
    }
    return lock_unlock();
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

ulog_status ulog_level_config(ulog_level_config_style style) {
    (void)(style);
    warn_not_enabled("ULOG_BUILD_DYNAMIC_CONFIG");
    return ULOG_STATUS_DISABLED;
}

#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define level_config_is_short(void) (ULOG_HAS_LEVEL_SHORT)
#endif  // ULOG_HAS_DYNAMIC_CONFIG

/* ============================================================================
   Core Feature: Outputs
   (`output_*`, depends on: Print, Log, Level)
============================================================================ */

//  Private
// ================
#if ULOG_HAS_EXTRA_OUTPUTS
#define OUTPUT_TOTAL_NUM (1 + ULOG_BUILD_EXTRA_OUTPUTS)  // stdout + extra
#else
#define OUTPUT_TOTAL_NUM 1  // Only stdout
#endif                      // ULOG_HAS_EXTRA_OUTPUTS

#define OUTPUT_STDOUT_DEFAULT_LEVEL ULOG_LEVEL_TRACE

// Prototypes
static void output_stdout_handler(ulog_event *ev, void *arg);
static void log_print_event(print_target *tgt, ulog_event *ev, bool full_time,
                            bool color, bool new_line);

typedef struct {
    ulog_output_handler_fn handler;
    void *arg;
    ulog_level level;
} output;

typedef struct {
    output outputs[OUTPUT_TOTAL_NUM];  // order num = id. 0 is for stdout
} output_data_t;

static output_data_t output_data = {
    .outputs = {{output_stdout_handler, NULL, OUTPUT_STDOUT_DEFAULT_LEVEL}}};

static void output_handle_single(ulog_event *ev, output *output) {
    if (output->handler == NULL) {
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
        output->handler(&ev_copy, output->arg);
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
    for (int i = 0; (i < OUTPUT_TOTAL_NUM); i++) {
        output_handle_single(ev, &output_data.outputs[i]);
    }
}

static void output_stdout_handler(ulog_event *ev, void *arg) {
    (void)(arg);  // Unused
    print_target tgt = {.type = PRINT_TARGET_STREAM, .dsc.stream = stdout};
    log_print_event(&tgt, ev, false, true, true);
}

// Public
// ================

ulog_status ulog_output_level_set(ulog_output_id output, ulog_level level) {
    if (!level_is_valid(level)) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    if (output < ULOG_OUTPUT_STDOUT || output >= OUTPUT_TOTAL_NUM) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    if (output_data.outputs[output].handler == NULL) {
        return ULOG_STATUS_NOT_FOUND;  // Output exists but no handler assigned
    }
    output_data.outputs[output].level = level;
    return ULOG_STATUS_OK;
}

ulog_status ulog_output_level_set_all(ulog_level level) {
    if (!level_is_valid(level)) {
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
static void output_file_handler(ulog_event *ev, void *arg) {
    print_target tgt = {.type = PRINT_TARGET_STREAM, .dsc.stream = (FILE *)arg};
    log_print_event(&tgt, ev, true, false, true);
}

// Public
// ================

ulog_output_id ulog_output_add(ulog_output_handler_fn handler, void *arg,
                               ulog_level level) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_OUTPUT_INVALID;
    }
    for (int i = 0; i < OUTPUT_TOTAL_NUM; i++) {
        if (output_data.outputs[i].handler == NULL) {
            output_data.outputs[i] = (output){handler, arg, level};
            (void)lock_unlock();
            return i;
        }
    }
    (void)lock_unlock();
    return ULOG_OUTPUT_INVALID;
}

/// @brief Add file handler
ulog_output_id ulog_output_add_file(FILE *file, ulog_level level) {
    return ulog_output_add(output_file_handler, file, level);
}

/// @brief Remove an output from the logging system
ulog_status ulog_output_remove(ulog_output_id output) {
    if (output < 0 || output >= OUTPUT_TOTAL_NUM) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }

    if (output == ULOG_OUTPUT_STDOUT) {
        return ULOG_STATUS_ERROR;  // Cannot remove stdout output
    }

    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;
    }
    if (output_data.outputs[output].handler == NULL) {
        if (lock_unlock() != ULOG_STATUS_OK) {
            return ULOG_STATUS_BUSY;
        }
        return ULOG_STATUS_NOT_FOUND;  // Output not found or already removed
    }

    // Mark output as removed by setting handler to NULL
    output_data.outputs[output].handler = NULL;
    output_data.outputs[output].arg     = NULL;
    output_data.outputs[output].level   = ULOG_LEVEL_TRACE;

    return lock_unlock();
}

#else  // ULOG_HAS_EXTRA_OUTPUTS

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

ulog_output_id ulog_output_add(ulog_output_handler_fn handler, void *arg,
                               ulog_level level) {
    (void)(handler);
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
    return ULOG_STATUS_DISABLED;
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

ulog_status ulog_topic_config(bool enabled) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;
    }
    topic_cfg.enabled = enabled;
    return lock_unlock();
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

ulog_status ulog_topic_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_DYNAMIC_CONFIG");
    return ULOG_STATUS_DISABLED;
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
#define TOPIC_IS_DYNAMIC (ULOG_BUILD_TOPICS_NUM < 0)
#define TOPIC_STATIC_NUM ULOG_BUILD_TOPICS_NUM
#define TOPIC_LEVEL_DEFAULT ULOG_LEVEL_TRACE

typedef struct topic_t {
    ulog_topic_id id;
    const char *name;
    bool enabled;
    ulog_level level;
    ulog_output_id output;

#if TOPIC_IS_DYNAMIC
    struct topic_t *next;  // Pointer to the next topic
#endif

} topic_t;

typedef struct {
    bool new_topic_enabled;  // Whether new topics are enabled by default

#if TOPIC_IS_DYNAMIC
    topic_t *topics;
#else
    topic_t topics[TOPIC_STATIC_NUM];
#endif

} topic_data_t;

static topic_data_t topic_data = {
    .new_topic_enabled = false,  // New topics are disabled by default

#if TOPIC_IS_DYNAMIC
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
static ulog_status topic_enable_all(void);

/// @brief Disable all topics
/// @return ulog_status
static ulog_status topic_disable_all(void);

/// @brief Add a new topic
/// @param topic_name - Topic name
/// @param output - Output id
/// @param enable - Whether the topic is enabled after creation
static ulog_topic_id topic_add(const char *topic_name, ulog_output_id output,
                               bool enable);

/// @brief Remove a topic by name
/// @param topic_name - Topic name
/// @return ulog_status
static ulog_status topic_remove(const char *topic_name);

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
    if (!level_is_valid(level)) {
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

ulog_status ulog_topic_remove(const char *topic_name) {
    if (is_str_empty(topic_name)) {
        return ULOG_STATUS_INVALID_ARGUMENT;  // Invalid topic name, do nothing
    }
    return topic_remove(topic_name);
}

#else  // ULOG_HAS_TOPICS

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

ulog_status ulog_topic_level_set(const char *topic_name, ulog_level level) {
    (void)(topic_name);
    (void)(level);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_DISABLED;
}

ulog_status ulog_topic_enable(const char *topic_name) {
    (void)(topic_name);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_DISABLED;
}

ulog_status ulog_topic_disable(const char *topic_name) {
    (void)(topic_name);
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_DISABLED;
}

ulog_status ulog_topic_enable_all(void) {
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_DISABLED;
}

ulog_status ulog_topic_disable_all(void) {
    warn_not_enabled("ULOG_BUILD_TOPICS_NUM");
    return ULOG_STATUS_DISABLED;
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
#if ULOG_HAS_TOPICS && TOPIC_IS_DYNAMIC == false
// Private
// ================

ulog_status topic_enable_all(void) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            continue;  // Skip empty slot; do not break to allow sparse reuse
        }
        topic_data.topics[i].enabled = true;
    }
    return ULOG_STATUS_OK;
}

ulog_status topic_disable_all(void) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            continue;  // Skip empty slot; allow sparse arrays
        }
        topic_data.topics[i].enabled = false;
    }
    return ULOG_STATUS_OK;
}

ulog_topic_id topic_str_to_id(const char *str) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            continue;  // Skip empty slot; continue searching
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
    if (lock_lock() != ULOG_STATUS_OK) {  // Lock the configuration
        return ULOG_TOPIC_ID_INVALID;
    }
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        // If there is an empty slot
        if (is_str_empty(topic_data.topics[i].name)) {
            topic_data.topics[i].id      = i;
            topic_data.topics[i].name    = topic_name;
            topic_data.topics[i].enabled = enable;
            topic_data.topics[i].level   = TOPIC_LEVEL_DEFAULT;
            topic_data.topics[i].output  = output;
            (void)lock_unlock();  // Unlock the configuration
            return i;
        }
        // If the topic already exists
        else if (strcmp(topic_data.topics[i].name, topic_name) == 0) {
            (void)lock_unlock();  // Unlock the configuration
            return i;
        }
    }
    (void)lock_unlock();           // Unlock the configuration
    return ULOG_TOPIC_ID_INVALID;  // No space for new topics
}

static ulog_status topic_remove(const char *topic_name) {
    if (is_str_empty(topic_name)) {
        return ULOG_STATUS_INVALID_ARGUMENT;  // Invalid topic name, do nothing
    }
    if (lock_lock() != ULOG_STATUS_OK) {  // Lock the configuration
        return ULOG_STATUS_BUSY;
    }
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            continue;  // Skip empty slot; continue search
        }
        if (strcmp(topic_data.topics[i].name, topic_name) == 0) {
            // Clear the topic entry
            topic_data.topics[i] = (topic_t){0};
            return lock_unlock();
        }
    }
    if (lock_unlock() != ULOG_STATUS_OK) {  // Unlock the configuration
        return ULOG_STATUS_BUSY;
    }
    return ULOG_STATUS_NOT_FOUND;  // Topic not found
}
#endif  // ULOG_HAS_TOPICS && TOPIC_IS_DYNAMIC == false

/* ============================================================================
   Optional Feature: Topics - Dynamic Allocation
   (`topic_*`, depends on: Topics)
============================================================================ */

#if ULOG_HAS_TOPICS && TOPIC_IS_DYNAMIC == true
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

static topic_t *topic_allocate(int id, const char *topic_name,
                               ulog_output_id output, bool enable) {
    if (is_str_empty(topic_name)) {
        return NULL;  // Invalid topic name, do not allocate
    }
    if (id < 0) {
        return NULL;  // Invalid ID, do not allocate
    }
    topic_t *t = malloc(sizeof(topic_t));
    if (t != NULL) {
        // Allocate memory for the topic name and copy it
        size_t name_len = strlen(topic_name) + 1;
        char *name_copy = malloc(name_len);
        if (name_copy == NULL) {
            free(t);
            return NULL;  // Failed to allocate memory for name
        }
        strcpy(name_copy, topic_name);

        t->id      = id;
        t->name    = name_copy;
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
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_TOPIC_ID_INVALID;
    }
    topic_t *t = topic_get_last();
    if (t == NULL) {
        topic_data.topics = topic_allocate(0, topic_name, output, enable);
        if (topic_data.topics != NULL) {
            (void)lock_unlock();
            return 0;
        }
        (void)lock_unlock();
        return ULOG_TOPIC_ID_INVALID;
    }

    // If the beginning is not empty
    t->next = topic_allocate(t->id + 1, topic_name, output, enable);
    if (t->next) {
        (void)lock_unlock();
        return t->id + 1;
    }
    (void)lock_unlock();
    return ULOG_TOPIC_ID_INVALID;
}

static ulog_status topic_remove(const char *topic_name) {
    if (is_str_empty(topic_name)) {
        return ULOG_STATUS_INVALID_ARGUMENT;  // Invalid topic name, do nothing
    }
    if (lock_lock() != ULOG_STATUS_OK) {  // Lock the configuration
        return ULOG_STATUS_BUSY;
    }

    topic_t *t      = topic_get_first();
    topic_t *t_prev = NULL;

    while (t != NULL) {
        if (!is_str_empty(t->name) && strcmp(t->name, topic_name) == 0) {
            // Found the topic to remove
            if (t_prev == NULL) {
                // Removing the first topic
                topic_data.topics = t->next;
            } else {
                t_prev->next = t->next;
            }
            free((void *)t->name);  // Free the allocated topic name
            free(t);                // Free the topic memory
            return lock_unlock();
        }
        t_prev = t;
        t      = t->next;
    }

    if (lock_unlock() != ULOG_STATUS_OK) {  // Unlock the configuration
        return ULOG_STATUS_BUSY;
    }
    return ULOG_STATUS_NOT_FOUND;  // Topic not found
}

#endif  // ULOG_HAS_TOPICS && TOPIC_IS_DYNAMIC == true

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

ulog_status ulog_source_location_config(bool enabled) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return ULOG_STATUS_BUSY;
    }
    src_loc_cfg.enabled = enabled;
    return lock_unlock();
}

#else  // ULOG_HAS_DYNAMIC_CONFIG

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED

ulog_status ulog_source_location_config(bool enabled) {
    (void)(enabled);
    warn_not_enabled("ULOG_BUILD_SOURCE_LOCATION");
    return ULOG_STATUS_DISABLED;
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

#if ULOG_HAS_SOURCE_LOCATION
    if (src_loc_config_is_enabled() && ev->file != NULL) {
        print_to_target(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
    }
#endif  // ULOG_HAS_SOURCE_LOCATION

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

    prefix_print(tgt);
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
    if (ev == NULL || out == NULL || out_size == 0) {
        return ULOG_STATUS_INVALID_ARGUMENT;
    }
    print_target tgt = {.type       = PRINT_TARGET_BUFFER,
                        .dsc.buffer = {out, 0, out_size}};

    // Create a copy of the event to avoid va_list issues
    ulog_event ev_copy = *ev;
    va_copy(ev_copy.message_format_args, ev->message_format_args);

    log_print_event(&tgt, &ev_copy, false, false, false);

    va_end(ev_copy.message_format_args);
    return ULOG_STATUS_OK;
}

void ulog_log(ulog_level level, const char *file, int line, const char *topic,
              const char *message, ...) {
    if (lock_lock() != ULOG_STATUS_OK) {
        return;  // Failed to acquire lock, drop log
    }

    // Try to get topic ID, outputs and check if logging is allowed for this
    // topic
    ulog_output_id output = ULOG_OUTPUT_ALL;
    int topic_id          = -1;
    if (!is_str_empty(topic)) {
        bool is_log_allowed = false;
        topic_process(topic, level, &is_log_allowed, &topic_id, &output);
        if (!is_log_allowed) {
            (void)lock_unlock();
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

    (void)lock_unlock();
}

/* ============================================================================
   Core Feature: Clean up
   (`init_*`, depends on: Locking, Outputs, Prefix, Time, Color)
============================================================================ */

// Public
// ================

ulog_status ulog_cleanup(void) {
    if (lock_lock() != ULOG_STATUS_OK) {  // Lock the configuration
        return ULOG_STATUS_BUSY;
    }
    // Cleanup Topics

    // TODO: this section can be improved with topic_remove_all() function
#if ULOG_HAS_TOPICS
    // Reset new-topic default enable flag
    topic_data.new_topic_enabled = false;
#if TOPIC_IS_DYNAMIC
    // Free linked list of dynamically allocated topics
    topic_t *t = topic_data.topics;
    while (t != NULL) {
        topic_t *next = t->next;
        free((void *)t->name);  // Free the allocated topic name
        free(t);
        t = next;
    }
    topic_data.topics = NULL;
#else
    // Zero out statically allocated topic array
    memset(topic_data.topics, 0, sizeof(topic_data.topics));
#endif
#endif  // ULOG_HAS_TOPICS

    // Cleanup Outputs (keep stdout (index 0) registered but reset its level)
    output_data.outputs[ULOG_OUTPUT_STDOUT].level = OUTPUT_STDOUT_DEFAULT_LEVEL;
#if ULOG_HAS_EXTRA_OUTPUTS
    for (int i = 1; i < OUTPUT_TOTAL_NUM; i++) {
        output_data.outputs[i].handler = NULL;
        output_data.outputs[i].arg     = NULL;
        output_data.outputs[i].level   = OUTPUT_STDOUT_DEFAULT_LEVEL;
    }
#endif  // ULOG_HAS_EXTRA_OUTPUTS

#if ULOG_HAS_PREFIX
    // Reset prefix state
    prefix_data.function = NULL;
    memset(prefix_data.prefix, 0, sizeof(prefix_data.prefix));
#endif

    return lock_unlock();
}
