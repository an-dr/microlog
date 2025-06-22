// *************************************************************************
//
// ulog v@{ULOG_VERSION}@ - A simple customizable logging library.
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

#include "ulog.h"
#include <stdlib.h>
#include <string.h>

/* ============================================================================
   Testing
============================================================================ */

// If testing is enabled, we define NOT_SO_STATIC as empty to allow
// the functions to be visible outside of this file for testing purposes.
#ifdef ULOG_TESTING
#define NOT_VERY_STATIC
#else
#define NOT_VERY_STATIC static
#endif

/* ============================================================================
   Core Feature: Print (Depends on: - )
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

typedef enum { T_BUFFER, T_STREAM } log_target_type;

typedef struct {
    log_target_type type;
    print_target_descriptor dsc;
} print_target;

static void print_args(print_target *tgt, const char *format, va_list args) {
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

static void print(print_target *tgt, const char *format, ...) {
    va_list args;
    va_start(args, format);
    print_args(tgt, format, args);
    va_end(args);
}

/* ============================================================================
   Prototypes
============================================================================ */

static void log(print_target *tgt, ulog_Event *ev, bool full_time, bool color,
                bool new_line);

/* ============================================================================
   Feature: Color (Depends on: Print)
============================================================================ */
#if FEATURE_COLOR

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

static void color_print_start(print_target *tgt, ulog_Event *ev) {
    print(tgt, "%s", color_levels[ev->level]);  // color start
}

static void color_print_end(print_target *tgt) {
    print(tgt, "%s", COLOR_TERMINATOR);  // color end
}

// Disabled Private
// ================
#else  // FEATURE_COLOR
#define color_print_start(tgt, ev) (void)(tgt), (void)(ev)
#define color_print_end(tgt) (void)(tgt)
#endif  // FEATURE_COLOR

/* ============================================================================
   Feature: Time (Depends on: Print)
============================================================================ */
#if FEATURE_TIME

#define TIME_SHORT_BUF_SIZE 10  // HH:MM:SS(8) + 1 space + null terminator
#define TIME_FULL_BUF_SIZE 21   // YYYY-MM-DD HH:MM:SS(19) + 1 space + null

typedef struct {
    char short_buf[TIME_SHORT_BUF_SIZE];

#if FEATURE_EXTRA_OUTPUTS  // Used for file for example
    char full_buf[TIME_FULL_BUF_SIZE];
#endif

} time_data_t;
static time_data_t time_data_buf = {0};

// Private
// ================
static void time_print_short(print_target *tgt, ulog_Event *ev,
                             bool append_space) {
    char *buf          = time_data_buf.short_buf;
    const char *format = append_space ? "%H:%M:%S " : "%H:%M:%S";

    size_t len = strftime(buf, TIME_SHORT_BUF_SIZE, format, ev->time);
    buf[len]   = '\0';  // Ensure null termination
    print(tgt, "%s", buf);
}

#if FEATURE_EXTRA_OUTPUTS
static void time_print_full(print_target *tgt, ulog_Event *ev,
                            bool append_space) {
    char *buf = time_data_buf.full_buf;
    const char *format =
        append_space ? "%Y-%m-%d %H:%M:%S " : "%Y-%m-%d %H:%M:%S";

    size_t len = strftime(buf, TIME_FULL_BUF_SIZE, format, ev->time);
    buf[len]   = '\0';  // Ensure null termination
    print(tgt, "%s", buf);
}
#endif  // FEATURE_EXTRA_OUTPUTS

// Disabled Private
// ================
#else  // FEATURE_TIME
#define time_print_short(tgt, ev, append_space)                                \
    (void)(tgt), (void)(ev), (void)(append_space)
#define time_print_full(tgt, ev, append_space)                                 \
    (void)(tgt), (void)(ev), (void)(append_space)
#endif  // FEATURE_TIME

/* ============================================================================
   Feature: Prefix (Depends on: Print)
============================================================================ */
#if FEATURE_CUSTOM_PREFIX

// Private
// ================
typedef struct {
    ulog_PrefixFn function;
    char prefix[CFG_CUSTOM_PREFIX_SIZE];
} prefix_data_t;

