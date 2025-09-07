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

/// @brief Core logging functionality (global, no topics)
class Log {
public:
    // Core logging functions with automatic source location
    template <typename... Args>
    static void
    trace(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    debug(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_OUTPUT_ALL, ULOG_LEVEL_DEBUG, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    info(const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_OUTPUT_ALL, ULOG_LEVEL_INFO, loc.file_name(), static_cast<int>(loc.line()),
                 nullptr, format.c_str(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    warn(const std::string &format, Args &&...args,
         std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_OUTPUT_ALL, ULOG_LEVEL_WARN, loc.file_name(), static_cast<int>(loc.line()),
                 nullptr, format.c_str(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    error(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_OUTPUT_ALL, ULOG_LEVEL_ERROR, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    template <typename... Args>
    static void
    fatal(const std::string &format, Args &&...args,
          std::source_location loc = std::source_location::current()) {
        ulog_log(ULOG_OUTPUT_ALL, ULOG_LEVEL_FATAL, loc.file_name(),
                 static_cast<int>(loc.line()), nullptr, format.c_str(),
                 std::forward<Args>(args)...);
    }

    /// @brief Set global log level for all outputs
    static void set_level(Level level) {
        ulog_output_level_set_all(static_cast<ulog_level>(level));
    }

private:
    Log() = delete;  // Prevent instantiation
    ~Log() = delete;
    Log(const Log &) = delete;
    Log &operator=(const Log &) = delete;
};

/// @brief Instance-based logger class with full topic management
class Logger {
  public:
    /// @brief Create a logger with optional topic and level
    Logger(const std::string &topic = "", Level level = Level::trace)
        : topic_(topic), level_(static_cast<ulog_level>(level)) {
        if (!topic_.empty()) {
            // Register topic and set its level
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

    /// @brief Set minimum log level for this logger instance
    void set_level(Level level) {
        level_ = static_cast<ulog_level>(level);
        if (!topic_.empty()) {
            ulog_topic_level_set(topic_.c_str(), level_);
        }
    }

    /// @brief Enable this logger's topic (if it has one)
    Status enable() {
        if (topic_.empty()) {
            return Status::ok; // No topic to enable
        }
        return static_cast<Status>(ulog_topic_enable(topic_.c_str()));
    }

    /// @brief Disable this logger's topic (if it has one)
    Status disable() {
        if (topic_.empty()) {
            return Status::ok; // No topic to disable
        }
        return static_cast<Status>(ulog_topic_disable(topic_.c_str()));
    }

    /// @brief Get the current log level
    Level get_level() const {
        return static_cast<Level>(level_);
    }

    /// @brief Get the topic name (empty if no topic)
    const std::string &get_topic() const {
        return topic_;
    }

    /// @brief Check if this logger has a topic
    bool has_topic() const {
        return !topic_.empty();
    }

    /// @brief Get the topic ID (returns ULOG_TOPIC_ID_INVALID if no topic)
    int get_topic_id() const {
        if (topic_.empty()) {
            return ULOG_TOPIC_ID_INVALID;
        }
        return ulog_topic_get_id(topic_.c_str());
    }

    // Static methods for global topic operations
    
    /// @brief Enable a specific topic globally (affects all loggers using this topic)
    static Status enable_topic(const std::string &topic_name) {
        return static_cast<Status>(ulog_topic_enable(topic_name.c_str()));
    }

    /// @brief Disable a specific topic globally (affects all loggers using this topic)
    static Status disable_topic(const std::string &topic_name) {
        return static_cast<Status>(ulog_topic_disable(topic_name.c_str()));
    }

    /// @brief Enable all existing topics globally
    static Status enable_all_topics() {
        return static_cast<Status>(ulog_topic_enable_all());
    }

    /// @brief Disable all existing topics globally
    static Status disable_all_topics() {
        return static_cast<Status>(ulog_topic_disable_all());
    }

    /// @brief Get topic ID by name (utility function)
    static int get_topic_id(const std::string &topic_name) {
        return ulog_topic_get_id(topic_name.c_str());
    }

    // Output-specific logging methods
    
    /// @brief Log to a specific output only
    template <typename... Args>
    void log_to_output(int output_handle, Level level, const std::string &format,
                      Args &&...args, std::source_location loc = std::source_location::current()) const {
        if (static_cast<ulog_level>(level) < level_) {
            return;
        }
        Output::log_to(output_handle, level, topic_, format, 
                      std::forward<Args>(args)..., loc);
    }

    /// @brief Trace to specific output
    template <typename... Args>
    void trace_to(int output_handle, const std::string &format, Args &&...args,
                 std::source_location loc = std::source_location::current()) const {
        log_to_output(output_handle, Level::trace, format, std::forward<Args>(args)..., loc);
    }

    /// @brief Debug to specific output
    template <typename... Args>
    void debug_to(int output_handle, const std::string &format, Args &&...args,
                 std::source_location loc = std::source_location::current()) const {
        log_to_output(output_handle, Level::debug, format, std::forward<Args>(args)..., loc);
    }

    /// @brief Info to specific output
    template <typename... Args>
    void info_to(int output_handle, const std::string &format, Args &&...args,
                std::source_location loc = std::source_location::current()) const {
        log_to_output(output_handle, Level::info, format, std::forward<Args>(args)..., loc);
    }

    /// @brief Warn to specific output
    template <typename... Args>
    void warn_to(int output_handle, const std::string &format, Args &&...args,
                std::source_location loc = std::source_location::current()) const {
        log_to_output(output_handle, Level::warn, format, std::forward<Args>(args)..., loc);
    }

    /// @brief Error to specific output
    template <typename... Args>
    void error_to(int output_handle, const std::string &format, Args &&...args,
                 std::source_location loc = std::source_location::current()) const {
        log_to_output(output_handle, Level::error, format, std::forward<Args>(args)..., loc);
    }

    /// @brief Fatal to specific output
    template <typename... Args>
    void fatal_to(int output_handle, const std::string &format, Args &&...args,
                 std::source_location loc = std::source_location::current()) const {
        log_to_output(output_handle, Level::fatal, format, std::forward<Args>(args)..., loc);
    }

  private:
    template <typename... Args>
    void log(ulog_level level, const std::string &format,
             const std::source_location &loc, Args &&...args) const {
        if (level < level_) {
            return;
        }
        ulog_log(ULOG_OUTPUT_ALL, level, loc.file_name(), static_cast<int>(loc.line()),
                 topic_.empty() ? nullptr : topic_.c_str(), format.c_str(),
                 std::forward<Args>(args)...);
    }

    std::string topic_;
    ulog_level level_;
};

}  // namespace ulog
