// *************************************************************************
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#pragma once

#include "ulog.h"

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
void print_formatted_message(log_target *tgt, ulog_Event *ev, bool full_time,
                             bool color, bool new_line);

void vprint(log_target *tgt, const char *format, va_list args);
void print(log_target *tgt, const char *format, ...);

