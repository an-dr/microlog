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
   Feature: Color Config (`color_cfg_*`, depends on: - )
============================================================================ */
#if ULOG_FEATURE_COLOR && ULOG_FEATURE_RUNTIME_CONFIG

typedef struct {
    bool enabled;
} color_cfg_t;

static color_cfg_t color_cfg = {
    .enabled = ULOG_FEATURE_COLOR,
};

// Private
// ================

bool color_cfg_is_enabled(void) {
    return color_cfg.enabled;
}

// Public
// ================

void ulog_configure_color(bool enabled) {
    lock_lock();  // Lock the configuration
    color_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_FEATURE_RUNTIME_CONFIG
#define color_cfg_is_enabled() (ULOG_FEATURE_COLOR)
#endif  // ULOG_FEATURE_RUNTIME_CONFIG

/* ============================================================================
   Feature: Color (`color_*`, depends on: Print, Color Config)
============================================================================ */
#if ULOG_FEATURE_COLOR

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
    if (!color_cfg_is_enabled()) {
        return;  // Color is disabled, do not print color codes
    }
    print_to_target(tgt, "%s", color_levels[ev->level]);  // color start
}

static void color_print_end(print_target *tgt) {
    if (!color_cfg_is_enabled()) {
        return;  // Color is disabled, do not print color codes
    }
    print_to_target(tgt, "%s", COLOR_TERMINATOR);  // color end
}

// Disabled Private
// ================
#else  // ULOG_FEATURE_COLOR
#define color_print_start(tgt, ev) (void)(tgt), (void)(ev)
#define color_print_end(tgt) (void)(tgt)
#endif  // ULOG_FEATURE_COLOR

/* ============================================================================
   Feature: Prefix Config (`prefix_cfg_*`, depends on: - )
============================================================================ */
#if ULOG_FEATURE_CUSTOM_PREFIX && ULOG_FEATURE_RUNTIME_CONFIG

typedef struct {
    bool enabled;
} prefix_cfg_t;

static prefix_cfg_t prefix_cfg = {
    .enabled = ULOG_FEATURE_CUSTOM_PREFIX,
};

// Private
// ================

bool prefix_cfg_is_enabled(void) {
    return prefix_cfg.enabled;
}

// Public
// ================

void ulog_configure_prefix(bool enabled) {
    lock_lock();  // Lock the configuration
    prefix_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_FEATURE_RUNTIME_CONFIG
#define prefix_cfg_is_enabled() (ULOG_FEATURE_CUSTOM_PREFIX)
#endif  // ULOG_FEATURE_RUNTIME_CONFIG

/* ============================================================================
   Feature: Prefix (`prefix_*`, depends on: Print, Prefix Config)
============================================================================ */
#if ULOG_FEATURE_CUSTOM_PREFIX

// Private
// ================
typedef struct {
    ulog_PrefixFn function;
    char prefix[ULOG_CUSTOM_PREFIX_SIZE];
} prefix_data_t;

static prefix_data_t prefix_data = {
    .function = NULL,
    .prefix   = {0},
};

static void prefix_print(print_target *tgt, ulog_Event *ev) {
    if (prefix_data.function == NULL || !prefix_cfg_is_enabled()) {
        return;
    }
    prefix_data.function(ev, prefix_data.prefix, ULOG_CUSTOM_PREFIX_SIZE);
    print_to_target(tgt, "%s", prefix_data.prefix);
}

// Public
// ================

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    prefix_data.function = function;
}

// Disabled Private
// ================
#else  // ULOG_FEATURE_CUSTOM_PREFIX
#define prefix_print(tgt, ev) (void)(tgt), (void)(ev)
#endif  // ULOG_FEATURE_CUSTOM_PREFIX

/* ============================================================================
   Feature: Time Config (`time_cfg_*`, depends on: - )
============================================================================ */
#if ULOG_FEATURE_TIME && ULOG_FEATURE_RUNTIME_CONFIG

typedef struct {
    bool enabled;
} time_cfg_t;

static time_cfg_t time_cfg = {
    .enabled = ULOG_FEATURE_TIME,
};

// Private
// ================

bool time_cfg_is_enabled(void) {
    return time_cfg.enabled;
}

// Public
// ================

