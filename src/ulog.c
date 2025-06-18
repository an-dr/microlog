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
   Core Feature: Printing (Depends on: - )
============================================================================ */

//  Private
// ================

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

static void _vprint(log_target *tgt, const char *format, va_list args) {
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

static void _print(log_target *tgt, const char *format, ...) {
    va_list args;
    va_start(args, format);
    _vprint(tgt, format, args);
    va_end(args);
}

/* ============================================================================
   Prototypes
============================================================================ */

static void _print_formatted_message(log_target *tgt, ulog_Event *ev,
                                     bool full_time, bool color, bool new_line);

/* ============================================================================
   Feature: Color (Depends on: Printing)
============================================================================ */
#if FEATURE_COLOR

//  Private
// ================
typedef struct {
    bool enabled;
} feature_color_t;

static feature_color_t feature_color = {
    .enabled = true,
};

static const char *level_colors[] = {
    "\x1b[37m",  // TRACE : White #000
    "\x1b[36m",  // DEBUG : Cyan #0ff
    "\x1b[32m",  // INFO : Green #0f0
    "\x1b[33m",  // WARN : Yellow #ff0
    "\x1b[31m",  // ERROR : Red #f00
    "\x1b[35m"   // FATAL : Magenta #f0f
};
#define COLOR_TERMINATOR "\x1b[0m"

static void _print_color_start(log_target *tgt, ulog_Event *ev) {
    if (feature_color.enabled) {
        _print(tgt, "%s", level_colors[ev->level]);  // color start
    }
}

static void _print_color_end(log_target *tgt) {
    if (feature_color.enabled) {
        _print(tgt, "%s", COLOR_TERMINATOR);  // color end
    }
}

// Public
// ================

void ulog_enable_color(bool enable) {
    feature_color.enabled = enable;
}

// Disabled Private
// ================
#else  // FEATURE_COLOR
#define _print_color_start(tgt, ev) (void)(tgt), (void)(ev)
#define _print_color_end(tgt) (void)(tgt)
#endif  // FEATURE_COLOR

/* ============================================================================
   Feature: Time (Depends on: Printing)
============================================================================ */
#if FEATURE_TIME

// Private
// ================
typedef struct {
    bool enabled;
} feature_time_t;
static feature_time_t feature_time = {
    .enabled = true,
};

static void _print_time_sec(log_target *tgt, ulog_Event *ev) {
    if (feature_time.enabled == false) {
        return;  // Time feature is disabled
    }

#if FEATURE_CUSTOM_PREFIX
    char buf[9];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
#else
    char buf[10];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S ", ev->time)] = '\0';
#endif
    _print(tgt, "%s", buf);
}

static void _print_time_full(log_target *tgt, ulog_Event *ev) {

#if FEATURE_CUSTOM_PREFIX
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
#else
    char buf[65];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", ev->time)] = '\0';
#endif
    _print(tgt, "%s", buf);
}

// Public
// ================

void ulog_enable_time(bool enable) {
    feature_time.enabled = enable;
}

// Disabled Private
// ================
#else  // FEATURE_TIME
#define _print_time_sec(tgt, ev) (void)(tgt), (void)(ev)
#define _print_time_full(tgt, ev) (void)(tgt), (void)(ev)
#endif  // FEATURE_TIME

/* ============================================================================
   Feature: Custom Prefix (Depends on: Printing)
============================================================================ */
#if FEATURE_CUSTOM_PREFIX

// Private
// ================
typedef struct {
    bool enabled;
    ulog_PrefixFn function;
    char custom_prefix[CFG_CUSTOM_PREFIX_SIZE];
} feature_custom_prefix_t;

static feature_custom_prefix_t feature_custom_prefix = {
    .enabled       = true,
    .function      = NULL,
    .custom_prefix = {0},
};

static void _print_prefix(log_target *tgt, ulog_Event *ev) {
    if (feature_custom_prefix.function && feature_custom_prefix.enabled) {
        feature_custom_prefix.function(ev, feature_custom_prefix.custom_prefix,
                                       CFG_CUSTOM_PREFIX_SIZE);
        _print(tgt, "%s", feature_custom_prefix.custom_prefix);
    }
}

// Public
// ================

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    feature_custom_prefix.function = function;
}

void ulog_enable_prefix(bool enable) {
    feature_custom_prefix.enabled = enable;
}

// Disabled Private
// ================
#else  // FEATURE_CUSTOM_PREFIX
#define _print_prefix(tgt, ev) (void)(tgt), (void)(ev)
#endif  // FEATURE_CUSTOM_PREFIX

