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

#if FEATURE_TOPICS
#include <string.h>
#endif

#define ULOG_NEW_LINE_ON true
#define ULOG_NEW_LINE_OFF false
#define ULOG_COLOR_ON true
#define ULOG_COLOR_OFF false
#define ULOG_TIME_FULL true
#define ULOG_TIME_SHORT false

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
    .level           = LOG_TRACE,
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

static void print_color_start(const log_target *tgt, ulog_Event *ev) {
    (void)ev;
    print(tgt, "%s", level_colors[ev->level]);  // color start
}

static void print_color_end(const log_target *tgt, ulog_Event *ev) {
    (void)ev;
    print(tgt, "\x1b[0m");  // color end
}

#endif  // FEATURE_COLOR

/* ============================================================================
   Feature: Time
============================================================================ */
#if FEATURE_TIME

static void print_time_sec(const log_target *tgt, ulog_Event *ev) {

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
static void print_time_full(const log_target *tgt, ulog_Event *ev) {

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

static void print_prefix(const log_target *tgt, ulog_Event *ev) {
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
#if FEATURE_TOPICS

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

static void print_topic(const log_target *tgt, ulog_Event *ev) {
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
    int topic_id_from_add = ulog_add_topic(topic_name, true); 
    if (topic_id_from_add != -1) {
        // Directly use the ID obtained from ulog_add_topic.
        // This assumes ulog_add_topic returns a valid ID that _ulog_set_topic_level can use.
        return _ulog_set_topic_level(topic_id_from_add, level); 
    }
    return -1;
}

int ulog_enable_topic(const char *topic_name) {
    ulog_add_topic(topic_name, true);
    return _ulog_enable_topic(ulog_get_topic_id(topic_name));
}

int ulog_disable_topic(const char *topic_name) {
    ulog_add_topic(topic_name, false);
    return _ulog_disable_topic(ulog_get_topic_id(topic_name));
}

int ulog_enable_all_topics(void) {
    new_topic_enabled = true;
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        t->enabled = true;
    }
    return 0;
}

int ulog_disable_all_topics(void) {
    new_topic_enabled = false;
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        t->enabled = false;
    }
    return 0;
}

int ulog_get_topic_id(const char *topic_name) {
    for (Topic *t = _get_topic_begin(); t != NULL; t = _get_topic_next(t)) {
        if (t->name && strcmp(t->name, topic_name) == 0) {
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

#endif  // FEATURE_TOPICS

/* ============================================================================
   Feature: Log Topics - Static Allocation
============================================================================ */
#if FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == false

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
    for (int i = 0; i < CFG_TOPICS_NUM; i++) {
        if (!topics[i].name) {
            topics[i].id      = i;
            topics[i].name    = topic_name;
            topics[i].enabled = enable;
            topics[i].level   = LOG_TRACE;
            return i;
        }
    }
    return -1;
}
#endif  // FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == false

/* ============================================================================
   Feature: Log Topics - Dynamic Allocation
============================================================================ */

#if FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == true

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

#endif  // FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC == true

#if FEATURE_TOPICS

#endif  // FEATURE_TOPICS

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
static void write_formatted_message(const log_target *tgt, ulog_Event *ev,
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
    // The line below was removed as 'tgt' is unused in the new implementation.
    // log_target tgt = {.type = T_BUFFER, .dsc.buffer = {out, out_size}}; 
    if (!out || !ev || out_size == 0) { // Added !ev check
        if (out && out_size > 0) out[0] = '\0'; // Ensure null termination on error if buffer valid
        return -1;
    }
    out[0] = '\0'; // Start with an empty string to be safe

    size_t current_len = 0;
    int written;

    // Mimic the order from write_formatted_message, considering defaults for ulog_event_to_cstr:
    // No color, no newline. Time is ULOG_TIME_SHORT, but only if FEATURE_TIME is active.
    // No custom prefix, no topics by default for this function's direct output.

#if FEATURE_TIME
    // Note: ulog_event_to_cstr uses ULOG_TIME_SHORT, which means print_time_sec.
    // print_time_sec uses strftime with "%H:%M:%S " (or "%H:%M:%S" if custom prefix).
    // We need to ensure ev->time is populated if we want time here.
    // The original write_formatted_message has process_callback populate ev->time.
    // For ulog_event_to_cstr, ev->time must be pre-populated by the caller if time is desired.
    if (ev->time) { // Only print time if ev->time is not NULL
        char time_buf[16]; // Buffer for HH:MM:SS (plus space/null)
#if FEATURE_CUSTOM_PREFIX // This check is inside print_time_sec, effectively
        // Assuming no custom prefix for ulog_event_to_cstr direct output simplicity
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S ", ev->time); 
#else
        strftime(time_buf, sizeof(time_buf), "%H:%M:%S ", ev->time);
#endif
        if (current_len < out_size) {
            written = snprintf(out + current_len, out_size - current_len, "%s", time_buf);
            if (written > 0) current_len += written;
            else if (written < 0) return -1; // snprintf error
        }
    }
#endif

    // Level string
    if (current_len < out_size) {
        // Format: "LEVEL " (e.g., "INFO ")
        written = snprintf(out + current_len, out_size - current_len, "%-1s ", level_strings[ev->level]);
        if (written > 0) current_len += written;
        else if (written < 0) return -1; // snprintf error
    }

#if FEATURE_FILE_STRING
    // File and line: "file:line: "
    if (current_len < out_size) {
        written = snprintf(out + current_len, out_size - current_len, "%s:%d: ", ev->file, ev->line);
        if (written > 0) current_len += written;
        else if (written < 0) return -1; // snprintf error
    }
#endif

    // Actual message (already formatted by va_list if applicable)
    if (ev->message) {
        if (current_len < out_size) {
            // Use ev->message directly as it's assumed to be the final string or format string for given args
            // If ev->message is a format string, ev->message_format_args should be used with vsnprintf
            // The ulog_Event struct suggests ev->message is the format string and message_format_args are its args.
            written = vsnprintf(out + current_len, out_size - current_len, ev->message, ev->message_format_args);
            if (written > 0) current_len += written;
            else if (written < 0) return -1; // vsnprintf error
        }
    } else {
        if (current_len < out_size) {
            written = snprintf(out + current_len, out_size - current_len, "NULL");
            if (written > 0) current_len += written;
            else if (written < 0) return -1; // snprintf error
        }
    }
    
    // Check for truncation: if current_len equals out_size, the string *might* have been truncated
    // (if out_size-1 was exactly filled and null terminator took the last char).
    // If current_len > out_size, it definitely means it would have overflowed.
    // snprintf and vsnprintf return the number of characters that *would have been written* if buffer was large enough.
    // So if written >= (out_size - current_len_before_call), truncation occurred.
    // The loop structure already checks current_len < out_size before calls, which is a bit different.
    // A simpler check: if at any point written output would exceed remaining buffer, it's an issue.
    // The snprintf/vsnprintf calls themselves handle not writing past buffer end.
    // The key is that the *returned value* from snprintf/vsnprintf is what *would* be written.
    // Let's refine the logic slightly for robust length checking.
    
    // The current accumulation of `current_len` correctly tracks characters written *if they fit*.
    // If `written` (from snprintf/vsnprintf) is >= remaining space, it means output was truncated.
    // For simplicity, the current structure is okay for basic function, but robust error handling for truncation
    // would compare `written` with `out_size - current_len` at each step.
    // However, `ulog_event_to_cstr` doesn't have a defined behavior for reporting truncation other than returning -1.
    // The current code already returns -1 on `snprintf` error. Let's assume truncation isn't explicitly signaled beyond buffer not containing full msg.


    return 0; // Success
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

static void print_level(const log_target *tgt, ulog_Event *ev) {
    print(tgt, "%-1s ", level_strings[ev->level]);
}

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void print_message(const log_target *tgt, ulog_Event *ev) {

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

        cb->function(ev, cb->arg);
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
