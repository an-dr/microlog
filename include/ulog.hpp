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

enum class Level : ulog_level {
    trace = ULOG_LEVEL_TRACE,
    debug = ULOG_LEVEL_DEBUG,
    info  = ULOG_LEVEL_INFO,
    warn  = ULOG_LEVEL_WARN,
    error = ULOG_LEVEL_ERROR,
    fatal = ULOG_LEVEL_FATAL,
};

/// @brief Status codes for ulog operations (C++ wrapper)
enum class Status : ulog_status {
    ok               = ULOG_STATUS_OK,
    error            = ULOG_STATUS_ERROR,
    invalid_argument = ULOG_STATUS_INVALID_ARGUMENT,
};

/// @brief Static class for interfacing with ulog C library
class Log {
  public:
    // Core logging functions with automatic source location
    template <typename... Args>
    static void
    trace(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_TRACE, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    debug(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_DEBUG, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    info(const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_INFO, loc.file_name(), static_cast<int>(loc.line()),
                 nullptr, format.c_str(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    warn(const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_WARN, loc.file_name(), static_cast<int>(loc.line()),
                 nullptr, format.c_str(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    error(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_ERROR, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    fatal(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_FATAL, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    // Topic-based logging functions
    template <typename... Args>
    static void
    trace(const std::string &topic, const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_TRACE, loc.file_name(),
                 static_cast<int>(loc.line()), topic.c_str(), format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    debug(const std::string &topic, const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_DEBUG, loc.file_name(),
                 static_cast<int>(loc.line()), topic.c_str(), format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    info(const std::string &topic, const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_INFO, loc.file_name(), static_cast<int>(loc.line()),
                 topic.c_str(), format.c_str(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    warn(const std::string &topic, const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_WARN, loc.file_name(), static_cast<int>(loc.line()),
                 topic.c_str(), format.c_str(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    error(const std::string &topic, const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_ERROR, loc.file_name(),
                 static_cast<int>(loc.line()), topic.c_str(), format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    fatal(const std::string &topic, const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_LEVEL_FATAL, loc.file_name(),
                 static_cast<int>(loc.line()), topic.c_str(), format.c_str(),
                 std::forward<Args>(args)...);
    }

    // Configuration functions
    static void set_level(Level level) {
        ulog_output_level_set_all(static_cast<ulog_level>(level));
    }

    static void enable_colors(bool enabled = true) {
        ulog_color_config(enabled);
    }

    static void enable_timestamps(bool enabled = true) {
        ulog_time_config(enabled);
    }

    static void enable_source_location(bool enabled = true) {
        ulog_source_location_config(enabled);
    }

    static void enable_prefix(bool enabled = true) {
        ulog_prefix_config(enabled);
    }

    static void enable_topics(bool enabled = true) {
        ulog_topic_config(enabled);
    }

    // Topic management
    static int add_topic(const std::string &topic_name, bool enable = true) {
        return ulog_topic_add(topic_name.c_str(), enable);
    }

    static Status set_topic_level(const std::string &topic_name, Level level) {
        return static_cast<Status>(ulog_topic_level_set(
            topic_name.c_str(), static_cast<ulog_level>(level)));
    }

    static Status enable_topic(const std::string &topic_name) {
        return static_cast<Status>(ulog_topic_enable(topic_name.c_str()));
    }

    static Status disable_topic(const std::string &topic_name) {
        return static_cast<Status>(ulog_topic_disable(topic_name.c_str()));
    }

    static Status enable_all_topics() {
        return static_cast<Status>(ulog_topic_enable_all());
    }

    static Status disable_all_topics() {
        return static_cast<Status>(ulog_topic_disable_all());
    }

    // Output management
    static int add_file_output(FILE *file, Level level) {
        return ulog_output_add_file(file, static_cast<ulog_level>(level));
    }

    static Status remove_output(int output_handle) {
        return static_cast<Status>(ulog_output_remove(output_handle));
    }

    static Status set_output_level(int output_handle, Level level) {
        return static_cast<Status>(ulog_output_level_set(
            output_handle, static_cast<ulog_level>(level)));
    }

    // Thread safety
    static void set_lock_function(ulog_lock_fn function,
                                  void *lock_arg = nullptr) {
        ulog_lock_set_fn(function, lock_arg);
    }

    // Prefix callback
    static void set_prefix_function(ulog_prefix_fn function) {
        ulog_prefix_set_fn(function);
    }

  private:
    Log()                       = delete;  // Prevent instantiation
    ~Log()                      = delete;
    Log(const Log &)            = delete;
    Log &operator=(const Log &) = delete;
};

/// @brief Instance-based logger class for scoped logging with topics
class Logger {
  public:
    Logger(const std::string &topic = "", Level level = Level::trace)
        : topic_(topic), level_(static_cast<ulog_level>(level)) {
        if (!topic_.empty()) {
            // Ensure topic is registered if not empty
            ulog_topic_add(topic_.c_str(), true);
            ulog_topic_level_set(topic_.c_str(), level_);
        }
    }

    ~Logger() = default;

    // Logging functions with automatic source location
    template <typename... Args>
    void
    trace(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) const {
        log(ULOG_LEVEL_TRACE, format, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void
    debug(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) const {
        log(ULOG_LEVEL_DEBUG, format, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void
    info(const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) const {
        log(ULOG_LEVEL_INFO, format, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void
    warn(const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) const {
        log(ULOG_LEVEL_WARN, format, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void
    error(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) const {
        log(ULOG_LEVEL_ERROR, format, loc, std::forward<Args>(args)...);
    }

    template <typename... Args>
    void
    fatal(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) const {
        log(ULOG_LEVEL_FATAL, format, loc, std::forward<Args>(args)...);
    }

    // Set minimum log level for this logger instance
    void set_level(Level level) {
        level_ = static_cast<ulog_level>(level);
        if (!topic_.empty()) {
            ulog_topic_level_set(topic_.c_str(), level_);
        }
    }

    Level get_level() const {
        return static_cast<Level>(level_);
    }

    const std::string &get_topic() const {
        return topic_;
    }

  private:
    template <typename... Args>
    void log(ulog_level level, const std::string &format,
             const std::source_location &loc, Args &&...args) const {
        if (level < level_) {
            return;
        }
        ulog_log(level, loc.file_name(), static_cast<int>(loc.line()),
                 topic_.empty() ? nullptr : topic_.c_str(), format.c_str(),
                 std::forward<Args>(args)...);
    }

    std::string topic_;
    ulog_level level_;
};

}  // namespace ulog
