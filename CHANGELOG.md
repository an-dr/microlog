# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [unreleased, v7.0.0-alpha.4]

### Added

- Add extensions to the release
- Add Static Configuration Header `ulog_config.h` feature to simplify configuration. Use `ULOG_BUILD_CONFIG_HEADER_ENABLED=1` to enable it.
- Add `ULOG_BUILD_CONFIG_HEADER_NAME` to use a static configuration header instead of default `ulog_config.h`.

### Changed

- Move extensions documentation from `doc/extensions.md` to `extensions/README.md` for better visibility
- Revamp topics handling from enable/disable to level-based
    - `ulog_topic_add()` now takes a level parameter instead of enabled/disabled state

### Removed

- Revamp topics handling from enable/disable to level-based
    - `ulog_topic_enable()` and `ulog_topic_enable_all()`
    - `ulog_topic_disable()` and `ulog_topic_disable_all()`

## [v7.0.0-alpha.3] - September 17, 2025

### Added

- Custom log levels via `ulog_level_set_new_levels(ulog_level_descriptor)` / `ulog_level_reset_levels()`
- Replace default levels with generic `ULOG_LEVEL_0...7` - for custom level schemes.
- New macros `ulog_t(level, topic, ...)` and `ulog(level, ...)` for dynamic level logging.
- `ULOG_STATUS_DISABLED` status code for disabled features.
- Extensions:
    - Syslog levels extension (`extensions/ulog_syslog.c/.h`) providing RFC 5424 style severities (DEBUG, INFO, NOTICE, WARN, ERR, CRIT, ALERT, EMERG)
    - Thread-safe lock extensions for CMSIS-RTOS2, FreeRTOS, POSIX, ThreadX, and Windows.
    - Generic logger interface extension for easier migration from/to other logging libraries.
    - New documentation page `doc/extensions.md` and README Extensions section.
- Unit tests. UT coverage: >85% lines, >90% functions

### Changed

- `ulog_prefix_set_fn` now returns `ulog_status` for error handling.
- Default levels are now aliases for generic `ULOG_LEVEL_0...7`. TRACE = 0, DEBUG = 1, INFO = 2, WARN = 3, ERROR = 4, FATAL = 5. Upper levels (6,7) are unused by default.
- Color of `FATAL` level from "magenta" to "red on white" for better visibility.
- Example application updated to demonstrate syslog levels and dynamic level switching.
- `ULOG_BUILD_LEVEL_STYLE` replaced with `ULOG_BUILD_LEVEL_SHORT` (bool, 0/1) for static configuration of level style.

### Fixed

- Minor consistency and formatting adjustments in feature documentation prior to extension introduction.
- Memory (de)allocation for dynamic topics, adding (de)allocation of name string.
- Potential problems with va_list in `ulog_event_get_message()` and `ulog_event_to_cstr()`

## [v7.0.0-alpha.2] - September 8, 2025

### Added

- Binding topic to a specific output via `ulog_topic_add(TOPIC, OUTPUT, ENABLED)`
- New status codes:
    - `ULOG_STATUS_NOT_FOUND` returned when a topic or output is not present (previously returned `ULOG_STATUS_ERROR`)
    - `ULOG_STATUS_BUSY` for lock timeouts / failed lock attempts
- `ulog_topic_remove(TOPIC)`
- `ulog_cleanup()` to free all dynamic resources and reset added entities (topics, outputs, etc.)

### Changed

- Topics are now require `ulog_topic_add()` to be used. For both static and dynamic topics.
- Standardized macro alias naming conventions for consistency
  - Renamed topic aliases: `logt_*` → `ulog_t_*` (e.g., `logt_info` → `ulog_t_info`)
  - Removed basic aliases: `log_*` → use `ulog_*` directly (e.g., `log_info` → `ulog_info`)
