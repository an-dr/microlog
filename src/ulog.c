// *************************************************************************
//
// ulog v6.4.1 - A simple customizable logging library.
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

#include <string.h>
#include "ulog.h"

#define ULOG_NEW_LINE_ON true
#define ULOG_NEW_LINE_OFF false
#define ULOG_COLOR_ON true
#define ULOG_COLOR_OFF false
#define ULOG_TIME_FULL true
#define ULOG_TIME_SHORT false

#ifndef ULOG_DEFAULT_LOG_LEVEL
#define ULOG_DEFAULT_LOG_LEVEL LOG_TRACE
#endif

/* ============================================================================
   Main logger object
============================================================================ */

/// @brief Callback structure
///
typedef struct {
    ulog_LogFn function;  // Callback function
    void *arg;            // Any argument that will be passed to the event
    int level;            // Debug level
} Callback;

/// @brief Logger object
typedef struct {
    ulog_LockFn lock_function;  // Mutex function
    void *lock_arg;             // Mutex argument
    int level;                  // Debug level
    bool quiet;                 // Quiet mode
    Callback callback_stdout;   // to stdout

#if FEATURE_EXTRA_OUTPUTS
    Callback callbacks[CFG_EXTRA_OUTPUTS];  // Extra callbacks
#endif

#if FEATURE_CUSTOM_PREFIX
    ulog_PrefixFn update_prefix_function;        // Custom prefix function
    char custom_prefix[CFG_CUSTOM_PREFIX_SIZE];  // Custom prefix
#endif

} ulog_t;

/// @brief Main logger object himself
static ulog_t ulog = {
    .lock_function   = NULL,
    .lock_arg        = NULL,
    .level           = ULOG_DEFAULT_LOG_LEVEL,
    .quiet           = false,
    .callback_stdout = {0},

#if FEATURE_EXTRA_OUTPUTS
    .callbacks = {{0}},
#endif

#if FEATURE_CUSTOM_PREFIX
    .update_prefix_function = NULL,
    .custom_prefix          = {0},
#endif

};

/* ============================================================================
   Output Printing
============================================================================ */

typedef struct {
    char *data;
    unsigned int curr_pos;
    size_t size;
} buffer_descriptor;

typedef union {
    buffer_descriptor buffer;
    FILE *stream;
} log_target_descriptor;

typedef enum { T_BUFFER, T_STREAM } log_target_type;

typedef struct {
    log_target_type type;
    log_target_descriptor dsc;
} log_target;

static void vprint(log_target *tgt, const char *format, va_list args) {
    if (tgt->type == T_BUFFER) {
        char *buf   = tgt->dsc.buffer.data + tgt->dsc.buffer.curr_pos;
        size_t size = tgt->dsc.buffer.size - tgt->dsc.buffer.curr_pos;
        if (size > 0) {
            tgt->dsc.buffer.curr_pos += vsnprintf(buf, size, format, args);
        }
    } else if (tgt->type == T_STREAM) {
        FILE *stream = tgt->dsc.stream;
        vfprintf(stream, format, args);
    }
}

static void print(log_target *tgt, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprint(tgt, format, args);
    va_end(args);
}

/* ============================================================================
   Prototypes
============================================================================ */

static void print_level(log_target *tgt, ulog_Event *ev);
static void print_message(log_target *tgt, ulog_Event *ev);
static void process_callback(ulog_Event *ev, Callback *cb);
static void write_formatted_message(log_target *tgt, ulog_Event *ev,
                                    bool full_time, bool color, bool new_line);

/* ============================================================================
   Feature: Color
============================================================================ */
#if FEATURE_COLOR

/// @brief Level colors
static const char *level_colors[] = {
    "\x1b[37m",  // TRACE : White #000
    "\x1b[36m",  // DEBUG : Cyan #0ff
    "\x1b[32m",  // INFO : Green #0f0
    "\x1b[33m",  // WARN : Yellow #ff0
    "\x1b[31m",  // ERROR : Red #f00
    "\x1b[35m"   // FATAL : Magenta #f0f
};

static void print_color_start(log_target *tgt, ulog_Event *ev) {
    (void)ev;
    print(tgt, "%s", level_colors[ev->level]);  // color start
}

static void print_color_end(log_target *tgt, ulog_Event *ev) {
    (void)ev;
    print(tgt, "\x1b[0m");  // color end
}

#endif  // FEATURE_COLOR

/* ============================================================================
   Feature: Time
============================================================================ */
#if FEATURE_TIME

static void print_time_sec(log_target *tgt, ulog_Event *ev) {

#if FEATURE_CUSTOM_PREFIX
    char buf[9];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
#else   // FEATURE_CUSTOM_PREFIX
    char buf[10];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S ", ev->time)] = '\0';
#endif  // FEATURE_CUSTOM_PREFIX

    print(tgt, "%s", buf);
}

