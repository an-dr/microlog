// *************************************************************************
//
// ulog v6.3.0 - A simple customizable logging library.
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

// Global ulog configuration instance
// Default values will be set by ulog_init based on compile-time flags.
ulog_config_t g_ulog_config; // Will be initialized by ulog_init

// Flag to track if ulog_init has been called
static bool s_ulog_initialized = false;

// Conditional include for string.h based on ULOG_TOPICS_NUM (compile-time)
#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0
#include <string.h>
#endif

#define ULOG_NEW_LINE_ON true
#define ULOG_NEW_LINE_OFF false
#define ULOG_COLOR_ON true
#define ULOG_COLOR_OFF false
#define ULOG_TIME_FULL true
#define ULOG_TIME_SHORT false

// Helper for min functionality, as standard C doesn't have a min macro/function
#define ULOG_MIN(a, b) ((a) < (b) ? (a) : (b))

void ulog_init(const ulog_config_t *config) {
    if (s_ulog_initialized && config != NULL) {
        // If already initialized and a new config is provided, re-initialize with new config.
        // This allows changing config on the fly.
    } else if (s_ulog_initialized && config == NULL) {
        // If already initialized and NULL is passed, do nothing (don't reset to defaults).
        return;
    }

    if (config == NULL) {
        // Default Initialization
        g_ulog_config.level                 = LOG_TRACE;
        g_ulog_config.quiet                 = false;
        g_ulog_config.time_enabled          = false; // Base default
        g_ulog_config.color_enabled         = true;  // Base default
        g_ulog_config.custom_prefix_size    = 0;     // Base default
        g_ulog_config.file_string_enabled   = true;  // Base default
        g_ulog_config.short_level_strings   = false; // Base default
        g_ulog_config.emoji_levels          = false; // Base default
        g_ulog_config.extra_outputs         = 0;     // Base default
        g_ulog_config.topics_num            = 0;     // Base default
        g_ulog_config.topics_dynamic_alloc  = false; // Base default

        // Compile-time Flag Overrides for default init
#ifdef ULOG_HAVE_TIME
        g_ulog_config.time_enabled = true;
#endif
#ifdef ULOG_NO_COLOR
        g_ulog_config.color_enabled = false;
#endif
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
        g_ulog_config.custom_prefix_size = ULOG_CUSTOM_PREFIX_SIZE;
#else
        g_ulog_config.custom_prefix_size = 0;
#endif
#ifdef ULOG_HIDE_FILE_STRING
        g_ulog_config.file_string_enabled = false;
#endif
#ifdef ULOG_SHORT_LEVEL_STRINGS
        g_ulog_config.short_level_strings = true;
#endif
#ifdef ULOG_USE_EMOJI
        g_ulog_config.emoji_levels = true;
        g_ulog_config.short_level_strings = false; // Emoji takes precedence
#endif
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
        g_ulog_config.extra_outputs = ULOG_EXTRA_OUTPUTS;
#else
        g_ulog_config.extra_outputs = 0;
#endif
#ifdef ULOG_TOPICS_NUM
    #if ULOG_TOPICS_NUM > 0
        g_ulog_config.topics_num = ULOG_TOPICS_NUM;
        g_ulog_config.topics_dynamic_alloc = false;
    #elif ULOG_TOPICS_NUM == -1
        g_ulog_config.topics_num = -1; 
        g_ulog_config.topics_dynamic_alloc = true;
    #else 
        g_ulog_config.topics_num = 0;
        g_ulog_config.topics_dynamic_alloc = false;
    #endif
#else 
        g_ulog_config.topics_num = 0;
        g_ulog_config.topics_dynamic_alloc = false;
#endif
    } else {
        // User-provided Configuration
        g_ulog_config = *config;

        // Re-apply restrictive compile-time flag overrides
#ifndef ULOG_HAVE_TIME
        g_ulog_config.time_enabled = false;
#endif
#ifdef ULOG_NO_COLOR 
        g_ulog_config.color_enabled = false;
#endif
#ifdef ULOG_HIDE_FILE_STRING 
        g_ulog_config.file_string_enabled = false;
#endif
        // Ensure runtime size does not exceed compile-time allocation for static arrays
#if defined(ULOG_CUSTOM_PREFIX_SIZE)
        g_ulog_config.custom_prefix_size = ULOG_MIN(g_ulog_config.custom_prefix_size, ULOG_CUSTOM_PREFIX_SIZE);
#else
        g_ulog_config.custom_prefix_size = 0; // Compiled out
#endif
#if defined(ULOG_EXTRA_OUTPUTS)
        g_ulog_config.extra_outputs = ULOG_MIN(g_ulog_config.extra_outputs, ULOG_EXTRA_OUTPUTS);
#else
        g_ulog_config.extra_outputs = 0; // Compiled out
#endif

#ifdef ULOG_TOPICS_NUM
    #if ULOG_TOPICS_NUM == 0 // Topics completely compiled out
        g_ulog_config.topics_num = 0;
        g_ulog_config.topics_dynamic_alloc = false;
    #elif ULOG_TOPICS_NUM > 0 // Static topics
        g_ulog_config.topics_dynamic_alloc = false; // Cannot be dynamic if compiled static
        g_ulog_config.topics_num = ULOG_MIN(g_ulog_config.topics_num, ULOG_TOPICS_NUM);
    #else // ULOG_TOPICS_NUM == -1 (Dynamic topics)
        g_ulog_config.topics_dynamic_alloc = true; // Must be dynamic if compiled dynamic
        // topics_num for dynamic can be set by user (e.g. as a hint or soft limit, though not strictly enforced by ulog core)
        // or defaults to -1 if user sets 0 or positive but dynamic_alloc is true.
        if (g_ulog_config.topics_num >=0) {
            // if user provides 0 or positive, and we are in dynamic mode, it means "no limit" or "user managed limit"
            // for simplicity, let's keep it as is, or map 0 to -1 to signify "no internal limit"
            // However, the problem description for ULOG_TOPICS_NUM = -1 implies g_ulog_config.topics_num = -1
            // So, if user provides something else, it's a bit ambiguous.
            // Let's stick to: if compiled dynamic, g_ulog_config.topics_num can be what user provides (if >0 means soft limit) or -1.
            // For now, if user set it to 0 or positive, we assume they are aware of its meaning in dynamic context.
            // Forcing it to -1 might override a user's intended soft limit.
            // The most important is topics_dynamic_alloc = true.
        } else { // user provided negative topics_num
            g_ulog_config.topics_num = -1; // Standardize to -1 for dynamic "no limit"
        }
    #endif
#else // ULOG_TOPICS_NUM not defined
        g_ulog_config.topics_num = 0;
        g_ulog_config.topics_dynamic_alloc = false;
#endif
    }
    s_ulog_initialized = true;
}

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
    Callback callback_stdout;   // to stdout

