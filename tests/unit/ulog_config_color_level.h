#pragma once
// Static config with COLOR=1, LEVEL_SHORT=1, SOURCE_LOCATION=0, TIME=1.
// Used by test_event_to_cstr to verify compile-time config flows through
// ulog_event_to_cstr / ulog_event_to_cstr_colored.
#define ULOG_BUILD_SOURCE_LOCATION  0  // user wants no file:line prefix
#define ULOG_BUILD_LEVEL_SHORT      1  // user wants short level names (I, W…)
#define ULOG_BUILD_COLOR            1  // user wants ANSI colour
#define ULOG_BUILD_TIME             1  // user wants timestamps
#define ULOG_BUILD_DYNAMIC_CONFIG   0  // user intends static config — must NOT activate dynamic mode
#define ULOG_BUILD_EXTRA_OUTPUTS    4  // needed to register a custom handler in tests
#define ULOG_BUILD_WARN_NOT_ENABLED 1
