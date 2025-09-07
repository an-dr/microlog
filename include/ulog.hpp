// *************************************************************************
//
// ulog v@{ULOG_VERSION}@  - A simple customizable logging library.
// https://github.com/an-dr/microlog
//
// *************************************************************************
//
// Original implementation by rxi: https://github.com/rxi
// Modified by Andrei Gramakov: https://agramakov.me, mail@agramakov.me
// Also modified by many beautiful contributors from GitHub
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// This file is licensed under the terms of the MIT license.
// For a copy, see: https://opensource.org/licenses/MIT
//
// *************************************************************************

#include <source_location>
#include <string>
#include <string_view>
#include "ulog.h"

namespace ulog {

enum class Level {
    trace = ULOG_LEVEL_TRACE,
    debug = ULOG_LEVEL_DEBUG,
    info  = ULOG_LEVEL_INFO,
    warn  = ULOG_LEVEL_WARN,
    error = ULOG_LEVEL_ERROR,
    fatal = ULOG_LEVEL_FATAL,
};

/// @brief Status codes for ulog operations (C++ wrapper)
enum class Status {
    ok               = ULOG_STATUS_OK,
    error            = ULOG_STATUS_ERROR,
    invalid_argument = ULOG_STATUS_INVALID_ARGUMENT,
    // not_found        = ULOG_STATUS_NOT_FOUND,
};

/// @brief Instance-based logger class with full topic management
class Logger {
  public:
    /// @brief Create a logger with optional topic and level
    Logger(const std::string &topic = "", Level level = Level::trace)
        : topic_(topic), level_(static_cast<ulog_level>(level)) {
        if (!topic_.empty()) {
            // Register topic and set its level
            ulog_topic_add(topic_.c_str(), ULOG_OUTPUT_ALL, true);
            ulog_topic_level_set(topic_.c_str(), level_);
        }
    }

    ~Logger() = default;

    template <class... Args>
    void info(std::string_view fmt, Args&&... args) {
        const auto& loc = std::source_location::current();
        ulog_log(static_cast<ulog_level>(Level::info),
                 loc.file_name(),
                 loc.line(),
                 topic_.c_str(),
                 fmt.data(),
                 std::forward<Args>(args)...);
    }

    std::string topic_;
    ulog_level level_;
};

}  // namespace ulog
