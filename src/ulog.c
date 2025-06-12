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

#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_EXTRA_OUTPUTS
    Callback callbacks[CFG_EXTRA_OUTPUTS];  // Extra callbacks
#endif

#if FEATURE_CUSTOM_PREFIX
    ulog_PrefixFn update_prefix_function;        // Custom prefix function
    char custom_prefix[CFG_CUSTOM_PREFIX_SIZE];  // Custom prefix
#endif

#ifdef FEATURE_RUNTIME_CONFIG
    ulog_config_t config; // Holds runtime configuration settings
#endif
} ulog_t;

/// @brief Main logger object himself
static ulog_t ulog = {
    .lock_function   = NULL,
    .lock_arg        = NULL,
    .level           = ULOG_DEFAULT_LOG_LEVEL, // Existing default
    .quiet           = false,                  // Existing default
    .callback_stdout = {0},

#if FEATURE_EXTRA_OUTPUTS
    // This might be superseded by config.extra_outputs_num
    .callbacks = {{0}},
#endif

#if FEATURE_CUSTOM_PREFIX
    // This might be superseded by config.custom_prefix_size
    .update_prefix_function = NULL,
    .custom_prefix          = {0},
#endif

#ifdef FEATURE_RUNTIME_CONFIG
    // Initialize config with sensible defaults or zero-initialize.
    // These defaults should ideally mirror the compile-time defaults.
    .config = {
        .enable_time = FEATURE_TIME, // Use existing macros for default values
        .enable_color = FEATURE_COLOR,
        .custom_prefix_size = CFG_CUSTOM_PREFIX_SIZE,
        .enable_file_string = FEATURE_FILE_STRING,
        .short_level_strings = FEATURE_SHORT_LEVELS,
        .use_emoji_levels = FEATURE_EMOJI_LEVELS,
        .extra_outputs_num = CFG_EXTRA_OUTPUTS,
        .topics_num = CFG_TOPICS_NUM, // This needs care if -1 is for dynamic
        .default_log_level = ULOG_DEFAULT_LOG_LEVEL
    }
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
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_COLOR

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

#endif  // defined(FEATURE_RUNTIME_CONFIG) || FEATURE_COLOR

/* ============================================================================
   Feature: Time
============================================================================ */
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TIME

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

#endif  // defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TIME

/* ============================================================================
   Feature: Custom Prefix
============================================================================ */
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_CUSTOM_PREFIX

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    ulog.update_prefix_function = function;
}

static void print_prefix(log_target *tgt, ulog_Event *ev) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.custom_prefix_size > 0 && ulog.update_prefix_function) {
        // Ensure custom_prefix buffer in ulog_t is large enough (CFG_CUSTOM_PREFIX_SIZE)
        // but only use up to ulog.config.custom_prefix_size.
        // The actual buffer ulog.custom_prefix is sized by CFG_CUSTOM_PREFIX_SIZE.
        // So, we pass min(ulog.config.custom_prefix_size, CFG_CUSTOM_PREFIX_SIZE)
        // to the callback to prevent buffer overflow if runtime asks for more than compiled.
        // However, ulog.config.custom_prefix_size is initialized with CFG_CUSTOM_PREFIX_SIZE.
        // If user sets it higher, it's their responsibility or we should cap it.
        // For now, assume ulog.config.custom_prefix_size is <= CFG_CUSTOM_PREFIX_SIZE or
        // that ulog.custom_prefix is dynamically sized (which it is not currently).
        // The safest is to use the compile-time allocated size for the buffer.
        // The runtime config's custom_prefix_size then indicates *if* prefix is used and its *intended* length.
        // Let's assume ulog.custom_prefix is always CFG_CUSTOM_PREFIX_SIZE.
        // The callback should respect the passed size.
        ulog.update_prefix_function(ev, ulog.custom_prefix, CFG_CUSTOM_PREFIX_SIZE); // Pass buffer capacity
        // The string produced by update_prefix_function should ideally be null-terminated within this capacity.
        print(tgt, "%s", ulog.custom_prefix);
    }
#else // !FEATURE_RUNTIME_CONFIG
    // Original compile-time path (FEATURE_CUSTOM_PREFIX is true here)
    if (ulog.update_prefix_function) {
        ulog.update_prefix_function(ev, ulog.custom_prefix,
                                    CFG_CUSTOM_PREFIX_SIZE);
        print(tgt, "%s", ulog.custom_prefix);
    }