static prefix_data_t prefix_data = {
    .function = NULL,
    .prefix   = {0},
};

static void prefix_print(print_target *tgt, ulog_Event *ev) {
    if (prefix_data.function) {
        prefix_data.function(ev, prefix_data.prefix, CFG_CUSTOM_PREFIX_SIZE);
        print(tgt, "%s", prefix_data.prefix);
    }
}

// Public
// ================

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    prefix_data.function = function;
}

// Disabled Private
// ================
#else  // FEATURE_CUSTOM_PREFIX
#define prefix_print(tgt, ev) (void)(tgt), (void)(ev)
#endif  // FEATURE_CUSTOM_PREFIX

/* ============================================================================
   Core Feature: Levels (Depends on: Print)
============================================================================ */

// Private
// ================
#ifndef ULOG_DEFAULT_LOG_LEVEL
#define ULOG_DEFAULT_LOG_LEVEL LOG_TRACE
#endif

typedef struct {
    int level;
    bool short_levels;
} levels_data_t;

static levels_data_t levels_data = {
    .level        = ULOG_DEFAULT_LOG_LEVEL,
    .short_levels = FEATURE_SHORT_LEVELS || FEATURE_EMOJI_LEVELS,
};

// clang-format off
#define LEVELS_USE_LONG     0
#define LEVELS_USE_SHORT    1
#define LEVELS_TOTAL        6

/// @brief Level strings
static const char *levels_strings[][LEVELS_TOTAL] = {
     {"TRACE",  "DEBUG",    "INFO",     "WARN",     "ERROR",    "FATAL"}
#if FEATURE_EMOJI_LEVELS
    ,{"⚪",     "🔵",       "🟢",       "🟡",       "🔴",       "💥"}
#endif
#if !FEATURE_EMOJI_LEVELS && FEATURE_SHORT_LEVELS
    ,{"T",      "D",        "I",        "W",        "E",        "F"}
#endif
};
// clang-format on

static void levels_print(print_target *tgt, ulog_Event *ev) {
#if FEATURE_SHORT_LEVELS || FEATURE_EMOJI_LEVELS
    if (levels_data.short_levels) {
        print(tgt, "%-1s ", levels_strings[LEVELS_USE_SHORT][ev->level]);
    } else {
        print(tgt, "%-5s ", levels_strings[LEVELS_USE_LONG][ev->level]);
    }
#else
    print(tgt, "%-1s ", levels_strings[LEVELS_USE_LONG][ev->level]);
#endif  // FEATURE_SHORT_LEVELS || FEATURE_EMOJI_LEVELS
}

// Public
// ================

/// @brief Returns the string representation of the level
const char *ulog_get_level_string(int level) {
    if (level < 0 || level >= LEVELS_TOTAL) {
        return "?";  // Return a default string for invalid levels
    }
    return levels_strings[LEVELS_USE_LONG][level];
}

/// @brief Sets the debug level
void ulog_set_level(int level) {
    if (level < LOG_TRACE || level > LOG_FATAL) {
        return;  // Invalid level, do nothing
    }
    levels_data.level = level;
}

/* ============================================================================
   Core Feature: Callbacks (Depends on: Logging)
============================================================================ */

//  Private
// ================

typedef struct {
    ulog_LogFn function;
    void *arg;
    int level;
} callback_t;

typedef struct {
    bool quiet_mode;
    callback_t callback;
} callbacks_data_t;

static callbacks_data_t callbacks_data = {
    .quiet_mode      = false,
    .stdout_callback = {0},
};

static void callbacks_to_stdout(ulog_Event *ev, void *arg) {
    print_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    log(&tgt, ev, false, true, true);
}

