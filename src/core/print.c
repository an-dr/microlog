// *************************************************************************
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#include "print.h"

void vprint(log_target *tgt, const char *format, va_list args) {
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

void print(log_target *tgt, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprint(tgt, format, args);
    va_end(args);
}

/// @brief Prints the topic
/// @param tgt - Target
/// @param ev - Event
static void print_level(log_target *tgt, ulog_Event *ev) {
    print(tgt, "%-1s ", ulog_get_level_string(ev->level));
}

/// @brief Prints the message
/// @param tgt - Target
/// @param ev - Event
static void print_message(log_target *tgt, ulog_Event *ev) {

#if FEATURE_FILE_STRING
    print(tgt, "%s:%d: ", ev->file, ev->line);  // file and line
#endif

    if (ev->message) {
        vprint(tgt, ev->message, ev->message_format_args);  // message
    } else {
        print(tgt, "NULL");  // message
    }
}

void print_formatted_message(log_target *tgt, ulog_Event *ev, bool full_time,
                             bool color, bool new_line) {

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

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    if (!out || out_size == 0) {
        return -1;
    }
    log_target tgt = {.type = T_BUFFER, .dsc.buffer = {out, 0, out_size}};
    print_formatted_message(&tgt, ev, false, false, false);
    return 0;
}
