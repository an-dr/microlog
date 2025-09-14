// *************************************************************************
//
// microlog extension: Syslog Levels.
//
// Convenience helpers for using RFC 5424 style syslog severities with
// microlog without modifying the core library headers or sources.
//
// Usage:
//
//   #include "ulog_syslog.h"
//  ...
//   ulog_syslog_enable(); // DEBUG |, INFO |, ...
//   ulog(ULOG_SYSLOG_DEBUG, "Starting (%d)", 42);
//   ulog(ULOG_SYSLOG_ERR, "Failure");
//   ulog_syslog_disable(); // Restore default microlog levels
//
// Topic variants (if topics enabled):
//
//   ulog_t(ULOG_SYSLOG_WARN, "Net", "Link down");
// 
// *************************************************************************

#pragma once

#include <stdbool.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

/// @brief Mapping macros for syslog severity names.
/// RFC 5424 severities ascending in numeric value but descending in priority.
/// microlog interprets a higher numeric value as higher severity; we keep the
/// conventional ordering but name macros for readability.

/// @brief Debug-level messages.
/// Provides detailed diagnostic information intended for developers.
/// Example: variable values, function entry/exit traces, protocol state
/// transitions.
#define ULOG_SYSLOG_DEBUG ULOG_LEVEL_0

/// @brief Informational messages.
/// Confirms normal operation and expected behavior of the system.
/// Example: module initialization completed, service started successfully.
#define ULOG_SYSLOG_INFO ULOG_LEVEL_1

/// @brief Notice conditions.
/// Highlights significant but non-error events that may require attention.
/// Example: configuration reloaded, user session established, firmware updated.
#define ULOG_SYSLOG_NOTICE ULOG_LEVEL_2

/// @brief Warning conditions.
/// Indicates abnormal or unexpected events that could escalate if not
/// addressed. Example: nearing memory or disk limits, transient network
/// failures, retries.
#define ULOG_SYSLOG_WARN ULOG_LEVEL_3

/// @brief Error conditions.
/// Reports failures of specific operations that impact functionality but are
/// not fatal. Example: file write failed, sensor read error, communication
/// timeout.
#define ULOG_SYSLOG_ERR ULOG_LEVEL_4

/// @brief Critical conditions.
/// Identifies serious issues that may compromise overall system functionality.
/// Example: database inaccessible, corrupted configuration, subsystem failure.
#define ULOG_SYSLOG_CRIT ULOG_LEVEL_5

/// @brief Alert conditions.
/// Requires immediate intervention to prevent full system failure.
/// Example: primary storage offline, authentication system unavailable,
/// resource exhaustion.
#define ULOG_SYSLOG_ALERT ULOG_LEVEL_6

/// @brief Emergency conditions.
/// System is unusable. Highest severity, triggers fail-safe or shutdown.
/// Example: kernel panic, unrecoverable hardware fault, power failure.
#define ULOG_SYSLOG_EMERG ULOG_LEVEL_7

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
