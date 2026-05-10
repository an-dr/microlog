#pragma once
// Config header that reproduces the user's setup from issue #157.
// The user explicitly disabled source location and level-short,
// and enabled color and time — expecting all four to be respected.
#define ULOG_BUILD_SOURCE_LOCATION 0  // user wants no file:line prefix
#define ULOG_BUILD_LEVEL_SHORT     1  // user wants short level names (I, W…)
#define ULOG_BUILD_COLOR           1  // user wants ANSI colour
#define ULOG_BUILD_TIME            1  // user wants timestamps
#define ULOG_BUILD_DYNAMIC_CONFIG  0
#define ULOG_BUILD_WARN_NOT_ENABLED 1