/* ============================================================================
   Core Feature: Levels (Depends on: Printing)
============================================================================ */

// Private
// ================
#ifndef ULOG_DEFAULT_LOG_LEVEL
#define ULOG_DEFAULT_LOG_LEVEL LOG_TRACE
#endif

typedef struct {
    int level;
    bool short_levels;
} feature_log_level_t;

static feature_log_level_t feature_log_level = {
    .level        = ULOG_DEFAULT_LOG_LEVEL,
    .short_levels = false,
};

// clang-format off
#define ULOG_LEVELS_LONG  0
#define ULOG_LEVELS_SHORT 1
#define ULOG_LEVELS_NUM   6

/// @brief Level strings
static const char *level_strings[][ULOG_LEVELS_NUM] = {
     {"TRACE",  "DEBUG",    "INFO",     "WARN",     "ERROR",    "FATAL"}
#if FEATURE_EMOJI_LEVELS
    ,{"⚪",     "🔵",       "🟢",       "🟡",       "🔴",       "💥"}
#endif
#if !FEATURE_EMOJI_LEVELS && FEATURE_SHORT_LEVELS
    ,{"T",      "D",        "I",        "W",        "E",        "F"}
#endif
};
// clang-format on

static void _print_level(log_target *tgt, ulog_Event *ev) {
#if FEATURE_SHORT_LEVELS || FEATURE_EMOJI_LEVELS
    if (feature_log_level.short_levels) {
        _print(tgt, "%-1s ", level_strings[ULOG_LEVELS_SHORT][ev->level]);
    } else {
        _print(tgt, "%-5s ", level_strings[ULOG_LEVELS_LONG][ev->level]);
    }
#else
    _print(tgt, "%-1s ", level_strings[ULOG_LEVELS_LONG][ev->level]);
#endif  // FEATURE_SHORT_LEVELS || FEATURE_EMOJI_LEVELS
}

// Public
// ================

/// @brief Returns the string representation of the level
const char *ulog_get_level_string(int level) {
    if (level < 0 || level >= ULOG_LEVELS_NUM) {
        return "?";  // Return a default string for invalid levels
    }
    return level_strings[ULOG_LEVELS_LONG][level];
}

/// @brief Sets the debug level
void ulog_set_level(int level) {
    if (level < LOG_TRACE || level > LOG_FATAL) {
        return;  // Invalid level, do nothing
    }
    feature_log_level.level = level;
}

/* ============================================================================
   Core Feature: Callback (Depends on: Logging)
============================================================================ */

//  Private
// ================

static void __callback_stdout(ulog_Event *ev, void *arg);

typedef struct {
    ulog_LogFn function;
    void *arg;
    int level;
} callback_t;

typedef struct {
    bool quiet_mode;
    callback_t callback;
} feature_callback_t;

static feature_callback_t feature_callback = {
    .quiet_mode = false,
    .callback   = {0},
};

static void __callback_stdout(ulog_Event *ev, void *arg) {
    log_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    _print_formatted_message(&tgt, ev, false, true, true);
}

