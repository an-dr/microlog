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
   Core Feature: Print (`print_*`, depends on: - )
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

typedef enum { LOG_TARGET_BUFFER, LOG_TARGET_STREAM } log_target_t;

typedef struct {
    log_target_t type;
    print_target_descriptor dsc;
} print_target;

static void print_to_target_valist(print_target *tgt, const char *format,
                                   va_list args) {
    if (tgt->type == LOG_TARGET_BUFFER) {
        char *buf   = tgt->dsc.buffer.data + tgt->dsc.buffer.curr_pos;
        size_t size = tgt->dsc.buffer.size - tgt->dsc.buffer.curr_pos;
        if (size > 0) {
            tgt->dsc.buffer.curr_pos += vsnprintf(buf, size, format, args);
        }
    } else if (tgt->type == LOG_TARGET_STREAM) {
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
   Prototypes
============================================================================ */

static void log_print_event(print_target *tgt, ulog_Event *ev, bool full_time,
                            bool color, bool new_line);

/* ============================================================================
   Feature: Color (`color_*`, depends on: Print)
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
    print_to_target(tgt, "%s", color_levels[ev->level]);  // color start
}

static void color_print_end(print_target *tgt) {
    print_to_target(tgt, "%s", COLOR_TERMINATOR);  // color end
}

// Disabled Private
// ================
#else  // FEATURE_COLOR
#define color_print_start(tgt, ev) (void)(tgt), (void)(ev)
#define color_print_end(tgt) (void)(tgt)
#endif  // FEATURE_COLOR

/* ============================================================================
   Feature: Prefix (`prefix_*`, depends on: Print)
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
    if (prefix_data.function != NULL) {
        prefix_data.function(ev, prefix_data.prefix, CFG_CUSTOM_PREFIX_SIZE);
        print_to_target(tgt, "%s", prefix_data.prefix);
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
   Feature: Time (`time_*`, depends on: Print)
============================================================================ */
#if FEATURE_TIME

#define TIME_SHORT_BUF_SIZE 10  // HH:MM:SS(8) + 1 space + null
#define TIME_FULL_BUF_SIZE 21   // YYYY-MM-DD HH:MM:SS(19) + 1 space + null

// Private
// ================
static bool time_print_if_invalid(print_target *tgt, ulog_Event *ev) {
    if (ev->time == NULL) {
        print_to_target(tgt, "INVALID_TIME");
        return true;  // Time is invalid, print error message
    }
    return false;  // Time is valid
}

/// @brief Fills the event time with the current local time
/// @param ev - Event to fill. Assumed not NULL
static void time_fill_current_time(ulog_Event *ev) {
    time_t current_time = time(NULL);     // Get current time
    ev->time = localtime(&current_time);  // Fill time with current value
}

static void time_print_short(print_target *tgt, ulog_Event *ev,
                             bool append_space) {
    if (time_print_if_invalid(tgt, ev)) {
        return;  // If time is not valid, stop printing
    }
    char buf[TIME_SHORT_BUF_SIZE] = {0};
    const char *format            = append_space ? "%H:%M:%S " : "%H:%M:%S";
    strftime(buf, TIME_SHORT_BUF_SIZE, format, ev->time);
    print_to_target(tgt, "%s", buf);
}

#if FEATURE_EXTRA_OUTPUTS
static void time_print_full(print_target *tgt, ulog_Event *ev,
                            bool append_space) {
    if (time_print_if_invalid(tgt, ev)) {
        return;  // If time is not valid, stop printing
    }
    char buf[TIME_FULL_BUF_SIZE] = {0};
    const char *format =
        append_space ? "%Y-%m-%d %H:%M:%S " : "%Y-%m-%d %H:%M:%S";
    strftime(buf, TIME_FULL_BUF_SIZE, format, ev->time);
    print_to_target(tgt, "%s", buf);
}
#else
#define time_print_full(tgt, ev, append_space) (void)(0)
#endif  // FEATURE_EXTRA_OUTPUTS

// Disabled Private
// ================
#else  // FEATURE_TIME
#define time_print_short(tgt, ev, append_space) (void)(0)
#define time_print_full(tgt, ev, append_space) (void)(0)
#define time_fill_current_time(ev) (void)(ev)
#endif  // FEATURE_TIME

/* ============================================================================
   Core Feature: Levels  (`levels_*`, depends on: Print)
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
        print_to_target(tgt, "%-1s ",
                        levels_strings[LEVELS_USE_SHORT][ev->level]);
    } else {
        print_to_target(tgt, "%-5s ",
                        levels_strings[LEVELS_USE_LONG][ev->level]);
    }
#else
    print_to_target(tgt, "%-1s ", levels_strings[LEVELS_USE_LONG][ev->level]);
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
   Core Feature: Callbacks (`cb_*`, depends on: Print, Log, Levels)
============================================================================ */

//  Private
// ================
#define CB_STDOUT_ID 0
#define CB_NUM_CALLBACKS (1 + CFG_EXTRA_OUTPUTS)  // 1 for stdout + extra

// Forward declarations for data initialization
static void cb_stdout(ulog_Event *ev, void *arg);

typedef struct {
    ulog_LogFn function;
    void *arg;
    int level;
    bool is_enabled;  // Whether the callback is enabled or not
} cb_t;

typedef struct {
    cb_t callbacks[CB_NUM_CALLBACKS];  // 0 is for stdout callback
} cb_data_t;

static cb_data_t cb_data = {.callbacks = {{cb_stdout, NULL, LOG_TRACE, true}}};

static void cb_handle_single(ulog_Event *ev, cb_t *cb) {
    if (cb->is_enabled && ev->level >= cb->level) {

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

static void cb_handle_all(ulog_Event *ev) {
    // Processing the message for callbacks
    for (int i = 0;
         (i < CB_NUM_CALLBACKS) && (cb_data.callbacks[i].function != NULL);
         i++) {
        cb_handle_single(ev, &cb_data.callbacks[i]);
    }
}

static void cb_stdout(ulog_Event *ev, void *arg) {
    print_target tgt = {.type = LOG_TARGET_STREAM, .dsc.stream = stdout};
    log_print_event(&tgt, ev, false, true, true);
}

// Public
// ================

void ulog_set_quiet(bool enable) {
    // Disable stdout callback if quiet mode is enabled
    cb_data.callbacks[CB_STDOUT_ID].is_enabled = !enable;
}

/* ============================================================================
   Feature: User Callbacks (`cb_user_*` depends on: Callbacks)
============================================================================ */
#if FEATURE_EXTRA_OUTPUTS

//  Private
// ================
static void cb_user_file(ulog_Event *ev, void *arg) {
    print_target tgt = {.type = LOG_TARGET_STREAM, .dsc.stream = (FILE *)arg};
    log_print_event(&tgt, ev, true, false, true);
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
        if (cb_data.callbacks[i].function == NULL) {
            cb_data.callbacks[i] = (cb_t){function, arg, level, true};
            return 0;
        }
    }
    return -1;
}

/// @brief Add file callback
int ulog_add_fp(FILE *fp, int level) {
    return ulog_add_callback(cb_user_file, fp, level);
}

#endif  // FEATURE_EXTRA_OUTPUTS

/* ============================================================================
   Feature: Topics (Depends on: Print)
============================================================================ */
#define TOPIC_ID_NO_TOPIC -1

#if FEATURE_TOPICS

// Private
// ================

typedef struct {
    int id;
    const char *name;
    bool enabled;
    int level;

#if CFG_TOPICS_DYNAMIC_ALLOC
    void *next;  // Pointer to the next topic pointer (Topic **)
#endif

} topic_t;

typedef struct {
    bool new_topic_enabled;  // Whether new topics are enabled by default

#if CFG_TOPICS_DYNAMIC_ALLOC
    topic_t *topics;
#else
    topic_t topics[CFG_TOPICS_NUM];
#endif

} topic_data_t;

static topic_data_t topic_data = {
    .new_topic_enabled = false,  // New topics are disabled by default

#if CFG_TOPICS_DYNAMIC_ALLOC
    .topics = NULL,  // No topics allocated by default
#else
    .topics = {{0}},  // Initialize static topics array to zero
#endif
};

// === Implementation specific functions for topics ===========================

/// @brief Converts a topic string to its ID
/// @param str - Topic string
/// @return Topic ID or -1 if not found
static int topic_str_to_id(const char *str);

/// @brief Gets the topic by ID
/// @param topic - Topic ID
/// @return Pointer to the topic if found, NULL otherwise
static topic_t *topic_get(int topic);

/// @brief Enable all topics
/// @return 0 on success, -1 if no topics
static int topic_enable_all();

/// @brief Disable all topics
/// @return 0 on success, -1 if no topics
static int topic_disable_all();

// === Common Topic Functions =================================================

static void topic_print(print_target *tgt, ulog_Event *ev) {
    topic_t *t = topic_get(ev->topic);
    if (t != NULL) {
        print_to_target(tgt, "[%s] ", t->name);
    }
}

/// @brief Sets the topic level
/// @param topic - Topic ID
/// @param level - Log level to set
/// @return 0 on success, -1 if topic not found
static int topic_set_level(int topic, int level) {
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        t->level = level;
        return 0;
    }
    return -1;
}

/// @brief Gets the topic level
/// @param topic - Topic ID
/// @return Log level of the topic, or LOG_TRACE if topic not found
static int topic_get_level(int topic) {
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        return t->level;
    }
    return LOG_TRACE;
}

/// @brief Enables the topic
/// @param topic - Topic ID
/// @return 0 on success, -1 if topic not found
static int topic_enable(int topic) {
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        t->enabled = true;
        return 0;
    }
    return -1;
}