// ULOG_EXTRA_OUTPUTS is a compile-time constant
// and determines the size of this array.
// g_ulog_config.extra_outputs (runtime) will control how many are used, up to ULOG_EXTRA_OUTPUTS.
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
    Callback callbacks[ULOG_EXTRA_OUTPUTS];  // Extra callbacks
#endif

// ULOG_CUSTOM_PREFIX_SIZE is a compile-time constant
// and determines the size of this array.
// g_ulog_config.custom_prefix_size (runtime) will control if it's used.
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    ulog_PrefixFn update_prefix_function;        // Custom prefix function
    char custom_prefix[ULOG_CUSTOM_PREFIX_SIZE];  // Custom prefix
#endif

} ulog_t;

/// @brief Main logger object himself
static ulog_t ulog = {
    .lock_function   = NULL,
    .lock_arg        = NULL,
    .callback_stdout = {0},

#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
    .callbacks = {{0}},
#endif

#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    .update_prefix_function = NULL,
    .custom_prefix          = {0},
#endif

};

/* ============================================================================
   Output Printing
============================================================================ */

typedef struct {
    char *data;
    size_t size;
} span;

typedef union {
    span buffer;
    FILE *stream;
} log_target_descriptor;

typedef enum { T_BUFFER, T_STREAM } log_target_type;

