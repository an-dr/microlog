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

/* ============================================================================
   Prototypes
============================================================================ */

static void print_level(log_target *tgt, ulog_Event *ev);
static void print_message(log_target *tgt, ulog_Event *ev);
static void process_callback(ulog_Event *ev, Callback *cb);
static void print_formatted_message(log_target *tgt, ulog_Event *ev,
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

static void print_color_start(log_target *tgt, ulog_Event *ev) {
    (void)ev;
    print(tgt, "%s", level_colors[ev->level]);  // color start
}

static void print_color_end(log_target *tgt, ulog_Event *ev) {
    (void)ev;
    print(tgt, "\x1b[0m");  // color end
}

#endif  // FEATURE_COLOR

/* ============================================================================
   Feature: Time
============================================================================ */
#if FEATURE_TIME

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

#endif  // FEATURE_TIME

/* ============================================================================
   Feature: Custom Prefix
============================================================================ */
#if FEATURE_CUSTOM_PREFIX

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    ulog.update_prefix_function = function;
}

static void print_prefix(log_target *tgt, ulog_Event *ev) {
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
    print_formatted_message(&tgt, ev, ULOG_TIME_FULL, ULOG_COLOR_OFF,
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

static void print_topic(log_target *tgt, ulog_Event *ev) {
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
    if (ulog_add_topic(topic_name, true) != -1) {
        return _ulog_set_topic_level(ulog_get_topic_id(topic_name), level);
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
            topics[i].level   = ULOG_DEFAULT_LOG_LEVEL;
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