void ulog_configure_time(bool enabled) {
    lock_lock();  // Lock the configuration
    time_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_FEATURE_RUNTIME_CONFIG
#define time_cfg_is_enabled() (ULOG_FEATURE_TIME)
#endif  // ULOG_FEATURE_RUNTIME_CONFIG

/* ============================================================================
   Feature: Time (`time_*`, depends on: Time, Print)
============================================================================ */
#if ULOG_FEATURE_TIME

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
    if (!time_cfg_is_enabled() || time_print_if_invalid(tgt, ev)) {
        return;  // If time is not valid or disabled, stop printing
    }
    char buf[TIME_SHORT_BUF_SIZE] = {0};
    const char *format            = append_space ? "%H:%M:%S " : "%H:%M:%S";
    strftime(buf, TIME_SHORT_BUF_SIZE, format, ev->time);
    print_to_target(tgt, "%s", buf);
}

#if ULOG_FEATURE_EXTRA_OUTPUTS
static void time_print_full(print_target *tgt, ulog_Event *ev,
                            bool append_space) {
    if (!time_cfg_is_enabled() || time_print_if_invalid(tgt, ev)) {
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
#endif  // ULOG_FEATURE_EXTRA_OUTPUTS

// Disabled Private
// ================
#else  // ULOG_FEATURE_TIME
#define time_print_short(tgt, ev, append_space) (void)(0)
#define time_print_full(tgt, ev, append_space) (void)(0)
#define time_fill_current_time(ev) (void)(ev)
#endif  // ULOG_FEATURE_TIME

/* ============================================================================
   Feature: Levels Config (`levels_cfg_*`, depends on: - )
============================================================================ */
#if ULOG_FEATURE_RUNTIME_CONFIG

typedef struct {
    bool short_levels;  // Use short level strings
} levels_cfg_t;

static levels_cfg_t levels_cfg = {
    .short_levels = FEATURE_SHORT_LEVELS || FEATURE_EMOJI_LEVELS,
};

// Private
// ================

bool levels_cfg_is_short(void) {
    return levels_cfg.short_levels;
}

// Public
// ================

void ulog_configure_levels(bool use_short_levels) {
    lock_lock();  // Lock the configuration
    levels_cfg.short_levels = use_short_levels;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_FEATURE_RUNTIME_CONFIG
#define levels_cfg_is_short() (ULOG_FEATURE_SHORT_LEVELS)
#endif  // ULOG_FEATURE_RUNTIME_CONFIG

/* ============================================================================
   Core Feature: Levels  (`levels_*`, depends on: Levels Config, Print)
============================================================================ */

// Private
// ================
#ifndef ULOG_DEFAULT_LOG_LEVEL
#define ULOG_DEFAULT_LOG_LEVEL LOG_TRACE
#endif

typedef struct {
    int level;
} levels_data_t;

static levels_data_t levels_data = {
    .level = ULOG_DEFAULT_LOG_LEVEL,
};

// clang-format off
#define LEVELS_USE_LONG     0
#define LEVELS_USE_SHORT    1
#define LEVELS_TOTAL        6

/// @brief Level strings
static const char *levels_strings[][LEVELS_TOTAL] = {
     {"TRACE",  "DEBUG",    "INFO",     "WARN",     "ERROR",    "FATAL"}
#if ULOG_FEATURE_EMOJI_LEVELS
    ,{"⚪",     "🔵",       "🟢",       "🟡",       "🔴",       "💥"}
#endif
#if !ULOG_FEATURE_EMOJI_LEVELS && ULOG_FEATURE_SHORT_LEVELS
    ,{"T",      "D",        "I",        "W",        "E",        "F"}
#endif
};
// clang-format on

static void levels_print(print_target *tgt, ulog_Event *ev) {
#if ULOG_FEATURE_SHORT_LEVELS || ULOG_FEATURE_EMOJI_LEVELS
    if (levels_cfg_is_short()) {
        print_to_target(tgt, "%-1s ",
                        levels_strings[LEVELS_USE_SHORT][ev->level]);
    } else {
        print_to_target(tgt, "%-5s ",
                        levels_strings[LEVELS_USE_LONG][ev->level]);
    }
#else
    print_to_target(tgt, "%-1s ", levels_strings[LEVELS_USE_LONG][ev->level]);
#endif  // ULOG_FEATURE_SHORT_LEVELS || ULOG_FEATURE_EMOJI_LEVELS
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
#if ULOG_FEATURE_EXTRA_OUTPUTS
#define CB_USER_NUM ULOG_EXTRA_OUTPUTS
#else
#define CB_USER_NUM 0
#endif  // ULOG_FEATURE_EXTRA_OUTPUTS

#define CB_STDOUT_ID 0
#define CB_TOTAL_NUM (1 + CB_USER_NUM)  // stdout + extra

// Forward declarations for data initialization
static void cb_stdout(ulog_Event *ev, void *arg);

typedef struct {
    ulog_LogFn function;
    void *arg;
    int level;
    bool is_enabled;  // Whether the callback is enabled or not
} cb_t;

typedef struct {
    cb_t callbacks[CB_TOTAL_NUM];  // 0 is for stdout callback
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
         (i < CB_TOTAL_NUM) && (cb_data.callbacks[i].function != NULL); i++) {
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
#if ULOG_FEATURE_EXTRA_OUTPUTS

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
    for (int i = 0; i < CB_TOTAL_NUM; i++) {
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

#endif  // ULOG_FEATURE_EXTRA_OUTPUTS

/* ============================================================================
   Feature: Topics Config (`topic_cfg_*`, depends on: - )
============================================================================ */
#if ULOG_FEATURE_TOPICS && ULOG_FEATURE_RUNTIME_CONFIG

typedef struct {
    bool enabled;
} topic_cfg_t;

static topic_cfg_t topic_cfg = {
    .enabled = ULOG_FEATURE_TOPICS,
};

// Private
// ================

bool topic_cfg_is_enabled(void) {
    return topic_cfg.enabled;
}

// Public
// ================

void ulog_configure_topics(bool enabled) {
    lock_lock();  // Lock the configuration
    topic_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_FEATURE_RUNTIME_CONFIG
#define topic_cfg_is_enabled() (ULOG_FEATURE_TOPICS)
#endif  // ULOG_FEATURE_RUNTIME_CONFIG

/* ============================================================================
   Feature: Topics (`topic_*`, depends on: Topics Config, Print, Levels)
============================================================================ */

#if ULOG_FEATURE_TOPICS

// Private
// ================
#define TOPIC_ID_NO_TOPIC -1
#define TOPIC_DYNAMIC (ULOG_TOPICS_NUM < 0)
#define TOPIC_STATIC_NUM ULOG_TOPICS_NUM

typedef struct {
    int id;
    const char *name;
    bool enabled;
    int level;

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

/// @brief Add a new topic
/// @param topic_name - Topic name
/// @param enable - Whether the topic is enabled after creation
static int topic_add(const char *topic_name, bool enable);

// === Common Topic Functions =================================================

static void topic_print(print_target *tgt, ulog_Event *ev) {
    if (!topic_cfg_is_enabled()) {
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
/// @return 0 on success, -1 if topic not found
static int topic_set_level(int topic, int level) {
    topic_t *t = topic_get(topic);
    if (t != NULL) {
        t->level = level;
        return 0;
    }
    return -1;
}

/// @brief Checks if the topic is loggable
/// @param t - Pointer to the topic, NULL is allowed
/// @param level - Log level to check against
/// @return true if loggable, false otherwise
static bool topic_is_loggable(topic_t *t, int level) {
    if (t == NULL) {
        return false;  // Topic not found, cannot log
    }
    if (!t->enabled || t->level > level) {
        return false;  // Topic is disabled, cannot log
    }
    return true;
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

    topic_t *t = topic_get(topic_str_to_id(topic));

#if TOPIC_DYNAMIC
    // Allocate a new topic if not found
    if (t == NULL) {
        *topic_id = ulog_add_topic(topic, topic_data.new_topic_enabled);
        if (*topic_id == -1) {
            *is_log_allowed = false;  // Topic was not added, processing failed
            return;                   // Processing failed, topic not found
        }
        t = topic_get(*topic_id);  // Get the newly added topic
    }
#endif  // TOPIC_DYNAMIC

    *is_log_allowed = topic_is_loggable(t, level);
    if (!*is_log_allowed) {
        return;  // Topic is not loggable, stop processing
    }
    *topic_id = t->id;  // Set topic ID
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
    return topic_enable(ulog_get_topic_id(topic_name));
}

int ulog_disable_topic(const char *topic_name) {
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

int ulog_add_topic(const char *topic_name, bool enable) {
    if (is_str_empty(topic_name)) {
        return -1;  // Invalid topic name, do nothing
    }
    return topic_add(topic_name, enable);
}

#else

#define topic_print(tgt, ev) (void)(tgt), (void)(ev)
#define topic_process(topic, level, is_log_allowed, topic_id)                  \
    (void)(topic), (void)(level), (void)(is_log_allowed), (void)(topic_id)

#endif  // ULOG_FEATURE_TOPICS

/* ============================================================================
   Feature: Topics - Static Allocation (`topic_*`, depends on: Topics)
============================================================================ */
#if ULOG_FEATURE_TOPICS && TOPIC_DYNAMIC == false
// Private
// ================

int topic_enable_all(void) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, no more to enable
        }
        topic_data.topics[i].enabled = true;
    }
    return 0;
}

int topic_disable_all(void) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        if (is_str_empty(topic_data.topics[i].name)) {
            break;  // End of topics, no more to disable
        }
        topic_data.topics[i].enabled = false;
    }
    return 0;
}

int topic_str_to_id(const char *str) {
    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
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
    if (topic < TOPIC_STATIC_NUM && topic >= 0) {
        return &topic_data.topics[topic];
    }
    return NULL;
}

static int topic_add(const char *topic_name, bool enable) {
    if (is_str_empty(topic_name)) {
        return -1;
    }

    for (int i = 0; i < TOPIC_STATIC_NUM; i++) {
        // If there is an empty slot
        if (is_str_empty(topic_data.topics[i].name)) {
            topic_data.topics[i].id      = i;
            topic_data.topics[i].name    = topic_name;
            topic_data.topics[i].enabled = enable;
            topic_data.topics[i].level   = LOG_TRACE;
            return i;
        }
        // If the topic already exists
        else if (strcmp(topic_data.topics[i].name, topic_name) == 0) {
            return i;
        }
    }
    return -1;
}
#endif  // ULOG_FEATURE_TOPICS && TOPIC_DYNAMIC == false

/* ============================================================================
   Feature: Topics - Dynamic Allocation (`topic_*`, depends on: Topics)
============================================================================ */

#if ULOG_FEATURE_TOPICS && TOPIC_DYNAMIC == true
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

static void *topic_allocate(int id, const char *topic_name, bool enable) {
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
        t->level   = LOG_TRACE;
        t->next    = NULL;
    }
    return t;
}

static int topic_add(const char *topic_name, bool enable) {
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
    topic_t *t = topic_get_last();
    if (t == NULL) {
        topic_data.topics = (topic_t *)topic_allocate(0, topic_name, enable);
        if (topic_data.topics != NULL) {
            return 0;
        }
        return -1;
    }

    // If the beginning is not empty
    t->next = topic_allocate(t->id + 1, topic_name, enable);
    if (t->next) {
        return t->id + 1;
    }
    return -1;
}

#endif  // ULOG_FEATURE_TOPICS && TOPIC_DYNAMIC == true

/* ============================================================================
   Feature: File Path Config (`file_path_cfg_*`, depends on: - )
============================================================================ */
#if ULOG_FEATURE_FILE_STRING && ULOG_FEATURE_RUNTIME_CONFIG

typedef struct {
    bool enabled;
} file_path_cfg_t;

static file_path_cfg_t file_path_cfg = {
    .enabled = FEATURE_FILE_STRING,
};

// Private
// ================

bool file_path_cfg_is_enabled(void) {
    return file_path_cfg.enabled;
}

// Public
// ================

void ulog_configure_file_string(bool enabled) {
    lock_lock();  // Lock the configuration
    file_path_cfg.enabled = enabled;
    lock_unlock();  // Unlock the configuration
}

#else  // ULOG_FEATURE_RUNTIME_CONFIG
#define file_path_cfg_is_enabled() (ULOG_FEATURE_FILE_STRING)
#endif  // ULOG_FEATURE_RUNTIME_CONFIG

/* ============================================================================
   Core Feature: Log (`log_*`, depends on: Print, Levels, Callbacks,
                      Extra Outputs, Custom Prefix, Topics, Time, Color,
                      Locking,File Path)
============================================================================ */

typedef struct {
    bool show_file_string;  // Show file and line in the log message
} log_data_t;

// Private
// ================

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void log_print_message(print_target *tgt, ulog_Event *ev) {

    if (file_path_cfg_is_enabled()) {
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
#if ULOG_FEATURE_CUSTOM_PREFIX
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
    ev->level   = level;

#if ULOG_FEATURE_FILE_STRING
    ev->file = file;
    ev->line = line;
#else
    (void)(file), (void)(line);  // Unused if file string is disabled
#endif  // ULOG_FEATURE_FILE_STRING

#if ULOG_FEATURE_TOPICS
    ev->topic = topic_id;
#else
    (void)(topic_id);  // Unused if topics are disabled
#endif

#if ULOG_FEATURE_TIME
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
    int topic_id = -1;
    if (!is_str_empty(topic)) {
        topic_process(topic, level, &is_log_allowed, &topic_id);
        if (!is_log_allowed) {
            lock_unlock();
            return;  // Topic is not enabled or level is lower than topic level
        }
    }

    ulog_Event ev = {0};
    log_fill_event(&ev, message, level, file, line, topic_id);
    va_start(ev.message_format_args, message);

    cb_handle_all(&ev);

    va_end(ev.message_format_args);

    lock_unlock();
}
