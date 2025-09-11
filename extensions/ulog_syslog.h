// *************************************************************************
//
// microlog extension: Syslog Levels
// ---------------------------------
// Convenience helpers for using RFC 5424 style syslog severities with
// microlog without modifying the core library headers or sources.
//
// This extension only relies on the public API from `ulog.h` and may be
// freely included or omitted. All symbols are prefixed with `ulog_syslog_`
// (functions) or `ULOG_SYSLOG_` (macros / enums) to avoid namespace clashes.
//
// Usage (long names):
//   #include "ulog_syslog.h"
//   ulog_syslog_enable(ULOG_SYSLOG_STYLE_LONG); // DEBUG |, INFO |, ...
//   ulog_sys_debug("Starting (%d)", 42);
//   ulog_sys_error("Failure");
//   ulog_syslog_disable(); // Restore default microlog levels
//
// Topic variants (if topics enabled):
//   ulog_sys_t_warn("Net", "Link down");
//
// You can switch to short names:
//   ulog_syslog_enable(ULOG_SYSLOG_STYLE_SHORT); // DBG|, INF|, etc.
//
// NOTES:
// * The underlying core expects `max_level` to be strictly greater than the
//   highest usable level index. We therefore set it to `ULOG_LEVEL_TOTAL` so
//   all 8 levels (0..7) are accepted.
// * Calling `ulog_level_set_new_levels()` directly elsewhere after enabling
//   this extension will override syslog levels; `ulog_syslog_is_active()` may
//   then return a stale value. Re-enable if needed.
// * Thread safety: The extension uses only atomic transitions via the core
//   API and internal static data; no additional locking is required beyond
//   what microlog already performs internally.
//
// MIT License (same as microlog core)
// *************************************************************************

#pragma once

#include <stdbool.h>
#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

// ==========================================================================
// Public: Mapping macros for syslog severity names
// ==========================================================================
// RFC 5424 severities ascending in numeric value but descending in priority.
// microlog interprets a higher numeric value as higher severity; we keep the
// conventional ordering but name macros for readability.

#define ULOG_SYSLOG_DEBUG  ULOG_LEVEL_0  // 7 Debug
#define ULOG_SYSLOG_INFO   ULOG_LEVEL_1  // 6 Informational
#define ULOG_SYSLOG_NOTICE ULOG_LEVEL_2  // 5 Notice
#define ULOG_SYSLOG_WARN   ULOG_LEVEL_3  // 4 Warning
#define ULOG_SYSLOG_ERR    ULOG_LEVEL_4  // 3 Error
#define ULOG_SYSLOG_CRIT   ULOG_LEVEL_5  // 2 Critical
#define ULOG_SYSLOG_ALERT  ULOG_LEVEL_6  // 1 Alert
#define ULOG_SYSLOG_EMERG  ULOG_LEVEL_7  // 0 Emergency (highest severity)

// ==========================================================================
// Styles (long vs short labels)
// ==========================================================================
typedef enum {
	ULOG_SYSLOG_STYLE_LONG = 0,  // e.g. "ERROR  |"
	ULOG_SYSLOG_STYLE_SHORT,     // e.g. "ERR|"
} ulog_syslog_style;

// ==========================================================================
// API
// ==========================================================================

/// Enable syslog level names (long or short). Replaces current level
/// descriptor with a persistent internal descriptor.
/// @param style ULOG_SYSLOG_STYLE_LONG or ULOG_SYSLOG_STYLE_SHORT
/// @return ULOG_STATUS_OK on success, error otherwise.
ulog_status ulog_syslog_enable(ulog_syslog_style style);

/// Disable syslog level names and restore default microlog levels.
/// @return ULOG_STATUS_OK on success, error otherwise.
ulog_status ulog_syslog_disable(void);

/// Returns true if the extension believes syslog levels are active.
bool ulog_syslog_is_active(void);

/// Get pointer to active syslog descriptor (NULL if not active).
const ulog_level_descriptor *ulog_syslog_get_descriptor(void);

// ==========================================================================
// Convenience logging macros (plain)
// ==========================================================================
// These map directly onto the generic `ulog_log()` macro which expects the
// level as the first argument. They continue to work even if you later switch
// back to default levels, though the textual representation will obviously
// change.

#define ulog_sys_debug(...)  ulog_log(ULOG_SYSLOG_DEBUG, __VA_ARGS__)
#define ulog_sys_info(...)   ulog_log(ULOG_SYSLOG_INFO, __VA_ARGS__)
#define ulog_sys_notice(...) ulog_log(ULOG_SYSLOG_NOTICE, __VA_ARGS__)
#define ulog_sys_warn(...)   ulog_log(ULOG_SYSLOG_WARN, __VA_ARGS__)
#define ulog_sys_error(...)  ulog_log(ULOG_SYSLOG_ERR, __VA_ARGS__)
#define ulog_sys_crit(...)   ulog_log(ULOG_SYSLOG_CRIT, __VA_ARGS__)
#define ulog_sys_alert(...)  ulog_log(ULOG_SYSLOG_ALERT, __VA_ARGS__)
#define ulog_sys_emerg(...)  ulog_log(ULOG_SYSLOG_EMERG, __VA_ARGS__)

// ==========================================================================
// Convenience logging macros (topics) - only effective if topics enabled
// ==========================================================================
#define ulog_sys_t_debug(TOPIC, ...)  ulog_topic_log(ULOG_SYSLOG_DEBUG, TOPIC, __VA_ARGS__)
#define ulog_sys_t_info(TOPIC, ...)   ulog_topic_log(ULOG_SYSLOG_INFO, TOPIC, __VA_ARGS__)
#define ulog_sys_t_notice(TOPIC, ...) ulog_topic_log(ULOG_SYSLOG_NOTICE, TOPIC, __VA_ARGS__)
#define ulog_sys_t_warn(TOPIC, ...)   ulog_topic_log(ULOG_SYSLOG_WARN, TOPIC, __VA_ARGS__)
#define ulog_sys_t_error(TOPIC, ...)  ulog_topic_log(ULOG_SYSLOG_ERR, TOPIC, __VA_ARGS__)
#define ulog_sys_t_crit(TOPIC, ...)   ulog_topic_log(ULOG_SYSLOG_CRIT, TOPIC, __VA_ARGS__)
#define ulog_sys_t_alert(TOPIC, ...)  ulog_topic_log(ULOG_SYSLOG_ALERT, TOPIC, __VA_ARGS__)
#define ulog_sys_t_emerg(TOPIC, ...)  ulog_topic_log(ULOG_SYSLOG_EMERG, TOPIC, __VA_ARGS__)

#ifdef __cplusplus
}
#endif

