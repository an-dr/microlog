// *************************************************************************
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// *************************************************************************

#include "object.h"
#include "ulog.h"

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