/// @brief Disables the topic
/// @param topic - Topic ID
/// @return 0 on success, -1 if topic not found
static int topic_disable(int topic) {
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        t->enabled = false;
        return 0;
    }
    return -1;
}

/// @brief Checks if the topic is enabled
/// @param topic - Topic ID or -1 if no topic
/// @return true if enabled or no topic, false otherwise
static bool topic_is_enabled(int topic) {
    if (topic < 0) {  // no topic, always allowed
        return true;
    }

    topic_t *t = topic_get(topic);
    if (t != NULL) {
        return t->enabled;
    }
    return false;
}

/// @brief Processes the topic
/// @param topic - Topic name
/// @param level - Log level
/// @param is_log_allowed - Output log allowed
/// @param topic_id - Output topic ID
static void topic_process(const char *topic, int level, bool *is_log_allowed,
                          int *topic_id) {
    if (is_log_allowed == NULL || topic_id == NULL) {
        return;  // Invalid arguments, do nothing
    }

    // Fail by default
    *is_log_allowed = false;              // Default to not allowed
    *topic_id       = TOPIC_ID_NO_TOPIC;  // Default to no topic

    // No topic, always allowed
    if (topic == NULL || strlen(topic) == 0) {
        *topic_id       = TOPIC_ID_NO_TOPIC;
        *is_log_allowed = true;
        return;
    }

    // If topic is not found, fail for STATIC allocation
    // or add a new one for DYNAMIC allocation
    *topic_id = ulog_get_topic_id(topic);
    if (*topic_id == TOPIC_NOT_FOUND) {
#if CFG_TOPICS_DYNAMIC_ALLOC
        // If no topic add a disabled one, so we can enable it later
        *topic_id = ulog_add_topic(topic, topic_data.new_topic_enabled);
#else
        *is_log_allowed = false;  // Topic not found, processing failed
        return;                   // Processing failed, topic not found
#endif
    }

    // If topic is not enabled or level is lower than topic level, skip logging
    if (!topic_is_enabled(*topic_id) || (level < topic_get_level(*topic_id))) {
        *is_log_allowed = false;
        return;
    }
    *is_log_allowed = true;
}