- Dynamic Config functions now return `ulog_status` for error handling - as they can fail on failed locks
- `ulog_lock_fn` now returns `ulog_status` - error handling for failed locks.
- Replace term "callback" with "handler" for clarity and consistency.

### Fixed

- Fix potential buffer overflows on printing
- Fix early exits on outputs and topics iteration after removal

## [v7.0.0-alpha.1] - September 4, 2025

> 🚨 BREAKING CHANGES: Complete API redesign from v6.5.0. This is a major architectural overhaul with new type system, function names, and build configuration.

### Added

- Output Management System: Outputs replace callbacks for logging destinations
  - `ulog_output_add()` - Add custom callback outputs (replaces `ulog_add_callback()`)
  - `ulog_output_add_file()` - Add file outputs (replaces `ulog_add_fp()`)
  - `ulog_output_remove()` - Remove outputs dynamically (NEW)
  - `ulog_output_level_set()` / `ulog_output_level_set_all()` - Per-output level control (replaces `ulog_set_quiet()`)
  - `ULOG_OUTPUT_STDOUT` - Predefined output for stdout (NEW)
- Enhanced Type System: Modern C enum types for better type safety
  - `ulog_level` enum: `ULOG_LEVEL_TRACE`, `ULOG_LEVEL_DEBUG`, `ULOG_LEVEL_INFO`, `ULOG_LEVEL_WARN`, `ULOG_LEVEL_ERROR`, `ULOG_LEVEL_FATAL`
  - `ulog_status` enum: `ULOG_STATUS_OK`, `ULOG_STATUS_ERROR`, `ULOG_STATUS_INVALID_ARGUMENT`
  - `ulog_topic_id` type for topic identification
  - `ulog_output` type for output handles

### Changed

- Renamed Features:
    - Custom Prefix → Prefix: Simplified naming (`ULOG_BUILD_PREFIX_SIZE`)
    - Extra Outputs → Outputs: Unified output system (`ULOG_BUILD_EXTRA_OUTPUTS`)
    - File String → Source Location: More descriptive (`ULOG_BUILD_SOURCE_LOCATION`)
    - Runtime Mode → Dynamic Config: More accurate (`ULOG_BUILD_DYNAMIC_CONFIG`)
- Enhanced Macro System: Dual macro system with backward compatibility
  - Primary: `ulog_trace()`, `ulog_debug()`, `ulog_info()`, `ulog_warn()`, `ulog_error()`, `ulog_fatal()`
  - Aliases: `log_trace()`, `log_debug()`, etc. (unchanged for compatibility)
  - Topic macros: `ulog_topic_*(topic, ...)` with `logt_*()` aliases
- Build Configuration Revolution
    - Old System: Feature flags (`ULOG_FEATURE_*`, `ULOG_NO_COLOR`, `ULOG_HAVE_TIME`, `ULOG_CUSTOM_PREFIX_SIZE`, etc.)
    - New System: Unified `ULOG_BUILD_*` pattern with consistent semantics:
    - `ULOG_BUILD_COLOR=1` (replaces `!ULOG_NO_COLOR`)
    - `ULOG_BUILD_TIME=1` (replaces `ULOG_HAVE_TIME`)
    - `ULOG_BUILD_PREFIX_SIZE=N` (replaces `ULOG_CUSTOM_PREFIX_SIZE`)
    - `ULOG_BUILD_EXTRA_OUTPUTS=N` (replaces `ULOG_EXTRA_OUTPUTS`)
    - `ULOG_BUILD_SOURCE_LOCATION=1` (replaces `!ULOG_HIDE_FILE_STRING`)
    - `ULOG_BUILD_LEVEL_STYLE` (replaces `ULOG_SHORT_LEVEL_STRINGS`)
    - `ULOG_BUILD_TOPICS_NUM=N` (replaces `ULOG_TOPICS_NUM`)
    - `ULOG_BUILD_DYNAMIC_CONFIG=1` (replaces `ULOG_RUNTIME_MODE`)
