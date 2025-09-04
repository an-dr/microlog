# Changelog

## [v7.0.0-alpha.1] - September 4, 2025

> **🚨 BREAKING CHANGES**: Complete API redesign from v6.5.0. This is a major architectural overhaul with new type system, function names, and build configuration.

### 🌟 New Features

- **Output Management System**: Complete redesign with unified output handling
  - `ulog_output_add()` - Add custom callback outputs (replaces `ulog_add_callback()`)
  - `ulog_output_add_file()` - Add file outputs (replaces `ulog_add_fp()`)
  - `ulog_output_remove()` - Remove outputs dynamically (NEW)
  - `ulog_output_level_set()` / `ulog_output_level_set_all()` - Per-output level control (replaces `ulog_set_quiet()`)

- **Enhanced Type System**: Modern C enum types for better type safety
  - `ulog_level` enum: `ULOG_LEVEL_TRACE`, `ULOG_LEVEL_DEBUG`, `ULOG_LEVEL_INFO`, `ULOG_LEVEL_WARN`, `ULOG_LEVEL_ERROR`, `ULOG_LEVEL_FATAL`
  - `ulog_status` enum: `ULOG_STATUS_OK`, `ULOG_STATUS_ERROR`, `ULOG_STATUS_INVALID_ARGUMENT`
  - `ulog_topic_id` type for topic identification
  - `ulog_output` type for output handles

- **Event Getter Functions**: Access to private event data through standardized getters
  - `ulog_event_get_message()` - Extract formatted message to buffer
  - `ulog_event_get_topic()` - Get topic ID from event
  - `ulog_event_get_time()` - Get timestamp from event  
  - `ulog_event_get_file()` / `ulog_event_get_line()` - Get source location
  - `ulog_event_get_level()` - Get log level from event

- **Enhanced Macro System**: Dual macro system with backward compatibility
  - Primary: `ulog_trace()`, `ulog_debug()`, `ulog_info()`, `ulog_warn()`, `ulog_error()`, `ulog_fatal()`
  - Aliases: `log_trace()`, `log_debug()`, etc. (unchanged for compatibility)
  - Topic macros: `ulog_topic_*(topic, ...)` with `logt_*()` aliases

### 🔧 Complete API Redesign

**Build Configuration Revolution**:

- **Old System**: Feature flags (`ULOG_FEATURE_*`, `ULOG_NO_COLOR`, `ULOG_HAVE_TIME`, `ULOG_CUSTOM_PREFIX_SIZE`, etc.)
- **New System**: Unified `ULOG_BUILD_*` pattern with consistent semantics:
  - `ULOG_BUILD_COLOR=1` (replaces `!ULOG_NO_COLOR`)
  - `ULOG_BUILD_TIME=1` (replaces `ULOG_HAVE_TIME`)
  - `ULOG_BUILD_PREFIX_SIZE=N` (replaces `ULOG_CUSTOM_PREFIX_SIZE`)
  - `ULOG_BUILD_EXTRA_OUTPUTS=N` (replaces `ULOG_EXTRA_OUTPUTS`)
  - `ULOG_BUILD_SOURCE_LOCATION=1` (replaces `!ULOG_HIDE_FILE_STRING`)
  - `ULOG_BUILD_LEVEL_STYLE` (replaces `ULOG_SHORT_LEVEL_STRINGS`)
  - `ULOG_BUILD_TOPICS_NUM=N` (replaces `ULOG_TOPICS_NUM`)
  - `ULOG_BUILD_DYNAMIC_CONFIG=1` (replaces `ULOG_RUNTIME_MODE`)

**Level Constants Migration**:

- **Old**: `LOG_TRACE`, `LOG_DEBUG`, `LOG_INFO`, `LOG_WARN`, `LOG_ERROR`, `LOG_FATAL`
- **New**: `ULOG_LEVEL_TRACE`, `ULOG_LEVEL_DEBUG`, `ULOG_LEVEL_INFO`, `ULOG_LEVEL_WARN`, `ULOG_LEVEL_ERROR`, `ULOG_LEVEL_FATAL`

**Function Renaming for Consistency**:

- **Core Functions**:
  - `ulog_get_level_string()` → `ulog_level_to_string()`
  - `ulog_set_level()` → `ulog_output_level_set_all()`
  - `ulog_set_quiet()` → `ulog_output_level_set(ULOG_OUTPUT_STDOUT, level)`

- **Output Management**:
  - `ulog_add_callback()` → `ulog_output_add()`
  - `ulog_add_fp()` → `ulog_output_add_file()`

- **Configuration Functions**:
  - `ulog_configure_levels()` → `ulog_level_config()`
  - `ulog_configure_prefix()` → `ulog_prefix_config()`
  - `ulog_configure_color()` → `ulog_color_config()`
  - `ulog_configure_time()` → `ulog_time_config()`
  - `ulog_configure_topics()` → `ulog_topic_config()`
  - `ulog_configure_file_string()` → `ulog_source_location_config()`

