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

#ifndef ULOG_EXTRA_DESTINATIONS
#define ULOG_EXTRA_DESTINATIONS 0
#endif

#ifndef ULOG_CUSTOM_PREFIX_SIZE
#define ULOG_CUSTOM_PREFIX_SIZE 0
#endif

/// @brief Callback structure
typedef struct {
    ulog_LogFn function;  // Callback function
    void *arg;            // Any argument that will be passed to the event
    int level;            // Debug level
} Callback;

//==================================================================
// Main logger object
//==================================================================

typedef struct {
    ulog_LockFn lock_function;          // Mutex function
    void *lock_arg;                     // Mutex argument
    int level;                          // Debug level
    bool quiet;                         // Quiet mode

#ifndef ULOG_NO_STDOUT
    Callback callback_stdout;  // to stdout
#endif

#if ULOG_EXTRA_DESTINATIONS > 0
    Callback callbacks[ULOG_EXTRA_DESTINATIONS];  // Extra callbacks
#endif                                            // ULOG_EXTRA_DESTINATIONS

#if ULOG_CUSTOM_PREFIX_SIZE > 0
    ulog_PrefixFn update_prefix_function;  // Custom prefix function
    char custom_prefix[ULOG_CUSTOM_PREFIX_SIZE];  // Custom prefix
#endif

} ulog_t;

static ulog_t ulog = {
        .lock_function          = NULL,
        .lock_arg               = NULL,
        .level                  = LOG_TRACE,
        .quiet                  = false,

#ifndef ULOG_NO_STDOUT
        .callback_stdout = {0},
#endif  // ULOG_NO_STDOUT

#if ULOG_EXTRA_DESTINATIONS > 0
        .callbacks = {{0}},
#endif  // ULOG_EXTRA_DESTINATIONS

#if ULOG_CUSTOM_PREFIX_SIZE > 0
        .update_prefix_function = NULL,
        .custom_prefix = {0},
#endif // ULOG_CUSTOM_PREFIX_SIZE

};

//==================================================================
// Internal callbacks for stdout and file
//==================================================================

/// @brief Level strings
static const char *level_strings[] = {
#ifdef ULOG_USE_EMOJI
        "⚪", "🔵", "🟢", "🟡", "🔴", "💥"
#else
    
#ifdef ULOG_SHORT_LEVEL_STRINGS
        "[T]", "[D]", "[I]", "[W]", "[E]", "[F]"
#else
        "[TRACE]", "[DEBUG]", "[INFO]", "[WARN]", "[ERROR]", "[FATAL]"
#endif

#endif // ULOG_USE_EMOJI
};

#ifndef ULOG_NO_COLOR
/// @brief Level colors
static const char *level_colors[] = {
        "\x1b[37m",  // TRACE : White #000
        "\x1b[36m",  // DEBUG : Cyan #0ff
        "\x1b[32m",  // INFO : Green #0f0
        "\x1b[33m",  // WARN : Yellow #ff0
        "\x1b[31m",  // ERROR : Red #f00
        "\x1b[35m"   // FATAL : Magenta #f0f
};
#endif

static void print_time_sec(ulog_Event *ev, FILE *file) {
#ifdef ULOG_HAVE_TIME
    char buf[9];
    buf[strftime(buf, sizeof(buf), "%H:%M:%S", ev->time)] = '\0';
    fprintf(file, "%s ", buf);
#else
    (void) ev;
    (void) file;
#endif
}


static void print_prefix(ulog_Event *ev, FILE *file) {
#if ULOG_CUSTOM_PREFIX_SIZE > 0
    if (ulog.update_prefix_function) {
        ulog.update_prefix_function(ev, ulog.custom_prefix, ULOG_CUSTOM_PREFIX_SIZE);
        fprintf(file, "%s ", ulog.custom_prefix);
    }
#else
    (void) ev;
    (void) file;
#endif
}

static void print_message(ulog_Event *ev, FILE *file) {

    fprintf(file, "%-1s ", level_strings[ev->level]);

#ifndef ULOG_HIDE_FILE_STRING
    fprintf(file, "%s:%d: ", ev->file, ev->line);  // file and line
#endif                                             // ULOG_HIDE_FILE_STRING

    vfprintf(file, ev->message, ev->message_format_args);  // message
}

