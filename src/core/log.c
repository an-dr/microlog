// ulog_core.c - Core logging logic
#include <string.h>
#include "lock.h"
#include "object.h"
#include "print.h"
#include "ulog.h"

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
    print_formatted_message(&tgt, ev, false, true, true);
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