#endif // FEATURE_RUNTIME_CONFIG
}

#endif  // defined(FEATURE_RUNTIME_CONFIG) || FEATURE_CUSTOM_PREFIX

/* ============================================================================
   Feature: Extra Outputs
============================================================================ */
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_EXTRA_OUTPUTS

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
    if (CFG_EXTRA_OUTPUTS == 0) return -1; // No space allocated for callbacks

    int active_limit = 0;
#ifdef FEATURE_RUNTIME_CONFIG
    active_limit = ulog.config.extra_outputs_num;
    if (active_limit <= 0) return -1; // Runtime configured to have no outputs
#else // FEATURE_EXTRA_OUTPUTS is true
    active_limit = CFG_EXTRA_OUTPUTS;
    if (active_limit <= 0) return -1; // Should not happen if FEATURE_EXTRA_OUTPUTS is true and CFG_EXTRA_OUTPUTS > 0
#endif

    // Loop up to the minimum of physical capacity (CFG_EXTRA_OUTPUTS) and configured active_limit
    int loop_limit = (active_limit < CFG_EXTRA_OUTPUTS) ? active_limit : CFG_EXTRA_OUTPUTS;

    for (int i = 0; i < loop_limit; i++) {
        if (!ulog.callbacks[i].function) {
            ulog.callbacks[i] = (Callback){function, arg, level};
            return 0;
        }
    }
    return -1; // No free slot found within the active_limit
}

/// @brief Add file callback
int ulog_add_fp(FILE *fp, int level) {
    // This function relies on ulog_add_callback, which now has the correct checks.
    return ulog_add_callback(callback_file, fp, level);
}

/// @brief Processes the extra callbacks
/// @param ev - Event
static void log_to_extra_outputs(ulog_Event *ev) {
    if (CFG_EXTRA_OUTPUTS == 0) return; // No space allocated for callbacks

    int active_limit = 0;
#ifdef FEATURE_RUNTIME_CONFIG
    active_limit = ulog.config.extra_outputs_num;
    if (active_limit <= 0) return; // Runtime configured to have no outputs
#else // FEATURE_EXTRA_OUTPUTS is true
    active_limit = CFG_EXTRA_OUTPUTS;
    if (active_limit <= 0) return; // Should not happen
#endif

    // Loop up to the minimum of physical capacity (CFG_EXTRA_OUTPUTS) and configured active_limit
    int loop_limit = (active_limit < CFG_EXTRA_OUTPUTS) ? active_limit : CFG_EXTRA_OUTPUTS;

    for (int i = 0; i < loop_limit && ulog.callbacks[i].function; i++) {
        process_callback(ev, &ulog.callbacks[i]);
    }
}

#endif  // defined(FEATURE_RUNTIME_CONFIG) || FEATURE_EXTRA_OUTPUTS

/* ============================================================================
   Feature: Log Topics
============================================================================ */
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS

typedef struct {
    int id;
    const char *name;
    bool enabled;
    int level;

#if CFG_TOPICS_DINAMIC_ALLOC == true
    void *next;  // Pointer to the next topic pointer (Topic **)
#endif

} Topic;

static bool is_topic_enabled(int topic);
static int _ulog_set_topic_level(int topic, int level);
static int _ulog_enable_topic(int topic);
static int _ulog_disable_topic(int topic);
static Topic *_get_topic_begin(void);
static Topic *_get_topic_next(Topic *t);
static Topic *_get_topic_ptr(int topic);

static bool new_topic_enabled = false;

static void print_topic(log_target *tgt, ulog_Event *ev) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0) { // Topics explicitly disabled by runtime config
        return;
    }
    // If topics_num is > 0 or -1 (dynamic), proceed.
#else
    // FEATURE_TOPICS is true here (due to outer #if), so topics are enabled by compile-time.
#endif

    Topic *t = _get_topic_ptr(ev->topic);
    if (t && t->name) {
        print(tgt, "[%s] ", t->name);
    }
}

static int _ulog_set_topic_level(int topic, int level) {
    Topic *t = _get_topic_ptr(topic);
    if (t) {
        t->level = level;
        return 0;
    }
    return -1;
}

static int _ulog_get_topic_level(int topic) {
    Topic *t = _get_topic_ptr(topic);
    if (t) {
        return t->level;
    }
    return LOG_TRACE;
}