- Level Constants Migration
    - Old: `LOG_TRACE`, `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL`
    - New: `ULOG_LEVEL_TRACE`, `ULOG_LEVEL_DEBUG`, `ULOG_LEVEL_INFO`, `ULOG_LEVEL_WARN`, `ULOG_LEVEL_ERROR`, `ULOG_LEVEL_FATAL`
- Function Renaming for Consistency
    - Core Functions:
        - `ulog_get_level_string()` → `ulog_level_to_string()`
        - `ulog_set_level()` → `ulog_output_level_set_all()`
        - `ulog_set_quiet()` → `ulog_output_level_set(ULOG_OUTPUT_STDOUT, level)`
    - Output Management:
        - `ulog_add_callback()` → `ulog_output_add()`
        - `ulog_add_fp()` → `ulog_output_add_file()`
    - Configuration Functions:
        - `ulog_configure_levels()` → `ulog_level_config()`
        - `ulog_configure_prefix()` → `ulog_prefix_config()`
        - `ulog_configure_color()` → `ulog_color_config()`
        - `ulog_configure_time()` → `ulog_time_config()`
        - `ulog_configure_topics()` → `ulog_topic_config()`
        - `ulog_configure_file_string()` → `ulog_source_location_config()`
    - Topic Functions (complete renaming):
        - `ulog_add_topic()` → `ulog_topic_add()`
        - `ulog_set_topic_level()` → `ulog_topic_level_set()`
        - `ulog_get_topic_id()` → `ulog_topic_get_id()`
        - `ulog_enable_topic()` → `ulog_topic_enable()`
        - `ulog_disable_topic()` → `ulog_topic_disable()`
        - `ulog_enable_all_topics()` → `ulog_topic_enable_all()`
        - `ulog_disable_all_topics()` → `ulog_topic_disable_all()`
    - Other Functions:
        - `ulog_set_lock()` → `ulog_lock_set_fn()`
        - `ulog_set_prefix_fn()` → `ulog_prefix_set_fn()`
- Type Naming Standardization
    - Old: Mixed case (`ulog_Event`, `ulog_LogFn`, `ulog_LockFn`, `ulog_PrefixFn`)
    - New: Consistent lowercase with underscores (`ulog_event`, `ulog_log_fn`, `ulog_lock_fn`, `ulog_prefix_fn`)
- Topic Display Format
    - Old: `[Topic] LEVEL` format
    - New: `LEVEL [Topic]` format for better readability
- Testing:
    - Updated all test cases to use new API functions
    - `test_runtime_config.cpp` → `test_dynamic_config.cpp`
    - `test_custom_prefix.cpp` → `test_prefix.cpp`
- Example Application:
    - Complete rewrite using new v7.0 API
    - Demonstrates new output management system
    - Better inline documentation

### Removed

- Emoji Levels: `ULOG_FEATURE_EMOJI_LEVELS` / `ULOG_USE_EMOJI` removed entirely
- Remove event from public API: Access to private data through standardized getters
  - `ulog_event_get_message()` - Extract formatted message to buffer
  - `ulog_event_get_topic()` - Get topic ID from event
  - `ulog_event_get_time()` - Get timestamp from event  
  - `ulog_event_get_file()` / `ulog_event_get_line()` - Get source location
  - `ulog_event_get_level()` - Get log level from event

### Fixed

- Prefix Execution: Fixed multiple prefix callback execution per single log call
- Topic Processing: Enhanced topic string processing and validation
- Memory Management: Improved buffer handling in print system
- Fix bugs in the `test_time.cpp`

### Critical Migration Notes

1. All function names changed - Use find/replace with mapping above
2. All log level constants changed - `LOG_*` → `ULOG_LEVEL_*`
3. All build defines changed - `ULOG_FEATURE_*` → `ULOG_BUILD_*`
4. Event access changed - Direct field access → getter functions
5. Output system changed - Handler registration → output management
6. Type names changed - CamelCase → snake_case
