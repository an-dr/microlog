# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [v7.0.1] - February 22, 2026

### Fixed

- Fixed a crash on ARM64 Windows where `va_start` corrupted the `time` field in a log event, causing an assertion failure inside `strftime()`.

## [v7.0.0] - October 23, 2025

> 🚨 **BREAKING CHANGES**: Complete API redesign from v6.5.0. This is a major architectural overhaul with new type system, function names, and build configuration.

### Added

#### Core Features

- **Output Management System**: Unified system replacing callbacks for logging destinations

  - `ulog_output_add()` - Add custom callback outputs
  - `ulog_output_add_file()` - Add file outputs
  - `ulog_output_remove()` - Remove outputs dynamically
  - `ulog_output_level_set()` / `ulog_output_level_set_all()` - Per-output level control
  - `ULOG_OUTPUT_STDOUT` - Predefined output for stdout
  - Topics can be bound to specific outputs via `ulog_topic_add(TOPIC, OUTPUT, LEVEL)`
- **Enhanced Type System**: Modern C enum types for better type safety

  - `ulog_level` enum: `ULOG_LEVEL_TRACE`, `ULOG_LEVEL_DEBUG`, `ULOG_LEVEL_INFO`, `ULOG_LEVEL_WARN`, `ULOG_LEVEL_ERROR`, `ULOG_LEVEL_FATAL`
  - `ulog_status` enum: `ULOG_STATUS_OK`, `ULOG_STATUS_ERROR`, `ULOG_STATUS_INVALID_ARGUMENT`, `ULOG_STATUS_NOT_FOUND`, `ULOG_STATUS_BUSY`, `ULOG_STATUS_DISABLED`
  - `ulog_topic_id` type for topic identification
  - `ulog_output` type for output handles
- **Custom Log Levels**: Define your own level schemes

  - `ulog_level_set_new_levels(ulog_level_descriptor)` - Set custom levels
  - `ulog_level_reset_levels()` - Reset to defaults
  - Generic `ULOG_LEVEL_0...7` available for custom schemes
  - New macros `ulog_t(level, topic, ...)` and `ulog(level, ...)` for dynamic level logging
- **Static Configuration Header**: Simplified build-time configuration

  - Enable with `ULOG_BUILD_CONFIG_HEADER_ENABLED=1`
  - Use `ulog_config.h` by default or customize with `ULOG_BUILD_CONFIG_HEADER_NAME`

#### Optional Features

- `ULOG_BUILD_DISABLED` option to completely disable the library at compile time.
- `ULOG_BUILD_WARN_NOT_ENABLED` option to emit warnings when used a feature that is disabled.
- `ULOG_BUILD_CONFIG_HEADER_ENABLED`, `ULOG_BUILD_CONFIG_HEADER_NAME` - options to enable a static configuration header to submit build-time options in a single file.

#### Extensions

- **Syslog Levels Extension** (`extensions/ulog_syslog.c/.h`)

  - RFC 5424 style severities: DEBUG, INFO, NOTICE, WARN, ERR, CRIT, ALERT, EMERG
- **Thread-Safe Lock Extensions** for multiple platforms:

  - CMSIS-RTOS2, FreeRTOS, POSIX, ThreadX, and Windows
- **Generic Logger Interface Extension**

  - Easier migration from/to other logging libraries
- **Microlog 6 compatibility layer**

  - Eases transition from Microlog 6 to ulog v7.0

#### Resource Management

- `ulog_topic_remove(TOPIC)` - Remove topics dynamically
- `ulog_cleanup()` - Free all dynamic resources and reset state

#### Event Access API

- `ulog_event_get_message()` - Extract formatted message to buffer
- `ulog_event_get_topic()` - Get topic ID from event
- `ulog_event_get_time()` - Get timestamp from event
- `ulog_event_get_file()` / `ulog_event_get_line()` - Get source location
- `ulog_event_get_level()` - Get log level from event
- `ulog_event_to_cstr()` - Convert event to C string

#### Quality Assurance

- Unit tests with >85% line coverage and >90% function coverage

### Changed

#### API Redesign

- **Macro System**: Dual macro system with improved naming

  - Primary: `ulog_trace()`, `ulog_debug()`, `ulog_info()`, `ulog_warn()`, `ulog_error()`, `ulog_fatal()`
  - Topic macros: `ulog_t_*()` (renamed from `logt_*`)
  - Basic `log_*` aliases removed - use `ulog_*` directly
