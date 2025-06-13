// ulog_core.c - Core logging logic
#include <string.h>
#include "print.h"
#include "ulog.h"
#include "ulog_object.h"

#define ULOG_NEW_LINE_ON true
#define ULOG_NEW_LINE_OFF false
#define ULOG_COLOR_ON true
#define ULOG_COLOR_OFF false
#define ULOG_TIME_FULL true
#define ULOG_TIME_SHORT false

/* ============================================================================
   Verbosity Levels
============================================================================ */

// clang-format off

/// @brief Level strings
static const char *level_strings[] = {
#if FEATURE_EMOJI_LEVELS
    "⚪",       "🔵",      "🟢",       "🟡",       "🔴",       "💥"
#elif FEATURE_SHORT_LEVELS
    "T",        "D",        "I",        "W",        "E",        "F"
#else
    "TRACE",    "DEBUG",    "INFO",     "WARN",     "ERROR",    "FATAL"
#endif
};

// clang-format on

/// @brief Returns the string representation of the level
const char *ulog_get_level_string(int level) {
    if (level < 0 ||
        level >= sizeof(level_strings) / sizeof(level_strings[0])) {
        return "?";  // Return a default string for invalid levels
    }
    return level_strings[level];
}

/// @brief Sets the debug level
void ulog_set_level(int level) {
    if (level < LOG_TRACE || level > LOG_FATAL) {
        return;  // Invalid level, do nothing
    }
    ulog.level = level;
}

/// @brief Sets the quiet mode
void ulog_set_quiet(bool enable) {
    ulog.quiet = enable;
}

/* ============================================================================
   Thread Safety
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
   ulog_log
============================================================================ */

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    if (!out || out_size == 0) {
        return -1;
    }
    log_target tgt = {.type = T_BUFFER, .dsc.buffer = {out, 0, out_size}};
    print_formatted_message(&tgt, ev, ULOG_TIME_SHORT, ULOG_COLOR_OFF,
                            ULOG_NEW_LINE_OFF);
    return 0;
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

/// @brief Callback for stdout
/// @param ev
static void callback_stdout(ulog_Event *ev, void *arg) {
    log_target tgt = {.type = T_STREAM, .dsc.stream = (FILE *)arg};
    print_formatted_message(&tgt, ev, ULOG_TIME_SHORT, ULOG_COLOR_ON,
                            ULOG_NEW_LINE_ON);
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
        if (!is_topic_enabled(topic_id) ||
            (level < _ulog_get_topic_level(topic_id))) {
            return;
        }
    }
#endif  // FEATURE_TOPICS

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