#if FEATURE_EXTRA_OUTPUTS
static void print_time_full(log_target *tgt, ulog_Event *ev) {

#if FEATURE_CUSTOM_PREFIX
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
#else   // FEATURE_CUSTOM_PREFIX
    char buf[65];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", ev->time)] = '\0';
#endif  // FEATURE_CUSTOM_PREFIX

    print(tgt, "%s", buf);
}
#endif  // FEATURE_EXTRA_OUTPUTS

#endif  // FEATURE_TIME

/* ============================================================================
   Feature: Custom Prefix
============================================================================ */
#if FEATURE_CUSTOM_PREFIX

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    ulog.update_prefix_function = function;
}

static void print_prefix(log_target *tgt, ulog_Event *ev) {
    if (ulog.update_prefix_function) {
        ulog.update_prefix_function(ev, ulog.custom_prefix,
                                    CFG_CUSTOM_PREFIX_SIZE);
        print(tgt, "%s", ulog.custom_prefix);
    }
}

#endif  // FEATURE_CUSTOM_PREFIX

/* ============================================================================
   Feature: Extra Outputs
============================================================================ */
#if FEATURE_EXTRA_OUTPUTS

/// @brief Callback for file
/// @param ev - Event
/// @param arg - File pointer
static void callback_file(ulog_Event *ev, void *arg) {
    log_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    write_formatted_message(&tgt, ev, ULOG_TIME_FULL, ULOG_COLOR_OFF,
                            ULOG_NEW_LINE_ON);
}

/// @brief Adds a callback
/// @param function - Callback function
/// @param arg - Optional argument that will be added to the event to be
///              processed by the callback
/// @param level - Debug level
int ulog_add_callback(ulog_LogFn function, void *arg, int level) {
    for (int i = 0; i < CFG_EXTRA_OUTPUTS; i++) {
        if (!ulog.callbacks[i].function) {
            ulog.callbacks[i] = (Callback){function, arg, level};
            return 0;
        }
    }
    return -1;
}

/// @brief Add file callback
int ulog_add_fp(FILE *fp, int level) {
    return ulog_add_callback(callback_file, fp, level);
}

/// @brief Processes the extra callbacks
/// @param ev - Event
static void log_to_extra_outputs(ulog_Event *ev) {
    // Processing the message for callbacks
    for (int i = 0; i < CFG_EXTRA_OUTPUTS && ulog.callbacks[i].function; i++) {
        process_callback(ev, &ulog.callbacks[i]);
    }
}

#endif  // FEATURE_EXTRA_OUTPUTS

/* ============================================================================
   Feature: Log Topics
============================================================================ */
// All topic-specific function definitions and static variables have been moved to ulog_topics.c
// Declarations are in ulog_topics.h, included via ulog.h if FEATURE_TOPICS is enabled.
// Calls to topic functions (ulog_get_topic_id, is_topic_enabled, _ulog_get_topic_level, print_topic)
// in ulog_log() and write_formatted_message() will now use the declarations from ulog_topics.h.

/* ============================================================================
   Core Functionality: Thread Safety
============================================================================ */

/// @brief Locks if the function provided
static void lock(void) {
    if (ulog.lock_function) {
        ulog.lock_function(true, ulog.lock_arg);
    }
}

/// @brief Unlocks if the function provided
static void unlock(void) {
    if (ulog.lock_function) {
        ulog.lock_function(false, ulog.lock_arg);
    }
}

/// @brief  Sets the lock function and user data
void ulog_set_lock(ulog_LockFn function, void *lock_arg) {
    ulog.lock_function = function;
    ulog.lock_arg      = lock_arg;
}

/* ============================================================================
   Core Functionality
============================================================================ */

/// @brief Level strings

// clang-format off
static const char *level_strings[] = {
#if FEATURE_EMOJI_LEVELS
    "⚪", "🔵", "🟢", "🟡", "🔴", "💥"
#elif FEATURE_SHORT_LEVELS
    "T", "D", "I", "W", "E", "F"
#else
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
#endif
};
// clang-format on

/// @brief Writes the formatted message
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
static void write_formatted_message(log_target *tgt, ulog_Event *ev,
                                    bool full_time, bool color, bool new_line) {

#if FEATURE_COLOR
    if (color) {
        print_color_start(tgt, ev);
    }
#else
    (void)color;
#endif  // FEATURE_COLOR

#if FEATURE_TIME
    if (full_time) {
#if FEATURE_EXTRA_OUTPUTS
        print_time_full(tgt, ev);
#endif
    } else {
        print_time_sec(tgt, ev);
    }
#else
    (void)full_time;
#endif  // FEATURE_TIME

#if FEATURE_CUSTOM_PREFIX
    print_prefix(tgt, ev);
#endif

#if FEATURE_TOPICS
    print_topic(tgt, ev);
#endif

    print_level(tgt, ev);

    print_message(tgt, ev);

#if FEATURE_COLOR
    if (color) {
        print_color_end(tgt, ev);
    }
#endif

    if (new_line) {
        print(tgt, "\n");
    }
}

