// *************************************************************************
//
// ulog v@{ULOG_VERSION}@ - A simple customizable logging library.
// https://github.com/an-dr/microlog
//
// Extension: Syslog Levels.
//
// Convenience helpers for using RFC 5424 style syslog severities with
// microlog without modifying the core library headers or sources.
//
// Usage:
//
//   #include "ulog_syslog.h"
//   ulog_syslog_enable(); // DEBUG |, INFO |, ...
//   ulog_sl_debug("Starting (%d)", 42);
//   ulog_sl_error("Failure");
//   ulog_syslog_disable(); // Restore default microlog levels
//
// Topic variants (if topics enabled):
//
//   ulog_sl_t_warn("Net", "Link down");
//
// *************************************************************************
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// site:    https://agramakov.me
// e-mail:  mail@agramakov.me
//
// Copyright (c) 2025 Andrei Gramakov. All rights reserved.
//
// This file is licensed under the terms of the MIT license.
// For a copy, see: https://opensource.org/licenses/MIT
//
// *************************************************************************

#pragma once

#include <stdbool.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off

/// @brief Mapping macros for syslog severity names.
/// RFC 5424 severities ascending in numeric value but descending in priority.
/// microlog interprets a higher numeric value as higher severity; we keep the
/// conventional ordering but name macros for readability.
#define ULOG_SYSLOG_DEBUG   ULOG_LEVEL_0    // 7 Debug
#define ULOG_SYSLOG_INFO    ULOG_LEVEL_1    // 6 Informational
#define ULOG_SYSLOG_NOTICE  ULOG_LEVEL_2    // 5 Notice
#define ULOG_SYSLOG_WARN    ULOG_LEVEL_3    // 4 Warning
#define ULOG_SYSLOG_ERR     ULOG_LEVEL_4    // 3 Error
#define ULOG_SYSLOG_CRIT    ULOG_LEVEL_5    // 2 Critical
#define ULOG_SYSLOG_ALERT   ULOG_LEVEL_6    // 1 Alert
#define ULOG_SYSLOG_EMERG   ULOG_LEVEL_7    // 0 Emergency (highest severity)

// Convenience logging macros.
// These map directly onto the generic `ulog_log()` macro which expects the
// level as the first argument. They continue to work even if you later switch
// back to default levels, though the textual representation will obviously
// change.
#define ulog_sl_debug(...)     ulog_log(ULOG_SYSLOG_DEBUG, __VA_ARGS__)
#define ulog_sl_info(...)      ulog_log(ULOG_SYSLOG_INFO, __VA_ARGS__)
#define ulog_sl_notice(...)    ulog_log(ULOG_SYSLOG_NOTICE, __VA_ARGS__)
#define ulog_sl_warn(...)      ulog_log(ULOG_SYSLOG_WARN, __VA_ARGS__)
#define ulog_sl_error(...)     ulog_log(ULOG_SYSLOG_ERR, __VA_ARGS__)
#define ulog_sl_crit(...)      ulog_log(ULOG_SYSLOG_CRIT, __VA_ARGS__)
#define ulog_sl_alert(...)     ulog_log(ULOG_SYSLOG_ALERT, __VA_ARGS__)
#define ulog_sl_emerg(...)     ulog_log(ULOG_SYSLOG_EMERG, __VA_ARGS__)
#define ulog_sl_t_debug(TOPIC, ...)    ulog_topic_log(ULOG_SYSLOG_DEBUG, TOPIC, __VA_ARGS__)
#define ulog_sl_t_info(TOPIC, ...)     ulog_topic_log(ULOG_SYSLOG_INFO, TOPIC, __VA_ARGS__)
#define ulog_sl_t_notice(TOPIC, ...)   ulog_topic_log(ULOG_SYSLOG_NOTICE, TOPIC, __VA_ARGS__)
#define ulog_sl_t_warn(TOPIC, ...)     ulog_topic_log(ULOG_SYSLOG_WARN, TOPIC, __VA_ARGS__)
#define ulog_sl_t_error(TOPIC, ...)    ulog_topic_log(ULOG_SYSLOG_ERR, TOPIC, __VA_ARGS__)
#define ulog_sl_t_crit(TOPIC, ...)     ulog_topic_log(ULOG_SYSLOG_CRIT, TOPIC, __VA_ARGS__)
#define ulog_sl_t_alert(TOPIC, ...)    ulog_topic_log(ULOG_SYSLOG_ALERT, TOPIC, __VA_ARGS__)
#define ulog_sl_t_emerg(TOPIC, ...)    ulog_topic_log(ULOG_SYSLOG_EMERG, TOPIC, __VA_ARGS__)

// clang-format on

/// @brief Enable syslog level names (long or short). Replaces current level
/// descriptor with a persistent internal descriptor.
/// @return ULOG_STATUS_OK on success, error otherwise.
ulog_status ulog_syslog_enable();

/// @brief Disable syslog level names and restore default microlog levels.
/// @return ULOG_STATUS_OK on success, error otherwise.
ulog_status ulog_syslog_disable(void);

#ifdef __cplusplus
}
#endif
