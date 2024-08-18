/*
 * Copyright (c) 2020 rxi
 * Additions Copyright (c) 2024 Andrei Gramakov - mail@agramakov.me
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "ulog.h"


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

#if FEATURE_EXTRA_DESTS
    Callback callbacks[CFG_EXTRA_DESTS];  // Extra callbacks
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


#if FEATURE_EXTRA_DESTS
        .callbacks = {{0}},
#endif


#if FEATURE_CUSTOM_PREFIX
        .update_prefix_function = NULL,
        .custom_prefix          = {0},
#endif

};

/* ============================================================================
   Prototypes
============================================================================ */

static void print_message(ulog_Event *ev, FILE *file);
static void print_newline_and_flush(ulog_Event *ev, FILE *file);
static void process_callback(ulog_Event *ev, Callback *cb);

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

static void print_color_start(ulog_Event *ev, FILE *file) {
    (void) ev;
    fprintf(file, "%s", level_colors[ev->level]);  // color start
}

static void print_color_end(ulog_Event *ev, FILE *file) {
    (void) ev;
    fprintf(file, "\x1b[0m");  // color end
}

#endif  // FEATURE_COLOR

/* ============================================================================
   Feature: Time
============================================================================ */
#if FEATURE_TIME

static void print_time_sec(ulog_Event *ev, FILE *file) {
    char buf[9];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
    fprintf(file, "%s ", buf);
}

static void print_time_full(ulog_Event *ev, FILE *file) {
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf(file, "%s ", buf);
}

#endif  // FEATURE_TIME

/* ============================================================================
   Feature: Custom Prefix
============================================================================ */
#if FEATURE_CUSTOM_PREFIX

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    ulog.update_prefix_function = function;
}

static void print_prefix(ulog_Event *ev, FILE *file) {
    if (ulog.update_prefix_function) {
        ulog.update_prefix_function(ev, ulog.custom_prefix, CFG_CUSTOM_PREFIX_SIZE);
        fprintf(file, "%s ", ulog.custom_prefix);
    }
}

#endif  // FEATURE_CUSTOM_PREFIX

/* ============================================================================
   Feature: Extra Destinations
============================================================================ */
#if FEATURE_EXTRA_DESTS

/// @brief Callback for file
/// @param ev - Event
/// @param arg - File pointer
static void callback_file(ulog_Event *ev, void *arg) {
    FILE *fp = (FILE *) arg;

#if FEATURE_TIME
    print_time_full(ev, fp);
#endif

#if FEATURE_CUSTOM_PREFIX
    print_prefix(ev, fp);
#endif

    print_message(ev, fp);
    print_newline_and_flush(ev, fp);
}

/// @brief Adds a callback
/// @param function - Callback function
/// @param arg - Optional argument that will be added to the event to be
///              processed by the callback
/// @param level - Debug level
int ulog_add_callback(ulog_LogFn function, void *arg, int level) {
    for (int i = 0; i < CFG_EXTRA_DESTS; i++) {
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
static void log_to_extra_destinations(ulog_Event *ev) {
    // Processing the message for callbacks
    for (int i = 0; i < CFG_EXTRA_DESTS && ulog.callbacks[i].function;
         i++) {
        process_callback(ev, &ulog.callbacks[i]);
    }
}

#endif  // FEATURE_EXTRA_DESTS

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
static const char *level_strings[] = {
#if FEATURE_EMOJI_LEVELS
        "⚪", "🔵", "🟢", "🟡", "🔴", "💥"
#elif FEATURE_SHORT_LEVELS
        "[T]", "[D]", "[I]", "[W]", "[E]", "[F]"
#else
        "[TRACE]", "[DEBUG]", "[INFO]", "[WARN]", "[ERROR]", "[FATAL]"
#endif
};

/// @brief Callback for stdout
/// @param ev
static void callback_stdout(ulog_Event *ev, void *arg) {
    FILE *fp = (FILE *) arg;

#if FEATURE_COLOR
    print_color_start(ev, fp);
#endif

#if FEATURE_TIME
    print_time_sec(ev, fp);
#endif

#if FEATURE_CUSTOM_PREFIX
    print_prefix(ev, fp);
#endif

    print_message(ev, fp);

#if FEATURE_COLOR
    print_color_end(ev, fp);
#endif

    print_newline_and_flush(ev, fp);
}

/// @brief Processes the stdout callback
/// @param ev - Event
static void log_to_stdout(ulog_Event *ev) {
    if (!ulog.quiet) {
        // Initializing the stdout callback if not set
        if (!ulog.callback_stdout.function) {
            ulog.callback_stdout = (Callback){callback_stdout,
                                              stdout,  // we use udata to pass the file pointer
                                              LOG_TRACE};
        }
        process_callback(ev, &ulog.callback_stdout);
    }
}

/// @brief Logs the message
void ulog_log(int level, const char *file, int line, const char *message, ...) {
    ulog_Event ev = {
            .message = message,
            .file    = file,
            .line    = line,
            .level   = level,
    };

    va_start(ev.message_format_args, message);

    lock();

    log_to_stdout(&ev);

#if FEATURE_EXTRA_DESTS
    log_to_extra_destinations(&ev);
#endif

    unlock();

    va_end(ev.message_format_args);
}


/// @brief Prints the message
/// @param ev
/// @param file
static void print_message(ulog_Event *ev, FILE *file) {

    fprintf(file, "%-1s ", level_strings[ev->level]);

#if FEATURE_FILE_STRING
    fprintf(file, "%s:%d: ", ev->file, ev->line);  // file and line
#endif

    vfprintf(file, ev->message, ev->message_format_args);  // message
}


/// @brief Prints the newline and flushes the file
/// @param ev
/// @param file
static void print_newline_and_flush(ulog_Event *ev, FILE *file) {
    (void) ev;
    fprintf(file, "\n");
    fflush(file);
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