/// @brief Callback for stdout
/// @param ev
static void callback_stdout(ulog_Event *ev, void *arg) {
    log_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    write_formatted_message(&tgt, ev, ULOG_TIME_SHORT, ULOG_COLOR_ON,
                            ULOG_NEW_LINE_ON);
}

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    if (!out || out_size == 0) {
        return -1;
    }
    log_target tgt = {.type = T_BUFFER, .dsc.buffer = {out, 0, out_size}};
    write_formatted_message(&tgt, ev, ULOG_TIME_SHORT, ULOG_COLOR_OFF,
                            ULOG_NEW_LINE_OFF);
    return 0;
}

/// @brief Processes the stdout callback
/// @param ev - Event
static void log_to_stdout(ulog_Event *ev) {
    if (!ulog.quiet) {
        // Initializing the stdout callback if not set
        if (!ulog.callback_stdout.function) {
            ulog.callback_stdout = (Callback){callback_stdout,
                                              stdout,  // pass stream
                                              LOG_TRACE};
        }
        process_callback(ev, &ulog.callback_stdout);
    }
}

/// @brief Logs the message
void ulog_log(int level, const char *file, int line, const char *topic,
              const char *message, ...) {

    if (level < ulog.level) {
        return;
    }
#if !FEATURE_TOPICS
    (void)topic;
#else
    int topic_id = TOPIC_NOT_FOUND; // TOPIC_NOT_FOUND is -1 as per ulog_topics.h
    if (topic != NULL) {
        // Working with topics
        topic_id = ulog_get_topic_id(topic); // Now calls function from ulog_topics.c

        if (topic_id == TOPIC_NOT_FOUND) {
            // If topic is not found (and not auto-created by ulog_get_topic_id),
            // then we don't log this message.
            // This applies to both static and dynamic allocation if auto-creation is off or fails.
            return;
        }

        // If the topic is disabled or set to a lower logging level, do not log
        // These functions are now declared in ulog_topics.h and defined in ulog_topics.c
        if (!is_topic_enabled(topic_id) ||
            (level < _ulog_get_topic_level(topic_id))) {
            return;
        }
    }
#endif

    ulog_Event ev = {
        .message = message,
        .file    = file,
        .line    = line,
        .level   = level,
#if FEATURE_TOPICS
        .topic = topic_id,
#endif
    };

    va_start(ev.message_format_args, message);

    lock();

    log_to_stdout(&ev);

#if FEATURE_EXTRA_OUTPUTS
    log_to_extra_outputs(&ev);
#endif

    unlock();

    va_end(ev.message_format_args);
}

static void print_level(log_target *tgt, ulog_Event *ev) {
    print(tgt, "%-1s ", level_strings[ev->level]);
}

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void print_message(log_target *tgt, ulog_Event *ev) {

#if FEATURE_FILE_STRING
    print(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
#endif

    if (ev->message) {
        vprint(tgt, ev->message, ev->message_format_args);  // message
    } else {
        print(tgt, "NULL");  // message
    }
}

/// @brief Processes the callback with the event
/// @param ev - Event
/// @param cb - Callback
static void process_callback(ulog_Event *ev, Callback *cb) {
    if (ev->level >= cb->level) {

#if FEATURE_TIME
        if (!ev->time) {
            time_t t = time(NULL);
            ev->time = localtime(&t);
        }
#endif  // FEATURE_TIME

        // Create event copy to avoid va_list issues
        ulog_Event ev_copy = { 0 };
        memcpy(&ev_copy, ev, sizeof(ulog_Event));
        
        // Initialize the va_list for the copied event
        // Note: We use a copy of the va_list to avoid issues with passing it
        // directly as on some platforms using the same va_list multiple times
        // can lead to undefined behavior.
        va_copy(ev_copy.message_format_args, ev->message_format_args);
        cb->function(&ev_copy, cb->arg);
        va_end(ev_copy.message_format_args);

    }
}

//==================================================================
// Core Functionality: Logger configuration
//==================================================================

/// @brief Returns the string representation of the level
const char *ulog_get_level_string(int level) {
    return level_strings[level];
}

/// @brief Sets the debug level
void ulog_set_level(int level) {
    ulog.level = level;
}

/// @brief Sets the quiet mode
void ulog_set_quiet(bool enable) {
    ulog.quiet = enable;
}