static int _ulog_enable_topic(int topic) {
    Topic *t = _get_topic_ptr(topic);
    if (t) {
        t->enabled = true;
        return 0;
    }
    return -1;
}

static int _ulog_disable_topic(int topic) {
    Topic *t = _get_topic_ptr(topic);
    if (t) {
        t->enabled = false;
        return 0;
    }
    return -1;
}

int ulog_set_topic_level(const char *topic_name, int level) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0) return -1; // Topics disabled
#endif
    if (ulog_add_topic(topic_name, true) != -1) { // ulog_add_topic also checks config.topics_num
        return _ulog_set_topic_level(ulog_get_topic_id(topic_name), level);
    }
    return -1;
}

int ulog_enable_topic(const char *topic_name) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0) return -1; // Topics disabled
#endif
    ulog_add_topic(topic_name, true); // ulog_add_topic also checks config.topics_num
    return _ulog_enable_topic(ulog_get_topic_id(topic_name));
}

int ulog_disable_topic(const char *topic_name) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0) return -1; // Topics disabled
#endif
    ulog_add_topic(topic_name, false); // ulog_add_topic also checks config.topics_num
    return _ulog_disable_topic(ulog_get_topic_id(topic_name));
}

int ulog_enable_all_topics(void) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0 && ulog.config.topics_num != -1 /* allow enable all for dynamic */) return -1; // Topics disabled (unless dynamic)
#endif
    new_topic_enabled = true;
    // Iterate only over active/allocatable topics if applicable
    // For static topics, runtime config might limit the effective number of topics.
    // However, _get_topic_begin/_get_topic_next iterate physical list.
    // This is okay, as individual topic structs store enabled state.
    // If topics_num (runtime) < CFG_TOPICS_NUM (compile), this will enable beyond runtime limit,
    // but they won't be used/found by get_topic_id if it respects runtime limits.
    // This is acceptable; they are just marked enabled in their struct.
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        t->enabled = true;
    }
    return 0;
}

int ulog_disable_all_topics(void) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0 && ulog.config.topics_num != -1) return -1; // Topics disabled (unless dynamic)
#endif
    new_topic_enabled = false;
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        t->enabled = false;
    }
    return 0;
}

int ulog_get_topic_id(const char *topic_name) {
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0) return TOPIC_NOT_FOUND; // Topics disabled
    // If topics_num > 0 (runtime static count), we should only search within that limit.
    // However, _get_topic_begin/_get_topic_next iterate physical list.
    // This might be an issue if runtime count < compile count for static topics.
    // For now, search full physical list. ulog_add_topic (static) respects runtime limit for adding.
#endif
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        if (t->name && strcmp(t->name, topic_name) == 0) {
            // Additional check for static topics with runtime limit:
#if defined(FEATURE_RUNTIME_CONFIG) && CFG_TOPICS_DINAMIC_ALLOC == false
            if (ulog.config.topics_num > 0 && t->id >= ulog.config.topics_num) {
                return TOPIC_NOT_FOUND; // Found beyond runtime configured limit
            }
#endif
            return t->id;
        }
    }
    return TOPIC_NOT_FOUND;
}

/// @brief Checks if the topic is enabled
/// @param topic - Topic ID or -1 if no topic
/// @return true if enabled or no topic, false otherwise
static bool is_topic_enabled(int topic) {
    if (topic < 0) {  // no topic, always allowed
        return true;
    }

    Topic *t = _get_topic_ptr(topic);
    if (t) {
        return t->enabled;
    }
    return false;
}

#endif  // defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS

/* ============================================================================
   Feature: Log Topics - Static Allocation
============================================================================ */
#if (defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS) && CFG_TOPICS_DINAMIC_ALLOC == false

static Topic topics[CFG_TOPICS_NUM] = {{0}};

static Topic *_get_topic_begin(void) {
    return topics;
}

static Topic *_get_topic_next(Topic *t) {
    if ((t >= topics) && (t < topics + CFG_TOPICS_NUM - 1)) {
        return t + 1;
    }
    return NULL;
}

static Topic *_get_topic_ptr(int topic) {
    if (topic < CFG_TOPICS_NUM) {
        return &topics[topic];
    }
    return NULL;
}