- **Function Renaming** for consistency (selected examples):

  - `ulog_get_level_string()` → `ulog_level_to_string()`
  - `ulog_set_level()` → `ulog_output_level_set_all()`
  - `ulog_set_quiet()` → `ulog_output_level_set(ULOG_OUTPUT_STDOUT, level)`
  - `ulog_add_callback()` → `ulog_output_add()`
  - `ulog_add_fp()` → `ulog_output_add_file()`
  - `ulog_set_lock()` → `ulog_lock_set_fn()`
  - `ulog_set_prefix_fn()` → `ulog_prefix_set_fn()`
  - All topic functions now use `ulog_topic_*` prefix
  - All configuration functions now use `*_config` suffix
- **Type Naming**: Consistent snake_case convention

  - `ulog_Event` → `ulog_event`
  - `ulog_LogFn` → `ulog_log_fn`
  - `ulog_LockFn` → `ulog_lock_fn`
  - `ulog_PrefixFn` → `ulog_prefix_fn`

#### Build Configuration Revolution

All build configuration now uses unified `ULOG_BUILD_*` pattern:

| Old                          | New                                                           |
| ---------------------------- | ------------------------------------------------------------- |
| `!ULOG_NO_COLOR`           | `ULOG_BUILD_COLOR=1`                                        |
| `ULOG_HAVE_TIME`           | `ULOG_BUILD_TIME=1`                                         |
| `ULOG_CUSTOM_PREFIX_SIZE`  | `ULOG_BUILD_PREFIX_SIZE=N`                                  |
| `ULOG_EXTRA_OUTPUTS`       | `ULOG_BUILD_EXTRA_OUTPUTS=N`                                |
| `!ULOG_HIDE_FILE_STRING`   | `ULOG_BUILD_SOURCE_LOCATION=1`                              |
| `ULOG_SHORT_LEVEL_STRINGS` | `ULOG_BUILD_LEVEL_SHORT=1`                                  |
| `ULOG_TOPICS_NUM`          | `ULOG_BUILD_TOPICS_MODE` + `ULOG_BUILD_TOPICS_STATIC_NUM` |
| `ULOG_RUNTIME_MODE`        | `ULOG_BUILD_DYNAMIC_CONFIG=1`                               |

#### Topics Overhaul

- **Level-Based Control**: Topics now use log levels instead of simple enable/disable
  - `ulog_topic_add()` now takes a level parameter
  - Replaces previous binary enabled/disabled state
- **Registration Required**: All topics (static and dynamic) must use `ulog_topic_add()`
- **Display Format**: Changed from `[Topic] LEVEL` to `LEVEL [Topic]`

#### Enhanced Behavior

- **Error Handling**: Dynamic config functions and lock functions now return `ulog_status`
- **Default Levels**: Now aliases for generic `ULOG_LEVEL_0...7`
  - TRACE=0, DEBUG=1, INFO=2, WARN=3, ERROR=4, FATAL=5, Upper levels (6,7) unused by default
- **Visual Improvements**: FATAL level color changed from magenta to "red on white"
- **Feature Names**: Simplified naming (Custom Prefix → Prefix, Extra Outputs → Outputs, File String → Source Location, Runtime Mode → Dynamic Config)

### Removed

- **Emoji Levels**: `ULOG_FEATURE_EMOJI_LEVELS` / `ULOG_USE_EMOJI` completely removed
- **Direct Event Access**: Event struct removed from public API (use getter functions instead)
- **Legacy Topic Functions**: Removed due to level-based topic system
  - `ulog_topic_enable()` / `ulog_topic_enable_all()`
  - `ulog_topic_disable()` / `ulog_topic_disable_all()`
- **Basic Macro Aliases**: `log_*` aliases removed (use `ulog_*` directly)

### Fixed

- Prefix callback execution (was called multiple times per log call)
- Potential buffer overflows in printing system
- Early exits on outputs and topics iteration after removal
- Memory allocation/deallocation for dynamic topics (including name strings)
- `va_list` handling issues in `ulog_event_get_message()` and `ulog_event_to_cstr()`
- Topic string processing and validation
- Buffer handling in print system
- Various bugs in `test_time.cpp`
- Consistency and formatting in feature documentation