- **Topic Functions** (complete renaming):
  - `ulog_add_topic()` → `ulog_topic_add()`
  - `ulog_set_topic_level()` → `ulog_topic_level_set()`
  - `ulog_get_topic_id()` → `ulog_topic_get_id()`
  - `ulog_enable_topic()` → `ulog_topic_enable()`
  - `ulog_disable_topic()` → `ulog_topic_disable()`
  - `ulog_enable_all_topics()` → `ulog_topic_enable_all()`
  - `ulog_disable_all_topics()` → `ulog_topic_disable_all()`

- **Other Functions**:
  - `ulog_set_lock()` → `ulog_lock_set_fn()`
  - `ulog_set_prefix_fn()` → `ulog_prefix_set_fn()`

**Type Naming Standardization**:

- **Old**: Mixed case (`ulog_Event`, `ulog_LogFn`, `ulog_LockFn`, `ulog_PrefixFn`)
- **New**: Consistent lowercase with underscores (`ulog_event`, `ulog_log_fn`, `ulog_lock_fn`, `ulog_prefix_fn`)

### 🏗️ Implementation Architecture Changes

**Event System Redesign**:

- **Private Event Structure**: `ulog_event` is now completely opaque
- **Safer Access**: All event data accessed through getter functions
- **Better Encapsulation**: Internal structure changes don't break API
- **Type Safety**: Stronger typing with proper enums instead of raw integers

**Print Target System**:

- **Old**: `log_target_t` with `LOG_TARGET_BUFFER`, `LOG_TARGET_STREAM`
- **New**: `print_target_type` with `PRINT_TARGET_BUFFER`, `PRINT_TARGET_STREAM`

**Callback System Overhaul**:

- **Old**: Fixed callback array with `cb_t` structures
- **New**: Dynamic output management with `ulog_output` handles
- **Better Resource Management**: Proper output creation/removal lifecycle
- **Unified Interface**: Same API for files, callbacks, and stdout

**Topic Display Format**:

- **Old**: `[Topic] LEVEL` format
- **New**: `LEVEL [Topic]` format for better readability

### 📦 Feature Changes

**Removed Features**:

- **Emoji Levels**: `ULOG_FEATURE_EMOJI_LEVELS` / `ULOG_USE_EMOJI` removed entirely
- **Mixed Level Styles**: Simplified to `ULOG_BUILD_LEVEL_STYLE` configuration

**Renamed Features**:

- **Custom Prefix** → **Prefix**: Simplified naming (`ULOG_BUILD_PREFIX_SIZE`)
- **Extra Outputs** → **Outputs**: Unified output system (`ULOG_BUILD_EXTRA_OUTPUTS`)
- **File String** → **Source Location**: More descriptive (`ULOG_BUILD_SOURCE_LOCATION`)
- **Runtime Mode** → **Dynamic Config**: More accurate (`ULOG_BUILD_DYNAMIC_CONFIG`)

### 🐞 Implementation Fixes

- **Color Logic**: Fixed inverted `ULOG_HAVE_COLOR` vs `ULOG_NO_COLOR` logic in color feature detection
- **Prefix Execution**: Fixed multiple prefix callback execution per single log call
- **Topic Processing**: Enhanced topic string processing and validation
- **Memory Management**: Improved buffer handling in print system
- **Type Safety**: Eliminated integer/enum mixing issues

### 🧪 Test Suite Updates

**Test Function Migration**:

- Updated all test cases to use new API functions
- `ulog_add_callback()` → `ulog_output_add()` in test fixtures
- `LOG_TRACE` → `ULOG_LEVEL_TRACE` in level tests
- `ulog_set_level()` → `ulog_output_level_set_all()` in level configuration
- Enhanced test coverage for new getter functions

**Test File Changes**:

- `test_runtime_config.cpp` → `test_dynamic_config.cpp`
- `test_custom_prefix.cpp` → `test_prefix.cpp`
- Updated callback headers and utilities

### 📚 Documentation & Examples

**Example Application**:

- Complete rewrite using new v7.0 API
- Demonstrates new output management system
- Shows both `ulog_*()` and `log_*()` macro usage
- Updated topic examples with new function names
- Better inline documentation

**Build System**:

- Updated CMake and Meson build files
- New build option examples in documentation
- Migration guides for build configuration

---

**Critical Migration Notes**:

1. **All function names changed** - Use find/replace with mapping above
2. **All log level constants changed** - `LOG_*` → `ULOG_LEVEL_*`
3. **All build defines changed** - `ULOG_FEATURE_*` → `ULOG_BUILD_*`
4. **Event access changed** - Direct field access → getter functions
5. **Output system changed** - Callback registration → output management
6. **Type names changed** - CamelCase → snake_case

**Version**: 6.5.0 → 7.0.0-alpha.1
