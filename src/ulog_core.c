// ulog_core.c - Core logging logic
#include "ulog_core.h"
#include <string.h>

// --- Core logger object and configuration ---
typedef struct {
    ulog_LockFn lock_function;
    void *lock_arg;
    int level;
    bool quiet;
    // ...other fields are in feature modules...
} ulog_t;

static ulog_t ulog = {
    .lock_function = NULL,
    .lock_arg = NULL,
    .level = ULOG_DEFAULT_LOG_LEVEL,
    .quiet = false,
};

// --- Core API implementations ---
const char *ulog_get_level_string(int level) {
#if FEATURE_EMOJI_LEVELS
    static const char *level_strings[] = {"⚪", "🔵", "🟢", "🟡", "🔴", "💥"};
#elif FEATURE_SHORT_LEVELS
    static const char *level_strings[] = {"T", "D", "I", "W", "E", "F"};
#else
    static const char *level_strings[] = {"TRACE", "DEBUG", "INFO", "WARN", "ERROR", "FATAL"};
#endif
    return level_strings[level];
}

void ulog_set_level(int level) { ulog.level = level; }
void ulog_set_quiet(bool enable) { ulog.quiet = enable; }

int ulog_event_to_cstr(ulog_Event *ev, char *out, size_t out_size) {
    // Forward to output module implementation
    extern int ulog_event_to_cstr_impl(ulog_Event *, char *, size_t);
    return ulog_event_to_cstr_impl(ev, out, out_size);
}

void ulog_log(int level, const char *file, int line, const char *topic, const char *message, ...) {
    if (level < ulog.level) return;
    ulog_Event ev = { .message = message, .file = file, .line = line, .level = level };
#if FEATURE_TOPICS
    // ...topic handling is in topics module...
#endif
    va_start(ev.message_format_args, message);
    // ...lock, output, unlock, va_end are in output module...
}