static void callbacks_execute(ulog_Event *ev, callback_t *cb) {
    if (ev->level >= cb->level) {
#if FEATURE_TIME
        if (!ev->time) {
            time_t t = time(NULL);
            ev->time = localtime(&t);
        }
#endif  // FEATURE_TIME

        // Create event copy to avoid va_list issues
        ulog_Event ev_copy = {0};
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

static void callbacks_send_to_stdout(ulog_Event *ev) {
    if (!callbacks_data.quiet_mode) {
        // Initializing the stdout callback if not set
        if (!callbacks_data.stdout_callback.function) {
            callbacks_data.stdout_callback =
                (callback_t){callbacks_to_stdout,
                             stdout,  // pass stream
                             LOG_TRACE};
        }
        callbacks_execute(ev, &callbacks_data.stdout_callback);
    }
}

// Public
// ================

/// @brief Sets the quiet mode
void ulog_set_quiet(bool enable) {
    callbacks_data.quiet_mode = enable;
}

/* ============================================================================
   Feature: Callbacks Extra (Depends on: Callbacks)
============================================================================ */
#if FEATURE_EXTRA_OUTPUTS
// Private
// ================
typedef struct {
    callback_t callbacks[CFG_EXTRA_OUTPUTS];
} feature_extra_outputs_t;

static feature_extra_outputs_t feature_extra_outputs = {.callbacks = {{0}}};

static void __callback_file(ulog_Event *ev, void *arg) {
    print_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    log(&tgt, ev, true, false, true);
}

static void _log_to_extra_outputs(ulog_Event *ev) {
    // Processing the message for callbacks
    for (int i = 0;
         i < CFG_EXTRA_OUTPUTS && feature_extra_outputs.callbacks[i].function;
         i++) {
        callbacks_execute(ev, &feature_extra_outputs.callbacks[i]);
    }
}

// Public
// ================

/// @brief Adds a callback
/// @param function - Callbacks function
/// @param arg - Optional argument that will be added to the event to be
///              processed by the callback
/// @param level - Debug level
int ulog_add_callback(ulog_LogFn function, void *arg, int level) {
    for (int i = 0; i < CFG_EXTRA_OUTPUTS; i++) {
        if (!feature_extra_outputs.callbacks[i].function) {
            feature_extra_outputs.callbacks[i] =
                (callback_t){function, arg, level};
            return 0;
        }
    }
    return -1;
}

/// @brief Add file callback
int ulog_add_fp(FILE *fp, int level) {
    return ulog_add_callback(__callback_file, fp, level);
}

// Disabled Private
// ================
#else  // FEATURE_EXTRA_OUTPUTS
#define _log_to_extra_outputs(ev) (void)(ev)
#endif  // FEATURE_EXTRA_OUTPUTS

/* ============================================================================
   Feature: Topics (Depends on: Print)
============================================================================ */
#if FEATURE_TOPICS
// Private
// ================
typedef struct {
    int id;
    const char *name;
    bool enabled;
    int level;

#if CFG_TOPICS_DINAMIC_ALLOC == true
    void *next;  // Pointer to the next topic pointer (Topic **)
#endif

} topic_t;

static bool _is_topic_enabled(int topic);
static int __set_topic_level(int topic, int level);
static int __enable_topic(int topic);
static int __disable_topic(int topic);
static topic_t *_get_topic_begin(void);
static topic_t *_get_topic_next(topic_t *t);
static topic_t *_get_topic_ptr(int topic);

static bool new_topic_enabled = false;

static void _print_topic(print_target *tgt, ulog_Event *ev) {
    topic_t *t = _get_topic_ptr(ev->topic);
    if (t != NULL && t->name) {
        print(tgt, "[%s] ", t->name);
    }
}

static int __set_topic_level(int topic, int level) {
    topic_t *t = _get_topic_ptr(topic);
    if (t != NULL) {
        t->level = level;
        return 0;
    }
    return -1;
}

static int _get_topic_level(int topic) {
    topic_t *t = _get_topic_ptr(topic);
    if (t != NULL) {
        return t->level;
    }
    return LOG_TRACE;
}

static int __enable_topic(int topic) {
    topic_t *t = _get_topic_ptr(topic);
    if (t != NULL) {
        t->enabled = true;
        return 0;
    }
    return -1;
}

static int __disable_topic(int topic) {
    topic_t *t = _get_topic_ptr(topic);
    if (t != NULL) {
        t->enabled = false;
        return 0;
    }
    return -1;
}

/// @brief Checks if the topic is enabled
/// @param topic - Topic ID or -1 if no topic
/// @return true if enabled or no topic, false otherwise
static bool _is_topic_enabled(int topic) {
    if (topic < 0) {  // no topic, always allowed
        return true;
    }

    topic_t *t = _get_topic_ptr(topic);
    if (t) {
        return t->enabled;
    }
    return false;
}

// Public
// ================

int ulog_set_topic_level(const char *topic_name, int level) {
    if (ulog_add_topic(topic_name, true) != -1) {
        return __set_topic_level(ulog_get_topic_id(topic_name), level);
    }
    return -1;
}

int ulog_enable_topic(const char *topic_name) {
    ulog_add_topic(topic_name, true);
    return __enable_topic(ulog_get_topic_id(topic_name));
}

int ulog_disable_topic(const char *topic_name) {
    ulog_add_topic(topic_name, false);
    return __disable_topic(ulog_get_topic_id(topic_name));
}

int ulog_enable_all_topics(void) {
    new_topic_enabled = true;
    for (topic_t *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        t->enabled = true;
    }
    return 0;
}

int ulog_disable_all_topics(void) {
    new_topic_enabled = false;
    for (topic_t *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        t->enabled = false;
    }
    return 0;
}

int ulog_get_topic_id(const char *topic_name) {
    for (topic_t *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        if (t->name && strcmp(t->name, topic_name) == 0) {
            return t->id;
        }
    }
    return TOPIC_NOT_FOUND;
}

#else

#define _print_topic(tgt, ev) (void)(tgt), (void)(ev)
#define _is_topic_enabled(topic) (void)(topic), true

#endif  // FEATURE_TOPICS

/* ============================================================================
   Feature: Topics - Static Allocation (Depends on: Topics)
============================================================================ */
#if FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == false
// Private
// ================
static topic_t topics[CFG_TOPICS_NUM] = {{0}};

static topic_t *_get_topic_begin(void) {
    return topics;
}

static topic_t *_get_topic_next(topic_t *t) {
    if ((t >= topics) && (t < topics + CFG_TOPICS_NUM - 1)) {
        return t + 1;
    }
    return NULL;
}

static topic_t *_get_topic_ptr(int topic) {
    if (topic < CFG_TOPICS_NUM) {
        return &topics[topic];
    }
    return NULL;
}

// Public
// ================

int ulog_add_topic(const char *topic_name, bool enable) {
    if (topic_name == NULL || strlen(topic_name) == 0) {
        return -1;
    }

    for (int i = 0; i < CFG_TOPICS_NUM; i++) {
        // If there is an empty slot
        if (!topics[i].name) {
            topics[i].id      = i;
            topics[i].name    = topic_name;
            topics[i].enabled = enable;
            topics[i].level   = ULOG_DEFAULT_LOG_LEVEL;
            return i;
        }
        // If the topic already exists
        else if (strcmp(topics[i].name, topic_name) == 0) {
            return i;
        }
    }
    return -1;
}
#endif  // FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == false

/* ============================================================================
   Feature: Topics - Dynamic Allocation (Depends on: Topics)
============================================================================ */

#if FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == true
// Private
// ================
static topic_t *topics = NULL;

static topic_t *_get_topic_begin(void) {
    return topics;
}

static topic_t *_get_topic_next(topic_t *t) {
    return t->next;
}

static topic_t *_get_topic_ptr(int topic) {
    for (topic_t *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        if (t->id == topic) {
            return t;
        }
    }
    return NULL;
}

static void *_create_topic(int id, const char *topic_name, bool enable) {
    topic_t *t = malloc(sizeof(topic_t));
    if (t != NULL) {
        t->id      = id;
        t->name    = topic_name;
        t->enabled = enable;
        t->next    = NULL;
    }
    return t;
}

// Public
// ================

int ulog_add_topic(const char *topic_name, bool enable) {
    if (topic_name == NULL || strlen(topic_name) == 0) {
        return -1;
    }

    // if exists
    for (topic_t *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        if (t->name && strcmp(t->name, topic_name) == 0) {
            return t->id;
        }
    }

    // If the beginning is empty
    if (!topics) {
        topics = (topic_t *)_create_topic(0, topic_name, enable);
        if (topics) {
            return 0;
        }
        return -1;
    }

    // If the beginning is not empty
    int current_id = 0;
    topic_t *t     = topics;
    while (t->next != NULL) {
        t = t->next;
        current_id++;
    }

    t->next = _create_topic(current_id + 1, topic_name, enable);
    if (t->next) {
        return current_id + 1;
    }
    return -1;
}

#endif  // FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == true

/* ============================================================================
   Core Functionality: Lock (Depends on: - )
============================================================================ */

// Private
// ================
typedef struct {
    ulog_LockFn function;  // Lock function
    void *args;            // Argument for the lock function
} lock_data_t;

static lock_data_t lock_data = {
    .function = NULL,  // No lock function by default
    .args     = NULL,  // No lock argument by default
};

static void lock_lock(void) {
    if (lock_data.function) {
        lock_data.function(true, lock_data.args);
    }
}

static void lock_unlock(void) {
    if (lock_data.function) {
        lock_data.function(false, lock_data.args);
    }
}

// Public
// ================

/// @brief  Sets the lock function and user data
void ulog_set_lock(ulog_LockFn function, void *lock_arg) {
    lock_data.function = function;
    lock_data.args     = lock_arg;
}

/* ============================================================================
   Core Feature: Log (Depends on: Print, Levels, Callbacks, Extra Outputs,
                      Custom Prefix, Topics, Time, Color, Locking)
============================================================================ */

bool show_file_string = false;  // Show file and line in the log message

// Private
// ================

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void log_print_message(print_target *tgt, ulog_Event *ev) {

#if FEATURE_FILE_STRING
    if (show_file_string) {
        print(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
    }
#endif

    if (ev->message) {
        print_args(tgt, ev->message, ev->message_format_args);  // message
    } else {
        print(tgt, "NULL");  // message
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
static void log(print_target *tgt, ulog_Event *ev, bool full_time, bool color,
                bool new_line) {

    color ? color_print_start(tgt, ev) : (void)0;

    bool append_space = true;
#if FEATURE_CUSTOM_PREFIX
    if (prefix_data.function) {
        append_space = false;  // Custom prefix does not need leading space
    }
#endif

    full_time ? time_print_full(tgt, ev, append_space)
              : time_print_short(tgt, ev, append_space);

    prefix_print(tgt, ev);
    _print_topic(tgt, ev);
    levels_print(tgt, ev);
    log_print_message(tgt, ev);

    color ? color_print_end(tgt) : (void)0;
    new_line ? print(tgt, "\n") : (void)0;
}

// Public
// ================

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    if (!out || out_size == 0) {
        return -1;
    }
    print_target tgt = {.type = T_BUFFER, .dsc.buffer = {out, 0, out_size}};
    log(&tgt, ev, false, false, false);
    return 0;
}

void ulog_log(int level, const char *file, int line, const char *topic,
              const char *message, ...) {

    if (level < levels_data.level) {
        return;
    }
#if !FEATURE_TOPICS
    (void)topic;
#else
    // TODO: Move topic handling to a separate function
    int topic_id = -1;
    if (topic != NULL) {
        // Working with topics

        topic_id = ulog_get_topic_id(topic);
        if (topic_id == TOPIC_NOT_FOUND) {
#if CFG_TOPICS_DINAMIC_ALLOC == false
            return;  // Topic not found
#else
            // If no topic add a disabled one, so we can enable it later
            topic_id = ulog_add_topic(topic, new_topic_enabled);
#endif
        }

        // If the topic is disabled or set to a lower logging level, do not
        // log
        if (!_is_topic_enabled(topic_id) ||
            (level < _get_topic_level(topic_id))) {
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

    lock_lock();

    callbacks_send_to_stdout(&ev);
    _log_to_extra_outputs(&ev);

    lock_unlock();

    va_end(ev.message_format_args);
}