static void print_newline_and_flush(ulog_Event *ev, FILE *file) {
    (void) ev;
    fprintf(file, "\n");
    fflush(file);
}

static void print_color_start(ulog_Event *ev, FILE *file) {
    (void) ev;
#ifndef ULOG_NO_COLOR
    fprintf(file, "%s", level_colors[ev->level]);  // color start
#else
    (void) file;
#endif
}

static void print_color_end(ulog_Event *ev, FILE *file) {
    (void) ev;
#ifndef ULOG_NO_COLOR
    fprintf(file, "\x1b[0m");  // color end
#else
    (void) file;
#endif

}

/// @brief Callback for stdout
/// @param ev
static void callback_stdout(ulog_Event *ev, void *arg) {
#ifndef ULOG_NO_STDOUT
    FILE *fp = (FILE *) arg;
    print_color_start(ev, fp);
    print_time_sec(ev, fp);
    print_prefix(ev, fp);
    print_message(ev, fp);
    print_color_end(ev, fp);

    print_newline_and_flush(ev, fp);
#else   // ULOG_NO_STDOUT
    (void) ev;
    (void) arg;
#endif  // ULOG_NO_STDOUT
}



//==================================================================
// Locks
//==================================================================

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

//==================================================================
// Logger configuration
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

#if ULOG_CUSTOM_PREFIX_SIZE > 0
void ulog_set_prefix_fn(ulog_PrefixFn function) {
    ulog.update_prefix_function = function;
}
#endif // ULOG_CUSTOM_PREFIX_SIZE


#if ULOG_EXTRA_DESTINATIONS > 0

static void print_time_full(ulog_Event *ev, FILE *file) {
#ifdef ULOG_HAVE_TIME
    char buf[64];
    buf[strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", ev->time)] = '\0';
    fprintf(file, "%s ", buf);
#else
    (void) ev;
    (void) file;
#endif
}

/// @brief Callback for file
static void callback_file(ulog_Event *ev, void *arg) {
    FILE *fp = (FILE *) arg;
    print_time_full(ev, fp);
    print_prefix(ev, fp);
    print_message(ev, fp);
    print_newline_and_flush(ev, fp);
}

/// @brief Adds a callback
int ulog_add_callback(ulog_LogFn function, void *arg, int level) {
    for (int i = 0; i < ULOG_EXTRA_DESTINATIONS; i++) {
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
#endif


//==================================================================
// Main logging code
//==================================================================

/// @brief Processes the callback with the event
/// @param ev - Event
/// @param cb - Callback
static void process_callback(ulog_Event *ev, Callback *cb) {
    if (ev->level >= cb->level) {
#ifdef ULOG_HAVE_TIME
        if (!ev->time) {
            time_t t = time(NULL);
            ev->time = localtime(&t);
        }
#endif  // ULOG_HAVE_TIME
        cb->function(ev, cb->arg);
    }
}


/// @brief Processes the stdout callback
/// @param ev - Event
static void log_to_stdout(ulog_Event *ev) {
#ifndef ULOG_NO_STDOUT
    if (!ulog.quiet) {
        // Initializing the stdout callback if not set
        if (!ulog.callback_stdout.function) {
            ulog.callback_stdout = (Callback){callback_stdout,
                                              stdout,  // we use udata to pass the file pointer
                                              LOG_TRACE};
        }
        process_callback(ev, &ulog.callback_stdout);
    }
#else   // ULOG_NO_STDOUT
    (void) ev;
#endif  // ULOG_NO_STDOUT
}


/// @brief Processes the extra callbacks
/// @param ev - Event
static void log_to_extra_destinations(ulog_Event *ev) {
#if ULOG_EXTRA_DESTINATIONS > 0
    // Processing the message for callbacks
    for (int i = 0;
         i < ULOG_EXTRA_DESTINATIONS && ulog.callbacks[i].function;
         i++) {
        process_callback(ev, &ulog.callbacks[i]);
    }
#else   // ULOG_EXTRA_DESTINATIONS
    (void) ev;
#endif  // ULOG_EXTRA_DESTINATIONS
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
    log_to_extra_destinations(&ev);
    unlock();

    va_end(ev.message_format_args);
}