typedef struct {
    log_target_type type;
    log_target_descriptor dsc;
} log_target;

static void vprint(const log_target *tgt, const char *format, va_list args) {
    if (tgt->type == T_BUFFER) {
        char *buf   = tgt->dsc.buffer.data;
        size_t size = tgt->dsc.buffer.size;
        if (size > 0) {
            vsnprintf(buf, size, format, args);
        }
    } else if (tgt->type == T_STREAM) {
        FILE *stream = tgt->dsc.stream;
        vfprintf(stream, format, args);
    }
}

static void print(const log_target *tgt, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprint(tgt, format, args);
    va_end(args);
}

/* ============================================================================
   Prototypes
============================================================================ */

static void print_level(const log_target *tgt, ulog_Event *ev);
static void print_message(const log_target *tgt, ulog_Event *ev);
static void process_callback(ulog_Event *ev, Callback *cb);
static void write_formatted_message(const log_target *tgt, ulog_Event *ev,
                                    bool full_time, bool color, bool new_line);

/* ============================================================================
   Feature: Color
============================================================================ */
// ULOG_NO_COLOR is a compile-time flag. If not defined, color support is compiled.
#ifndef ULOG_NO_COLOR
static const char *level_colors[] = {
    "\x1b[37m",  // TRACE : White #000
    "\x1b[36m",  // DEBUG : Cyan #0ff
    "\x1b[32m",  // INFO : Green #0f0
    "\x1b[33m",  // WARN : Yellow #ff0
    "\x1b[31m",  // ERROR : Red #f00
    "\x1b[35m"   // FATAL : Magenta #f0f
};

static void print_color_start(const log_target *tgt, ulog_Event *ev) {
    (void)ev;
    // Runtime check for color_enabled is done by the caller (write_formatted_message)
    print(tgt, "%s", level_colors[ev->level]);
}

static void print_color_end(const log_target *tgt, ulog_Event *ev) {
    (void)ev;
    // Runtime check for color_enabled is done by the caller (write_formatted_message)
    print(tgt, "\x1b[0m");
}
#endif  // !ULOG_NO_COLOR

/* ============================================================================
   Feature: Time
============================================================================ */
// ULOG_HAVE_TIME is a compile-time flag.
#ifdef ULOG_HAVE_TIME

static void print_time_sec(const log_target *tgt, ulog_Event *ev) {
    char buf[10]; 
    bool custom_prefix_active = false;
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    custom_prefix_active = g_ulog_config.custom_prefix_size > 0;
#endif
    if (custom_prefix_active) { // Format without trailing space if custom prefix is active
        buf[strftime(buf, 9, "%H:%M:%S", ev->time)] = '\0'; 
    } else {
        buf[strftime(buf, sizeof(buf), "%H:%M:%S ", ev->time)] = '\0'; 
    }
    print(tgt, "%s", buf);
}