static void _process_callback(ulog_Event *ev, callback_t *cb) {
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

static void _log_to_stdout(ulog_Event *ev) {
    if (!feature_callback.quiet_mode) {
        // Initializing the stdout callback if not set
        if (!feature_callback.callback.function) {
            feature_callback.callback = (callback_t){__callback_stdout,
                                                     stdout,  // pass stream
                                                     LOG_TRACE};
        }
        _process_callback(ev, &feature_callback.callback);
    }
}

// Public
// ================

/// @brief Sets the quiet mode
void ulog_set_quiet(bool enable) {
    feature_callback.quiet_mode = enable;
}

/* ============================================================================
   Feature: Extra Outputs (Depends on: Callback)
============================================================================ */
#if FEATURE_EXTRA_OUTPUTS
// Private
// ================
typedef struct {
    bool enabled;
    callback_t callbacks[CFG_EXTRA_OUTPUTS];
} feature_extra_outputs_t;

static feature_extra_outputs_t feature_extra_outputs = {.enabled   = true,
                                                        .callbacks = {{0}}};

static void __callback_file(ulog_Event *ev, void *arg) {
    log_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    _print_formatted_message(&tgt, ev, true, false, true);
}

static void _log_to_extra_outputs(ulog_Event *ev) {
    if (!feature_extra_outputs.enabled) {
        return;  // Feature is disabled
    }
    // Processing the message for callbacks
    for (int i = 0;
         i < CFG_EXTRA_OUTPUTS && feature_extra_outputs.callbacks[i].function;
         i++) {
        _process_callback(ev, &feature_extra_outputs.callbacks[i]);
    }
}

// Public
// ================

/// @brief Adds a callback
/// @param function - Callback function
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
   Feature: Topics (Depends on: Printing)
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
static int _enable_topic(int topic);
static int _disable_topic(int topic);
static topic_t *_get_topic_begin(void);
static topic_t *_get_topic_next(topic_t *t);
static topic_t *_get_topic_ptr(int topic);

static bool new_topic_enabled = false;

static void _print_topic(log_target *tgt, ulog_Event *ev) {
    topic_t *t = _get_topic_ptr(ev->topic);
    if (t && t->name) {
        _print(tgt, "[%s] ", t->name);
    }
}

static int __set_topic_level(int topic, int level) {
    topic_t *t = _get_topic_ptr(topic);
    if (t) {
        t->level = level;
        return 0;
    }
    return -1;
}

static int _get_topic_level(int topic) {
    topic_t *t = _get_topic_ptr(topic);
    if (t) {
        return t->level;
    }
    return LOG_TRACE;
}

static int __enable_topic(int topic) {
    topic_t *t = _get_topic_ptr(topic);
    if (t) {
        t->enabled = true;
        return 0;
    }
    return -1;
}

static int __disable_topic(int topic) {
    topic_t *t = _get_topic_ptr(topic);
    if (t) {
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
    if (topic_name == NULL) {
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
        else if(strcmp(topics[i].name, topic_name) == 0) {
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
    if (t) {
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
    if (topic_name == NULL) {
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
   Core Functionality: Thread Safety (Depends on: - )
============================================================================ */

// Private
// ================
typedef struct {
    ulog_LockFn lock_function;  // Lock function
    void *lock_arg;             // Argument for the lock function
} feature_lock_t;

static feature_lock_t feature_lock = {
    .lock_function = NULL,  // No lock function by default
    .lock_arg      = NULL,  // No lock argument by default
};

static void _lock(void) {
    if (feature_lock.lock_function) {
        feature_lock.lock_function(true, feature_lock.lock_arg);
    }
}

static void _unlock(void) {
    if (feature_lock.lock_function) {
        feature_lock.lock_function(false, feature_lock.lock_arg);
    }
}

// Public
// ================

/// @brief  Sets the lock function and user data
void ulog_set_lock(ulog_LockFn function, void *lock_arg) {
    feature_lock.lock_function = function;
    feature_lock.lock_arg      = lock_arg;
}

/* ============================================================================
   Core Feature: Logging (Depends on: Printing, Levels, Callback, Extra Outputs,
                          Custom Prefix, Topics, Time, Color, Locking)
============================================================================ */

bool show_file_string = false;  // Show file and line in the log message

// Private
// ================

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void __print_message(log_target *tgt, ulog_Event *ev) {

#if FEATURE_FILE_STRING
    if (show_file_string) {
        _print(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
    }
#endif

    if (ev->message) {
        _vprint(tgt, ev->message, ev->message_format_args);  // message
    } else {
        _print(tgt, "NULL");  // message
    }
}

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
static void _print_formatted_message(log_target *tgt, ulog_Event *ev,
                                     bool full_time, bool color,
                                     bool new_line) {

    color ? _print_color_start(tgt, ev) : (void)0;

    full_time ? _print_time_full(tgt, ev) : _print_time_sec(tgt, ev);

    _print_prefix(tgt, ev);
    _print_topic(tgt, ev);
    _print_level(tgt, ev);
    __print_message(tgt, ev);

    color ? _print_color_end(tgt) : (void)0;
    new_line ? _print(tgt, "\n") : (void)0;
}

// Public
// ================

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    if (!out || out_size == 0) {
        return -1;
    }
    log_target tgt = {.type = T_BUFFER, .dsc.buffer = {out, 0, out_size}};
    _print_formatted_message(&tgt, ev, false, false, false);
    return 0;
}

void ulog_log(int level, const char *file, int line, const char *topic,
              const char *message, ...) {

    if (level < feature_log_level.level) {
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

        // If the topic is disabled or set to a lower logging level, do not log
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

    _lock();

    _log_to_stdout(&ev);
    _log_to_extra_outputs(&ev);

    _unlock();

    va_end(ev.message_format_args);
}
