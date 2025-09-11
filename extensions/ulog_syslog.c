// *************************************************************************
// microlog extension: Syslog Levels (implementation)
// *************************************************************************
// This file provides an opt-in level descriptor matching traditional syslog
// severities. It leaves the core library untouched. Include the header and
// call `ulog_syslog_enable(...)` to activate.
// *************************************************************************

#include "ulog_syslog.h"

// We intentionally avoid including any non-public microlog internals.
// All operations go through the stable public functions.

// Long variant (aligned + pipe for visual separation from message body)
static ulog_level_descriptor syslog_levels_long = {
	.max_level = ULOG_LEVEL_TOTAL,  // allow 0..7
	.names = {"DEBUG  |", "INFO   |", "NOTICE |", "WARN   |", "ERR    |", "CRIT   |",
			  "ALERT  |", "EMERG  |"},
};

// Short variant (compact)
static ulog_level_descriptor syslog_levels_short = {
	.max_level = ULOG_LEVEL_TOTAL,
	.names = {"DBG|", "INF|", "NTC|", "WRN|", "ERR|", "CRT|", "ALR|", "EMG|"},
};

// Track which descriptor we installed. This is advisory only; if user calls
// ulog_level_set_new_levels() directly we won't know. They can re-enable.
static const ulog_level_descriptor *active_descriptor = NULL;

ulog_status ulog_syslog_enable(ulog_syslog_style style) {
	const ulog_level_descriptor *target = NULL;
	switch (style) {
		case ULOG_SYSLOG_STYLE_LONG: target = &syslog_levels_long; break;
		case ULOG_SYSLOG_STYLE_SHORT: target = &syslog_levels_short; break;
		default: return ULOG_STATUS_INVALID_ARGUMENT;
	}
	ulog_status st = ulog_level_set_new_levels((ulog_level_descriptor *)target);
	if (st == ULOG_STATUS_OK) {
		active_descriptor = target;
	}
	return st;
}

ulog_status ulog_syslog_disable(void) {
	ulog_status st = ulog_level_reset_levels();
	if (st == ULOG_STATUS_OK) {
		active_descriptor = NULL;
	}
	return st;
}

bool ulog_syslog_is_active(void) { return active_descriptor != NULL; }

const ulog_level_descriptor *ulog_syslog_get_descriptor(void) {
	return active_descriptor;
}