// Public
// ================

int ulog_set_topic_level(const char *topic_name, int level) {
    if (ulog_add_topic(topic_name, true) != -1) {
        return topic_set_level(ulog_get_topic_id(topic_name), level);
    }
    return -1;
}

int ulog_enable_topic(const char *topic_name) {
    ulog_add_topic(topic_name, true);
    return topic_enable(ulog_get_topic_id(topic_name));
}

int ulog_disable_topic(const char *topic_name) {
    ulog_add_topic(topic_name, false);
    return topic_disable(ulog_get_topic_id(topic_name));
}

int ulog_enable_all_topics(void) {
    topic_data.new_topic_enabled = true;
    topic_enable_all();
    return 0;
}

int ulog_disable_all_topics(void) {
    topic_data.new_topic_enabled = false;
    topic_disable_all();
    return 0;
}

int ulog_get_topic_id(const char *topic_name) {
    return topic_str_to_id(topic_name);
}

#else

#define topic_print(tgt, ev) (void)(tgt), (void)(ev)
#define topic_process(topic, level, is_log_allowed, topic_id)                  \
    (void)(topic), (void)(level), (void)(is_log_allowed), (void)(topic_id)

#endif  // FEATURE_TOPICS

/* ============================================================================
   Feature: Topics - Static Allocation (`topic_*`, depends on: Topics)
============================================================================ */
#if FEATURE_TOPICS && CFG_TOPICS_DYNAMIC_ALLOC == false
// Private
// ================