#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
static void print_time_full(const log_target *tgt, ulog_Event *ev) {
    char buf[65]; 
    bool custom_prefix_active = false;
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    custom_prefix_active = g_ulog_config.custom_prefix_size > 0;
#endif
    if (custom_prefix_active) { // Format without trailing space
        buf[strftime(buf, 64, "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    } else {
        buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S ", ev->time)] = '\0';
    }
    print(tgt, "%s", buf);
}
#endif  // ULOG_EXTRA_OUTPUTS > 0
#endif  // ULOG_HAVE_TIME

/* ============================================================================
   Feature: Custom Prefix
============================================================================ */
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
void ulog_set_prefix_fn(ulog_PrefixFn function) {
    ulog.update_prefix_function = function;
}

static void print_prefix(const log_target *tgt, ulog_Event *ev) {
    // This function is called only if g_ulog_config.custom_prefix_size > 0
    if (ulog.update_prefix_function) {
        ulog.update_prefix_function(ev, ulog.custom_prefix, ULOG_CUSTOM_PREFIX_SIZE);
        print(tgt, "%s", ulog.custom_prefix);
    }
}
#endif  // ULOG_CUSTOM_PREFIX_SIZE > 0

/* ============================================================================
   Feature: Extra Outputs
============================================================================ */
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0

/// @brief Callback for file
/// @param arg - File pointer
static void callback_file(ulog_Event *ev, void *arg) {
    log_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    write_formatted_message(&tgt, ev, ULOG_TIME_FULL, ULOG_COLOR_OFF, ULOG_NEW_LINE_ON);
}

int ulog_add_callback(ulog_LogFn function, void *arg, int level) {
    // Use g_ulog_config.extra_outputs to iterate, bounded by compile-time ULOG_EXTRA_OUTPUTS
    for (int i = 0; i < g_ulog_config.extra_outputs && i < ULOG_EXTRA_OUTPUTS; i++) {
        if (!ulog.callbacks[i].function) {
            ulog.callbacks[i] = (Callback){function, arg, level};
            return 0;
        }
    }
    return -1;
}

int ulog_add_fp(FILE *fp, int level) {
    return ulog_add_callback(callback_file, fp, level);
}

static void log_to_extra_outputs(ulog_Event *ev) {
    // Use g_ulog_config.extra_outputs to iterate, bounded by compile-time ULOG_EXTRA_OUTPUTS
    for (int i = 0; i < g_ulog_config.extra_outputs && i < ULOG_EXTRA_OUTPUTS; i++) {
        if (ulog.callbacks[i].function) { 
            process_callback(ev, &ulog.callbacks[i]);
        }
    }
}
#endif  // ULOG_EXTRA_OUTPUTS > 0

/* ============================================================================
   Feature: Log Topics
============================================================================ */
// ULOG_TOPICS_NUM is a compile-time define.
#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0

typedef struct Topic_s {
    int id;
    const char *name;
    bool enabled;
    int level;
#if (ULOG_TOPICS_NUM == -1) // Dynamic allocation part of the struct
    struct Topic_s *next;
#endif
} Topic;

// Prototypes for internal _impl functions
static Topic *_get_topic_ptr_impl(int topic_id);
static Topic *_get_topic_begin_impl(void);
static Topic *_get_topic_next_impl(Topic *t);
static int ulog_add_topic_impl(const char *topic_name, bool enable);
static int _ulog_get_topic_level_internal(int topic_id); // Forward declared for ulog_log


static bool new_topic_enabled = false; // Default for dynamically added topics

static void print_topic(const log_target *tgt, ulog_Event *ev) {
    // This function is called only if g_ulog_config.topics_num != 0
    Topic *t = _get_topic_ptr_impl(ev->topic);
    if (t && t->name) {
        print(tgt, "[%s] ", t->name);
    }
}

static int _ulog_set_topic_level_internal(int topic_id, int level) {
    Topic *t = _get_topic_ptr_impl(topic_id);
    if (t) {
        t->level = level;
        return 0;
    }
    return -1;
}

static int _ulog_get_topic_level_internal(int topic_id) {
    Topic *t = _get_topic_ptr_impl(topic_id);
    if (t) {
        return t->level;
    }
    return LOG_TRACE; // Default if topic not found
}

static int _ulog_enable_topic_internal(int topic_id) {
    Topic *t = _get_topic_ptr_impl(topic_id);
    if (t) {
        t->enabled = true;
        return 0;
    }
    return -1;
}

static int _ulog_disable_topic_internal(int topic_id) {
    Topic *t = _get_topic_ptr_impl(topic_id);
    if (t) {
        t->enabled = false;
        return 0;
    }
    return -1;
}

// --- Static Topics Implementation ---
#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM > 0
static Topic topics_static_array[ULOG_TOPICS_NUM];

static Topic *_get_topic_begin_impl(void) { return topics_static_array; }
static Topic *_get_topic_next_impl(Topic *t) {
    if (!t) return NULL;
    if ((t >= topics_static_array) && (t < topics_static_array + ULOG_TOPICS_NUM - 1)) {
         if (t + 1 < topics_static_array + ULOG_TOPICS_NUM) {
            return t + 1;
        }
    }
    return NULL;
}
static Topic *_get_topic_ptr_impl(int topic_id) {
    if (topic_id >= 0 && topic_id < ULOG_TOPICS_NUM) return &topics_static_array[topic_id];
    return NULL;
}
static int ulog_add_topic_impl(const char *topic_name, bool enable) {
    for (int i = 0; i < ULOG_TOPICS_NUM; i++) { 
        if (topics_static_array[i].name && strcmp(topics_static_array[i].name, topic_name) == 0) return i;
    }
    for (int i = 0; i < ULOG_TOPICS_NUM; i++) { 
        if (!topics_static_array[i].name) {
            topics_static_array[i].id = i;
            topics_static_array[i].name = topic_name; 
            topics_static_array[i].enabled = enable;
            topics_static_array[i].level = LOG_TRACE; 
            return i;
        }
    }
    return -1; 
}
#endif // Static Topics (ULOG_TOPICS_NUM > 0)

// --- Dynamic Topics Implementation ---
#if defined(ULOG_TOPICS_NUM) && (ULOG_TOPICS_NUM == -1)
#include <stdlib.h> 
static Topic *topics_dynamic_head = NULL;

static Topic *_get_topic_begin_impl(void) { return topics_dynamic_head; }
static Topic *_get_topic_next_impl(Topic *t) { return t ? t->next : NULL; }
static Topic *_get_topic_ptr_impl(int topic_id) {
    for (Topic *t = topics_dynamic_head; t != NULL; t = t->next) {
        if (t->id == topic_id) return t;
    }
    return NULL;
}
static Topic *_create_dynamic_topic(int id, const char *topic_name, bool enable) {
    Topic *new_topic = (Topic *)malloc(sizeof(Topic));
    if (new_topic) {
        new_topic->id = id;
        new_topic->name = topic_name; 
        new_topic->enabled = enable;
        new_topic->level = LOG_TRACE; 
        new_topic->next = NULL;
    }
    return new_topic;
}
static int ulog_add_topic_impl(const char *topic_name, bool enable) {
    if (!topic_name) return -1;
    Topic *current = topics_dynamic_head;
    Topic *last = NULL;
    int next_id = 0;
    while (current) {
        if (current->name && strcmp(current->name, topic_name) == 0) return current->id;
        if (current->id >= next_id) next_id = current->id + 1;
        last = current;
        current = current->next;
    }
    Topic *new_topic = _create_dynamic_topic(next_id, topic_name, enable);
    if (!new_topic) return -1; 
    if (last) last->next = new_topic;
    else topics_dynamic_head = new_topic;
    return next_id;
}
#endif // Dynamic Topics (ULOG_TOPICS_NUM == -1)

// --- Common Topic Interface Functions (using the _impl versions) ---
static bool is_topic_enabled(int topic_id) { 
    if (topic_id < 0) return true; 
    Topic *t = _get_topic_ptr_impl(topic_id);
    return t ? t->enabled : false; 
}

// Public API functions
int ulog_add_topic(const char *topic_name, bool enable) {
    if (g_ulog_config.topics_num == 0) return -1; 
#if ULOG_TOPICS_NUM > 0
    if (!g_ulog_config.topics_dynamic_alloc) return ulog_add_topic_impl(topic_name, enable);
#endif
#if ULOG_TOPICS_NUM == -1
    if (g_ulog_config.topics_dynamic_alloc) return ulog_add_topic_impl(topic_name, enable);
#endif
    return -1; 
}

int ulog_set_topic_level(const char *topic_name, int level) {
    if (g_ulog_config.topics_num == 0) return -1;
    int topic_id = ulog_get_topic_id(topic_name); 
    if (topic_id == TOPIC_NOT_FOUND) {
        topic_id = ulog_add_topic(topic_name, true); 
        if (topic_id == TOPIC_NOT_FOUND) return -1;
    }
    return _ulog_set_topic_level_internal(topic_id, level);
}

int ulog_enable_topic(const char *topic_name) {
    if (g_ulog_config.topics_num == 0) return -1;
    int topic_id = ulog_get_topic_id(topic_name);
    if (topic_id == TOPIC_NOT_FOUND) {
        topic_id = ulog_add_topic(topic_name, true); 
        if (topic_id == TOPIC_NOT_FOUND) return -1;
    }
    return _ulog_enable_topic_internal(topic_id);
}

int ulog_disable_topic(const char *topic_name) {
    if (g_ulog_config.topics_num == 0) return -1;
    int topic_id = ulog_get_topic_id(topic_name);
    if (topic_id == TOPIC_NOT_FOUND) { 
        topic_id = ulog_add_topic(topic_name, false); 
        if (topic_id == TOPIC_NOT_FOUND) return -1;
        return 0; 
    }
    return _ulog_disable_topic_internal(topic_id);
}

int ulog_enable_all_topics(void) {
    if (g_ulog_config.topics_num == 0) return -1;
    new_topic_enabled = true; 
    for (Topic *t = _get_topic_begin_impl(); t != NULL; t = _get_topic_next_impl(t)) {
        t->enabled = true;
    }
    return 0;
}

int ulog_disable_all_topics(void) {
    if (g_ulog_config.topics_num == 0) return -1;
    new_topic_enabled = false;
    for (Topic *t = _get_topic_begin_impl(); t != NULL; t = _get_topic_next_impl(t)) {
        t->enabled = false;
    }
    return 0;
}

int ulog_get_topic_id(const char *topic_name) { 
    if (g_ulog_config.topics_num == 0 || !topic_name) return TOPIC_NOT_FOUND;
    for (Topic *t = _get_topic_begin_impl(); t != NULL; t = _get_topic_next_impl(t)) {
        if (t->name && strcmp(t->name, topic_name) == 0) return t->id;
    }
    return TOPIC_NOT_FOUND;
}
#endif // ULOG_TOPICS_NUM != 0

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

// Level strings are selected at runtime based on g_ulog_config
static const char *level_strings_default[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
static const char *level_strings_short[]   = {"T", "D", "I", "W", "E", "F"};
static const char *level_strings_emoji[]   = {"⚪", "🔵", "🟢", "🟡", "🔴", "💥"};

static const char *get_current_level_string(int level) { 
    if (level < LOG_TRACE || level > LOG_FATAL) return "UNKNOWN"; 
    if (g_ulog_config.emoji_levels) return level_strings_emoji[level];
    if (g_ulog_config.short_level_strings) return level_strings_short[level];
    return level_strings_default[level];
}

// Restored print_level
static void print_level(const log_target *tgt, ulog_Event *ev) {
    print(tgt, "%s ", get_current_level_string(ev->level));
}

// Restored print_message
static void print_message(const log_target *tgt, ulog_Event *ev) {
    if (g_ulog_config.file_string_enabled) {
        print(tgt, "%s:%d: ", ev->file, ev->line);
    }
    if (ev->message) {
        vprint(tgt, ev->message, ev->message_format_args);
    } else {
        print(tgt, "NULL");
    }
}

/// @brief Writes the formatted message
static void write_formatted_message(const log_target *tgt, ulog_Event *ev,
                                    bool use_time_format, bool use_color, bool add_newline) {

#ifndef ULOG_NO_COLOR
    if (use_color && g_ulog_config.color_enabled) { 
        print_color_start(tgt, ev);
    }
#else
    (void)use_color; 
#endif

#ifdef ULOG_HAVE_TIME
    if (g_ulog_config.time_enabled) { 
        if (use_time_format == ULOG_TIME_FULL) { 
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0 
            print_time_full(tgt, ev);
#else
            print_time_sec(tgt, ev); 
#endif
        } else {
            print_time_sec(tgt, ev);
        }
    }
#else
    (void)use_time_format; 
#endif

#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    if (g_ulog_config.custom_prefix_size > 0) { 
        print_prefix(tgt, ev);
    }
#endif

#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0
    if (g_ulog_config.topics_num != 0 && ev->topic != -1) { 
        print_topic(tgt, ev);
    }
#endif

    print_level(tgt, ev); 
    print_message(tgt, ev); 

#ifndef ULOG_NO_COLOR
    if (use_color && g_ulog_config.color_enabled) { 
        print_color_end(tgt, ev);
    }
#endif

    if (add_newline) {
        print(tgt, "\n");
    }
}

/// @brief Callback for stdout
static void callback_stdout(ulog_Event *ev, void *arg) {
    log_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    write_formatted_message(&tgt, ev, ULOG_TIME_SHORT, g_ulog_config.color_enabled, ULOG_NEW_LINE_ON);
}

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    if (!out || out_size == 0) {
        return -1;
    }
    log_target tgt = {.type = T_BUFFER, .dsc.buffer = {out, out_size}};
    write_formatted_message(&tgt, ev, ULOG_TIME_SHORT, ULOG_COLOR_OFF, ULOG_NEW_LINE_OFF);
    return 0;
}

/// @brief Processes the stdout callback
static void log_to_stdout(ulog_Event *ev) {
    if (!g_ulog_config.quiet) { 
        if (!ulog.callback_stdout.function) {
            ulog.callback_stdout = (Callback){callback_stdout, stdout, LOG_TRACE};
        }
        process_callback(ev, &ulog.callback_stdout);
    }
}

/// @brief Logs the message
void ulog_log(int level, const char *file, int line, const char *topic_name,
              const char *message, ...) {

    if (!s_ulog_initialized) {
        ulog_init(NULL);
    }

    if (level < g_ulog_config.level) { 
        return;
    }

    int current_topic_id = -1; 

#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0
    if (g_ulog_config.topics_num != 0 && topic_name != NULL) { 
        current_topic_id = ulog_get_topic_id(topic_name);
        if (current_topic_id == TOPIC_NOT_FOUND) {
            if (g_ulog_config.topics_dynamic_alloc) { 
                current_topic_id = ulog_add_topic(topic_name, new_topic_enabled); 
                 if (current_topic_id == -1) return; 
            } else { 
                return; 
            }
        }
        if (!is_topic_enabled(current_topic_id) || level < _ulog_get_topic_level_internal(current_topic_id)) {
            return;
        }
    }
#else
    (void)topic_name; 
#endif

    ulog_Event ev = {
        .message = message,
        .file    = file,
        .line    = line,
        .level   = level,
    };

#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0
    ev.topic = current_topic_id; 
#endif
#ifdef ULOG_HAVE_TIME
    ev.time    = NULL; 
#endif

    va_start(ev.message_format_args, message);

    lock();

    log_to_stdout(&ev);

#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
    if (g_ulog_config.extra_outputs > 0) { 
        log_to_extra_outputs(&ev);
    }
#endif

    unlock();

    va_end(ev.message_format_args);
}


/// @brief Processes the callback with the event
static void process_callback(ulog_Event *ev, Callback *cb) {
    if (ev->level >= cb->level) { 
#ifdef ULOG_HAVE_TIME
        if (g_ulog_config.time_enabled && !ev->time) { 
            time_t t = time(NULL);
            ev->time = localtime(&t); 
        }
#endif
        cb->function(ev, cb->arg);
    }
}

//==================================================================
// Core Functionality: Logger configuration
//==================================================================

/// @brief Returns the string representation of the level
const char *ulog_get_level_string(int level) {
    return get_current_level_string(level);
}

/// @brief Sets the debug level
void ulog_set_level(int level) {
    if (!s_ulog_initialized) {
        ulog_init(NULL);
    }
    g_ulog_config.level = level; 
}

/// @brief Sets the quiet mode
void ulog_set_quiet(bool enable) {
    if (!s_ulog_initialized) {
        ulog_init(NULL);
    }
    g_ulog_config.quiet = enable; 
}
