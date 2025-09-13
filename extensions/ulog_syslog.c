// *************************************************************************
// microlog extension: Syslog Levels (implementation)
// *************************************************************************
// This file provides an opt-in level descriptor matching traditional syslog
// severities. It leaves the core library untouched. Include the header and
// call `ulog_syslog_enable()` to activate.
// *************************************************************************

#include "ulog_syslog.h"

static ulog_level_descriptor syslog_levels = {
    .max_level = ULOG_LEVEL_7,  // allow 0..7
    .names     = {"DEBUG ", "INFO  ", "NOTICE", "WARN  ", "ERR   ", "CRIT  ",
                  "ALERT ", "EMERG "},  // We use fixed-width names for alignment
};

ulog_status ulog_syslog_enable(void) {
    return ulog_level_set_new_levels(&syslog_levels);
}

ulog_status ulog_syslog_disable(void) {
    return ulog_level_reset_levels();
}