int topic_enable_all(void) {
    for (int i = 0; i < CFG_TOPICS_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, no more to enable
        }
        topic_data.topics[i].enabled = true;
    }
    return 0;
}

int topic_disable_all(void) {
    for (int i = 0; i < CFG_TOPICS_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, no more to disable
        }
        topic_data.topics[i].enabled = false;
    }
    return 0;
}

int topic_str_to_id(const char *str) {
    for (int i = 0; i < CFG_TOPICS_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, not found
        }
        if (strcmp(topic_data.topics[i].name, str) == 0) {
            return topic_data.topics[i].id;
        }
    }
    return TOPIC_NOT_FOUND;  // Not found
}

static topic_t *topic_get(int topic) {
    if (topic < CFG_TOPICS_NUM && topic >= 0) {
        return &topic_data.topics[topic];
    }
    return NULL;
}

// Public
// ================

int ulog_add_topic(const char *topic_name, bool enable) {
    if (is_str_empty(topic_name)) {
        return -1;
    }

    for (int i = 0; i < CFG_TOPICS_NUM; i++) {
        // If there is an empty slot
        if (is_str_empty(topic_data.topics[i].name)) {
            topic_data.topics[i].id      = i;
            topic_data.topics[i].name    = topic_name;
            topic_data.topics[i].enabled = enable;
            topic_data.topics[i].level   = ULOG_DEFAULT_LOG_LEVEL;
            return i;
        }
        // If the topic already exists
        else if (strcmp(topic_data.topics[i].name, topic_name) == 0) {
            return i;
        }
    }
    return -1;
}
#endif  // FEATURE_TOPICS && CFG_TOPICS_DYNAMIC_ALLOC == false

/* ============================================================================
   Feature: Topics - Dynamic Allocation (`topic_*`, depends on: Topics)
============================================================================ */

#if FEATURE_TOPICS && CFG_TOPICS_DYNAMIC_ALLOC == true
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

static int topic_enable_all(void) {
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        t->enabled = true;
    }
    return 0;
}

static int topic_disable_all(void) {
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        t->enabled = false;
    }
    return 0;
}

int topic_str_to_id(const char *str) {
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        if (!is_str_empty(t->name) && strcmp(t->name, str) == 0) {
            return t->id;
        }
    }
    return TOPIC_NOT_FOUND;
}

static topic_t *topic_get(int topic) {
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        if (t->id == topic) {
            return t;
        }
    }
    return NULL;
}

