#pragma once
// Config header that reproduces saphieron's setup from issue #157.
// Matches the relevant fields from their ulog_config.h (STM32H7 / USART project).
#define ULOG_BUILD_SOURCE_LOCATION  0  // user wants no file:line prefix
#define ULOG_BUILD_LEVEL_SHORT      1  // user wants short level names (I, W…)
#define ULOG_BUILD_COLOR            1  // user wants ANSI colour
#define ULOG_BUILD_TIME             1  // user wants timestamps
#define ULOG_BUILD_DYNAMIC_CONFIG   0  // user intends static config — must NOT activate dynamic mode
#define ULOG_BUILD_EXTRA_OUTPUTS    4  // needed to register a custom handler in tests
#define ULOG_BUILD_WARN_NOT_ENABLED 1