int ulog_add_topic(const char *topic_name, bool enable) {
    // This function is within the block: #if (defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS) && CFG_TOPICS_DINAMIC_ALLOC == false
    if (CFG_TOPICS_NUM == 0) return -1; // No static topic array allocated.

#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.topics_num == 0) return -1; // Topics explicitly disabled by runtime.
    // If ulog.config.topics_num == -1 (dynamic mode by runtime), this static ulog_add_topic shouldn't be hit
    // because CFG_TOPICS_DINAMIC_ALLOC would be true.
    // So, ulog.config.topics_num > 0 here.
    int active_limit = (ulog.config.topics_num < CFG_TOPICS_NUM) ? ulog.config.topics_num : CFG_TOPICS_NUM;
    if (active_limit <=0) return -1; // Should be caught by topics_num == 0, but defensive.
#else // Compile-time path (FEATURE_TOPICS is true, CFG_TOPICS_DINAMIC_ALLOC == false)
    // CFG_TOPICS_NUM > 0 here.
    int active_limit = CFG_TOPICS_NUM;
#endif

    for (int i = 0; i < active_limit; i++) {
        // First, check if topic already exists (within the active_limit)
        if (topics[i].name && strcmp(topics[i].name, topic_name) == 0) {
            return topics[i].id; // Return existing ID
        }
    }
    // If not found, try to add it if there's space within active_limit
    for (int i = 0; i < active_limit; i++) {
        if (!topics[i].name) { // Find first empty slot
            topics[i].id      = i;
            topics[i].name    = topic_name;
            topics[i].enabled = enable;
            topics[i].level   = ULOG_DEFAULT_LOG_LEVEL;
            return i;
        }
    }
    return -1;
}
#endif  // (defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS) && CFG_TOPICS_DINAMIC_ALLOC == false

/* ============================================================================
   Feature: Log Topics - Dynamic Allocation
============================================================================ */

#if (defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS) && CFG_TOPICS_DINAMIC_ALLOC == true

#include <stdlib.h>

static Topic *topics = NULL;

static Topic *_get_topic_begin(void) {
    return topics;
}

static Topic *_get_topic_next(Topic *t) {
    return t->next;
}

static Topic *_get_topic_ptr(int topic) {
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        if (t->id == topic) {
            return t;
        }
    }
    return NULL;
}

static void *_create_topic(int id, const char *topic_name, bool enable) {
    Topic *t = malloc(sizeof(Topic));
    if (t) {
        t->id      = id;
        t->name    = topic_name;
        t->enabled = enable;
        t->next    = NULL;
    }
    return t;
}

int ulog_add_topic(const char *topic_name, bool enable) {
    if (!topic_name) {
        return -1;
    }

    // if exists
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        if (t->name && strcmp(t->name, topic_name) == 0) {
            return t->id;
        }
    }

    // If the beginning is empty
    if (!topics) {
        topics = (Topic *)_create_topic(0, topic_name, enable);
        if (topics) {
            return 0;
        }
        return -1;
    }

    // If the beginning is not empty
    int current_id = 0;
    Topic *t       = topics;
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

#endif  // (defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS) && CFG_TOPICS_DINAMIC_ALLOC == true

// This standalone #if FEATURE_TOPICS is redundant due to the section guard above.
// Removing it or ensuring it matches the section guard.
// For now, let's assume it should align with the section guard.
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS

#endif  // defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS

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

#ifdef FEATURE_RUNTIME_CONFIG
// clang-format off
static const char *level_strings_full[] = {
    "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};
static const char *level_strings_short[] = {
    "T", "D", "I", "W", "E", "F"
};
static const char *level_strings_emoji[] = {
    "⚪", "🔵", "🟢", "🟡", "🔴", "💥"
};
// clang-format on
#else // !FEATURE_RUNTIME_CONFIG
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
#endif // FEATURE_RUNTIME_CONFIG

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

#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.enable_color && color) {
        print_color_start(tgt, ev);
    }
#else // !FEATURE_RUNTIME_CONFIG
#if FEATURE_COLOR
    if (color) {
        print_color_start(tgt, ev);
    }
#else
    // In original code, if !FEATURE_COLOR, (void)color was here.
    // It's implicitly handled now by 'color' param potentially being unused.
#endif  // FEATURE_COLOR
#endif // FEATURE_RUNTIME_CONFIG