static void *topic_create(int id, const char *topic_name, bool enable) {
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
    if (is_str_empty(topic_name)) {
        return -1;
    }

    // if exists
    for (topic_t *t = topic_get_first(); t != NULL; t = topic_get_next(t)) {
        if (!is_str_empty(t->name) && strcmp(t->name, topic_name) == 0) {
            return t->id;
        }
    }

    // If the beginning is empty
    if (topic_data.topics == NULL) {
        topic_data.topics = (topic_t *)topic_create(0, topic_name, enable);
        if (topic_data.topics != NULL) {
            return 0;
        }
        return -1;
    }

    // If the beginning is not empty
    int current_id = 0;
    topic_t *t     = topic_data.topics;
    while (t->next != NULL) {
        t = t->next;
        current_id++;
    }

    t->next = topic_create(current_id + 1, topic_name, enable);
    if (t->next) {
        return current_id + 1;
    }
    return -1;
}

#endif  // FEATURE_TOPICS && CFG_TOPICS_DYNAMIC_ALLOC == true

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
void ulog_set_lock(ulog_LockFn function, void *lock_arg) {
    lock_data.function = function;
    lock_data.args     = lock_arg;
}

/* ============================================================================
   Core Feature: Log (`log_*`, depends on: Print, Levels, Callbacks,
                      Extra Outputs, Custom Prefix, Topics, Time, Color,
                      Locking)
============================================================================ */

typedef struct {
    bool show_file_string;  // Show file and line in the log message
} log_data_t;

static log_data_t log_data = {
    .show_file_string = FEATURE_FILE_STRING,
};

// Private
// ================

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void log_print_message(print_target *tgt, ulog_Event *ev) {

    if (log_data.show_file_string) {
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
static void log_print_event(print_target *tgt, ulog_Event *ev, bool full_time,
                            bool color, bool new_line) {

    color ? color_print_start(tgt, ev) : (void)0;

    bool append_space = true;
    (void)append_space;  // May be unused if no prefix and time
#if FEATURE_CUSTOM_PREFIX
    if (prefix_data.function != NULL) {
        append_space = false;  // Custom prefix does not need leading space
    }
#endif

    full_time ? time_print_full(tgt, ev, append_space)
              : time_print_short(tgt, ev, append_space);

    prefix_print(tgt, ev);
    topic_print(tgt, ev);
    levels_print(tgt, ev);
    log_print_message(tgt, ev);

    color ? color_print_end(tgt) : (void)0;
    new_line ? print_to_target(tgt, "\n") : (void)0;
}

void log_fill_event(ulog_Event *ev, const char *message, int level,
                    const char *file, int line, int topic_id) {
    if (ev == NULL) {
        return;  // Invalid event, do nothing
    }

    ev->message = message;
    ev->file    = file;
    ev->line    = line;
    ev->level   = level;

#if FEATURE_TOPICS
    ev->topic = topic_id;
#endif

#if FEATURE_TIME
    ev->time = NULL;  // Time will be filled later
#endif

    time_fill_current_time(ev);  // Fill time with current value
}

// Public
// ================

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    if (out == NULL || out_size == 0) {
        return -1;
    }
    print_target tgt = {.type       = LOG_TARGET_BUFFER,
                        .dsc.buffer = {out, 0, out_size}};
    log_print_event(&tgt, ev, false, false, false);
    return 0;
}

void ulog_log(int level, const char *file, int line, const char *topic,
              const char *message, ...) {

    lock_lock();

    // Skip if level is lower than sets
    bool is_log_allowed = level >= levels_data.level;
    if (!is_log_allowed) {
        lock_unlock();
        return;
    }

    // Try to get topic ID and check if logging is allowed for this topic
    int topic_id = TOPIC_ID_NO_TOPIC;
    topic_process(topic, level, &is_log_allowed, &topic_id);
    if (!is_log_allowed) {
        lock_unlock();
        return;  // Topic is not enabled or level is lower than topic level
    }

    ulog_Event ev = {0};
    log_fill_event(&ev, message, level, file, line, topic_id);
    va_start(ev.message_format_args, message);

    cb_handle_all(&ev);

    va_end(ev.message_format_args);

    lock_unlock();
}
