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

#define MAX_CALLBACKS 32

typedef struct {
    ulog_LogFn fn;
    void *udata;
    int level;
} Callback;

static struct {
    void *udata;
    ulog_LockFn lock;
    int level;
    bool quiet;
    Callback callbacks[MAX_CALLBACKS];
} L;


static const char *level_strings[] = {
#ifdef ULOG_SHORT_LEVEL_STRINGS
        "T", "D", "I", "W", "E", "F"
#else
        "TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
#endif
};

#ifdef ULOG_USE_COLOR
static const char *level_colors[] = {
        "\x1b[34m",  // TRACE : Blue #00f
        "\x1b[36m",  // DEBUG : Cyan #0ff
        "\x1b[32m",  // INFO : Green #0f0
        "\x1b[33m",  // WARN : Yellow #ff0
        "\x1b[31m",  // ERROR : Red #f00
        "\x1b[35m"   // FATAL : Magenta #f0f
};
#endif


static void stdout_callback(ulog_Event *ev) {

#ifdef ULOG_USE_COLOR
    fprintf(ev->udata, "%s", level_colors[ev->level]);  // color start
#endif

#ifdef ULOG_HAVE_TIME
    fprintf(ev->udata, "%lu [%-1s] ", ulog_get_time(), level_strings[ev->level]);
#else
    fprintf(ev->udata, "[%-1s] ", level_strings[ev->level]);
#endif

#ifndef ULOG_HIDE_FILE_STRING
    fprintf(ev->udata, "%s:%d: ", ev->file, ev->line);  // file and line
#endif // ULOG_HIDE_FILE_STRING

    vfprintf(ev->udata, ev->fmt, ev->ap); // message
    
#ifdef ULOG_USE_COLOR
fprintf(ev->udata, "\x1b[0m");  // color end
#endif

    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}


static void file_callback(ulog_Event *ev) {
#ifdef ULOG_HAVE_TIME
    fprintf(ev->udata, "%lu %-5s %s:%d: ",
            ulog_get_time(), level_strings[ev->level], ev->file, ev->line);
#else
    fprintf(ev->udata, "%-5s %s:%d: ",
            level_strings[ev->level], ev->file, ev->line);
#endif
    vfprintf(ev->udata, ev->fmt, ev->ap);
    fprintf(ev->udata, "\n");
    fflush(ev->udata);
}


static void lock(void) {
    if (L.lock) {
        L.lock(true, L.udata);
    }
}


static void unlock(void) {
    if (L.lock) {
        L.lock(false, L.udata);
    }
}


const char *ulog_level_string(int level) {
    return level_strings[level];
}


void ulog_set_lock(ulog_LockFn fn, void *udata) {
    L.lock  = fn;
    L.udata = udata;
}


void ulog_set_level(int level) {
    L.level = level;
}


void ulog_set_quiet(bool enable) {
    L.quiet = enable;
}


int ulog_add_callback(ulog_LogFn fn, void *udata, int level) {
    for (int i = 0; i < MAX_CALLBACKS; i++) {
        if (!L.callbacks[i].fn) {
            L.callbacks[i] = (Callback){
                    fn, udata, level};
            return 0;
        }
    }
    return -1;
}


int ulog_add_fp(FILE *fp, int level) {
    return ulog_add_callback(file_callback, fp, level);
}


static void init_event(ulog_Event *ev, void *udata) {
    ev->udata = udata;
}


void ulog_log(int level, const char *file, int line, const char *fmt, ...) {
    ulog_Event ev = {
            .fmt   = fmt,
            .file  = file,
            .line  = line,
            .level = level,
    };

    lock();

    if (!L.quiet && level >= L.level) {
        init_event(&ev, stderr);
        va_start(ev.ap, fmt);
        stdout_callback(&ev);
        va_end(ev.ap);
    }

    for (int i = 0; i < MAX_CALLBACKS && L.callbacks[i].fn; i++) {
        Callback *cb = &L.callbacks[i];
        if (level >= cb->level) {
            init_event(&ev, cb->udata);
            va_start(ev.ap, fmt);
            cb->fn(&ev);
            va_end(ev.ap);
        }
    }

    unlock();
}