#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.enable_time) {
        if (full_time) {
// print_time_full is guarded by (FEATURE_RUNTIME_CONFIG || FEATURE_EXTRA_OUTPUTS)
// and (FEATURE_RUNTIME_CONFIG || FEATURE_TIME)
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_EXTRA_OUTPUTS
            print_time_full(tgt, ev);
#else
            print_time_sec(tgt, ev); // Fallback if print_time_full not compiled
#endif
        } else {
            print_time_sec(tgt, ev);
        }
    }
#else // !FEATURE_RUNTIME_CONFIG
#if FEATURE_TIME
    if (full_time) {
#if FEATURE_EXTRA_OUTPUTS
        print_time_full(tgt, ev);
#endif
    } else {
        print_time_sec(tgt, ev);
    }
#else
    (void)full_time; // full_time is unused if FEATURE_TIME is false
#endif  // FEATURE_TIME
#endif // FEATURE_RUNTIME_CONFIG

#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_CUSTOM_PREFIX
    // print_prefix internally checks if it should run based on config
    print_prefix(tgt, ev);
#endif

#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS
    // print_topic internally checks if it should run based on config
    print_topic(tgt, ev);
#endif

    print_level(tgt, ev);

    print_message(tgt, ev);

#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.enable_color && color) {
        print_color_end(tgt, ev);
    }
#else // !FEATURE_RUNTIME_CONFIG
#if FEATURE_COLOR
    if (color) {
        print_color_end(tgt, ev);
    }
#endif // FEATURE_COLOR
#endif // FEATURE_RUNTIME_CONFIG

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

#if defined(FEATURE_RUNTIME_CONFIG)
    // Runtime Configuration Path
    int topic_id_for_event = -1; // Default for no topic or if topics are disabled
    if (ulog.config.topics_num != 0 && topic != NULL) { // Topics enabled by runtime (val > 0 or -1) AND topic string provided
        int current_topic_id = ulog_get_topic_id(topic);
        if (current_topic_id == TOPIC_NOT_FOUND) {
            #if CFG_TOPICS_DINAMIC_ALLOC == true // Dynamic allocation compiled in
                // Only add topic if runtime config explicitly allows dynamic topics
                if (ulog.config.topics_num == -1) {
                     current_topic_id = ulog_add_topic(topic, new_topic_enabled);
                } else { // Runtime config has a fixed number of topics (ulog.config.topics_num > 0) and it wasn't found
                    return; // Topic not in runtime-defined static list
                }
            #else // No dynamic allocation compiled.
                return; // Topic not in compile-time defined static list (which runtime config might be using)
            #endif
        }

        // If current_topic_id is still TOPIC_NOT_FOUND or -1 (error from add_topic), then filter out.
        if (current_topic_id == TOPIC_NOT_FOUND || current_topic_id == -1) {
             return;
        }
        // Check topic's own level and enabled status
        if (!is_topic_enabled(current_topic_id) || (level < _ulog_get_topic_level(current_topic_id))) {
            return;
        }
        topic_id_for_event = current_topic_id;
    } else if (ulog.config.topics_num == 0 && topic != NULL) {
        // Topics are disabled by runtime config (topics_num == 0), but a topic string was given.
        // This means the message should be filtered out as it belongs to a (now disabled) topic.
        return;
    }
    // If topic == NULL, topic_id_for_event remains -1 (no topic filtering based on topic name).
    // If ulog.config.topics_num == 0 and topic == NULL, message is logged (not belonging to any specific topic).

#elif FEATURE_TOPICS // Compile-time Path: FEATURE_RUNTIME_CONFIG is false, and FEATURE_TOPICS is true
    int topic_id_for_event = -1;
    if (topic != NULL) {
        int current_topic_id = ulog_get_topic_id(topic);
        if (current_topic_id == TOPIC_NOT_FOUND) {
            #if CFG_TOPICS_DINAMIC_ALLOC == false
            return; // Topic not found in static list
            #else
            current_topic_id = ulog_add_topic(topic, new_topic_enabled); // Attempt to add dynamically
            #endif
        }
        // Check topic_id after attempting to add
        if (current_topic_id == TOPIC_NOT_FOUND || current_topic_id == -1) {
            return; // Failed to find or add topic
        }
        if (!is_topic_enabled(current_topic_id) || (level < _ulog_get_topic_level(current_topic_id))) {
            return;
        }
        topic_id_for_event = current_topic_id;
    }
    // If topic == NULL, topic_id_for_event remains -1.
#else
    // FEATURE_RUNTIME_CONFIG is false, and FEATURE_TOPICS is false (compile-time)
    // (void)topic; // topic parameter is unused.
    // No topic_id_for_event is needed as ev.topic is not part of ulog_Event in this case.
#endif

    ulog_Event ev = {
        .message = message,
        .file    = file,
        .line    = line,
        .level   = level,
#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TOPICS
        .topic = topic_id_for_event, // Use the determined topic_id
#endif
    };

    va_start(ev.message_format_args, message);

    lock();

    log_to_stdout(&ev);

#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_EXTRA_OUTPUTS
    log_to_extra_outputs(&ev);
#endif

    unlock();

    va_end(ev.message_format_args);
}

static void print_level(log_target *tgt, ulog_Event *ev) {
    // %-1s ensures minimum width of 1, but emoji are wider.
    // For emojis, we might want a different format string or just accept the variable width.
    // For simplicity, we'll use a fixed approach for now.
    // Consider "%-5s" for short, "%-8s" for full if alignment is critical and emojis are not used.
    // However, existing code uses "%-1s" which works well for single char short levels.
    // Emojis will naturally take more space.
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.use_emoji_levels) {
        print(tgt, "%s ", ulog_get_level_string(ev->level)); // Emoji + space
    } else if (ulog.config.short_level_strings) {
        print(tgt, "%-1s ", ulog_get_level_string(ev->level)); // Short (T, D) + space
    } else {
        print(tgt, "%-5s ", ulog_get_level_string(ev->level)); // Full (TRACE) + space
    }
#else
    print(tgt, "%-1s ", ulog_get_level_string(ev->level));
#endif
}

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void print_message(log_target *tgt, ulog_Event *ev) {

#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.enable_file_string) {
        print(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
    }
#else // !FEATURE_RUNTIME_CONFIG
#if FEATURE_FILE_STRING
    print(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
#endif // FEATURE_FILE_STRING
#endif // FEATURE_RUNTIME_CONFIG

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

#if defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TIME
        if (!ev->time) { // Only try to initialize if not already set
#ifdef FEATURE_RUNTIME_CONFIG
            if (ulog.config.enable_time) {
                time_t t = time(NULL);
                ev->time = localtime(&t);
            }
#else // !FEATURE_RUNTIME_CONFIG
            // This is the original compile-time only path, FEATURE_TIME is true here
            time_t t = time(NULL);
            ev->time = localtime(&t);
#endif // FEATURE_RUNTIME_CONFIG
        }
#endif // defined(FEATURE_RUNTIME_CONFIG) || FEATURE_TIME

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
#ifdef FEATURE_RUNTIME_CONFIG
    if (ulog.config.use_emoji_levels) {
        return level_strings_emoji[level];
    } else if (ulog.config.short_level_strings) {
        return level_strings_short[level];
    } else {
        return level_strings_full[level];
    }
#else // !FEATURE_RUNTIME_CONFIG
    return level_strings[level];
#endif // FEATURE_RUNTIME_CONFIG
}

/// @brief Sets the debug level
void ulog_set_level(int level) {
    ulog.level = level;
}

/// @brief Sets the quiet mode
void ulog_set_quiet(bool enable) {
    ulog.quiet = enable;
}

#ifdef FEATURE_RUNTIME_CONFIG
int ulog_init_config(const ulog_config_t *config) {
    if (!config) {
        return -1; // Config pointer is NULL
    }

    // Apply the new configuration
    // Consider thread safety if ulog can be configured while other threads are logging
    // lock(); // If a lock function is set and initialization needs to be atomic

    ulog.config = *config; // Copy the provided configuration

    // Apply fundamental settings from the new config to the global ulog state
    ulog.level = ulog.config.default_log_level;
    // ulog.quiet will retain its value unless explicitly part of ulog_config_t and set here.

    // Other settings from ulog.config (like enable_time, enable_color, etc.)
    // will be checked at the point of use (e.g., in write_formatted_message)
    // or would require more extensive re-initialization if they changed global
    // structures (e.g., if CFG_EXTRA_OUTPUTS was not compile-time fixed).

    // unlock(); // Release lock if taken

    return 0; // Success
}
#endif // FEATURE_RUNTIME_CONFIG
