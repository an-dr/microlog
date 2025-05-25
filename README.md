# microlog

[![latest](https://img.shields.io/github/v/tag/an-dr/microlog?filter=v*&label=latest)](https://github.com/an-dr/microlog/tags)

A simple customizable logging library. Features:

- **Log topics**
    - To filter log messages by subsystems, e.g. "network", "storage", etc.
- **Callbacks for custom output**
    - E.g. files, serial ports, etc.
- **Thread-safety**
    - Via external locking injection
- **Customizable log format**
    - Color, custom dynamic data, emojis, etc.
- **Support for embedded systems**
    - Optional static memory allocation, optional color, no special dependencies

In the default configuration it looks like this:

<img src="doc/demo0.png" width="600">

...but in can be very minimalistic

<img src="doc/demo1.png" width="600">

... or feature-rich:

<img src="doc/demo2.png" width="600">

The project is based on several core principles:

- Universal for embedded and desktop applications
- No feature - no code for compilation
- Shallow learning curve, works out of box
- No dependencies
- Two files

## Table of Contents

- [microlog](#microlog)
    - [Table of Contents](#table-of-contents)
    - [Quick Start](#quick-start)
        - [Install](#install)
        - [Use](#use)
    - [User Manual](#user-manual)
        - [Basics](#basics)
        - [Log Verbosity](#log-verbosity)
        - [Thread-safety](#thread-safety)
        - [Log Topics](#log-topics)
        - [Extra Outputs](#extra-outputs)
            - [File Output](#file-output)
            - [Custom Output](#custom-output)
        - [Custom Log Prefix](#custom-log-prefix)
        - [Timestamp](#timestamp)
        - [Other Customization](#other-customization)
    - [Runtime Configuration](#runtime-configuration)
    - [Contributing](#contributing)
    - [License](#license)
    - [Credits](#credits)

## Quick Start

### Install

**Option 1 - Sources**:

- Download a Source Package from [Releases](https://github.com/an-dr/microlog/releases)
- Add sources to your system manually

**Option 2 - CMake Package**:

- Download a CMake Package from [Releases](https://github.com/an-dr/microlog/releases)
- Specify the install location:
    - Specify package storage `cmake -B./build -DCMAKE_PREFIX_PATH="~/MyCmakePackages"` or
    - Set `microlog_DIR` variable with path to the package `microlog_DIR=~/microlog-6.3.0-cmake`
- Use in your project:

```cmake
find_package(microlog 6.3.0 REQUIRED)

add_executable(example_package example.cpp)
target_link_libraries(example_package PRIVATE microlog::microlog)

add_definitions(-DULOG_NO_COLOR) # configuration
```

**Option 3 - Meson Package**:

- Download a Meson Package from [Releases](https://github.com/an-dr/microlog/releases)
- Copy the content to `MyMesonProject/subprojects`
- Add to your dependencies:

```meson
add_global_arguments('-DULOG_NO_COLOR', language: 'c') # configuration

exe = executable(
    meson.project_name(),
    src,
    include_directories: include,
    dependencies: dependency('microlog'),
)
```

### Use

```cpp
#include "ulog.h"

int main() {
    log_info("Test message from test package");
    return 0;
}
```

## User Manual

### Basics

The library provides printf-like macros for logging:

```c
log_trace(const char *fmt, ...);
log_debug(const char *fmt, ...);
log_info(const char *fmt, ...);
log_warn(const char *fmt, ...);
log_error(const char *fmt, ...);
log_fatal(const char *fmt, ...);
```

Each function takes a printf format string followed by additional arguments:

```c
log_info("Hello %s", "world")
```

Resulting in a line with the given format printed to stdout:

```
INFO src/main.c:11: Hello world
```

While some features depend on compile-time flags for their availability (e.g., `ULOG_HAVE_TIME` for timestamp functionality), most operational aspects of the logger are configured at runtime. See the [Runtime Configuration](#runtime-configuration) section for details.

Compile-time flags are typically set via compiler options, e.g., `-DULOG_NO_COLOR`.

For CMake projects, you can use the `add_definitions` function:

```cmake
add_definitions(-DULOG_NO_COLOR)
```

For Meson projects, you can use the `meson` command.

```meson
add_global_arguments('-DULOG_NO_COLOR', language: 'c')
```

Note: You might want to adjust the compiler argument  `-fmacro-prefix-map=OLD_PATH=NEW_PATH` to to get the right file paths, e.g. for meson:

```meson
add_global_arguments('-fmacro-prefix-map=../=',language: 'c')
```

### Log Verbosity

The current logging level can be set by using the `ulog_set_level()` function.
All logs below the given level will not be written to `stderr`. By default the
level is `LOG_TRACE`, such that nothing is ignored.

```c
ulog_set_level(LOG_INFO);
```

To get the name of the log level use `ulog_get_level_string`:

```c
const char *level = ulog_get_level_string(LOG_INFO);
ptrintf("Level: %s\n", level);
```

Quiet-mode can be enabled by passing `true` to the `ulog_set_quiet()` function.
While this mode is enabled the library will not output anything to `stderr`, but will continue to write to files and callbacks if any are set.

```c
ulog_set_quiet(true);
```

These functions, and others that modify the logger's behavior, interact with a global configuration structure. This structure is initialized automatically on the first logging call or can be set up explicitly.

### Thread-safety

If the log will be written to from multiple threads a lock function can be set. To do this use the `ulog_set_lock()` function.
The function is passed the boolean `true` if the lock should be acquired or `false` if the lock should be released and the given `udata` value.

```c

void lock_function(bool lock, void *lock_arg) {
    if (lock) {
        pthread_mutex_lock((pthread_mutex_t *) lock_arg);
    } else {
        pthread_mutex_unlock((pthread_mutex_t *) lock_arg);
    }
}

. . .

pthread_mutex_t mutex;
ulog_set_lock(lock_function, mutex);
```

### Log Topics

The availability of the topic logging feature is controlled by the compile-time define `ULOG_TOPICS_NUM`.
- If `ULOG_TOPICS_NUM` is defined as `0` or is not defined, topic logging is disabled.
- If `ULOG_TOPICS_NUM > 0`, static allocation is used for up to `ULOG_TOPICS_NUM` topics.
- If `ULOG_TOPICS_NUM == -1`, dynamic allocation is used for topics, allowing an unlimited number of topics (memory permitting).

The runtime behavior (e.g., whether topics are actually used, how many can be added if dynamic) is further controlled by the `ulog_config_t` structure. See [Runtime Configuration](#runtime-configuration).

Printing the log message with the topic is done by the set of function-like macros similar to log_xxx, but with the topic as the first argument:

```c
logt_trace(const char *topic_name, const char *fmt, ...)
logt_debug(const char *topic_name, const char *fmt, ...)
logt_info(const char *topic_name, const char *fmt, ...) 
logt_warn(const char *topic_name, const char *fmt, ...) 
logt_error(const char *topic_name, const char *fmt, ...)
logt_fatal(const char *topic_name, const char *fmt, ...)
```

In static mode you can decide whether enable or disable the topic during its definition. In dynamic mode all topics are disabled by default.

For example:

```c
// Static topics

ulog_add_topic("network", true); // enabled by default
ulog_add_topic("storage", false); // disabled by default

logt_info("network", "Connected to server");

ulog_enable_topic("storage");
logt_warn("storage", "No free space");

. . .

// Dynamic topics

// by default all topics are disabled
ulog_enable_topic("storage");
logt_error("storage", "No free space");

ulog_enable_all_topics(); 
logt_trace("network", "Disconnected from server");
logt_fatal("video", "No signal");
```

By default, the logging level of each topic is set to `LOG_TRACE`. It is possible to alter this behavior by calling `ulog_set_topic_level()`. All topics below the level set by `ulog_set_level()` (`LOG_TRACE` by default) will not generate log.

For example:

```c
// By default, both topic logging levels are set to LOG_TRACE
ulog_add_topic("network", true);
ulog_add_topic("storage", true);

// Both topics generate log as global logging level is set to LOG_TRACE
logt_info("network", "Connected to server");
logt_warn("storage", "No free space");

ulog_set_level(LOG_INFO);
ulog_set_topic_level("storage", LOG_WARN);

// Only "storage" topic generates log
logt_info("network", "Connected to server");
logt_info("storage", "No free space");
```

### Extra Outputs

The maximum number of extra logging outputs is controlled by the compile-time define `ULOG_EXTRA_OUTPUTS`.
- If `ULOG_EXTRA_OUTPUTS` is `0` or not defined, this feature is disabled.
- If `ULOG_EXTRA_OUTPUTS > 0`, up to that many extra outputs (file pointers or callbacks) can be added.

The actual number of active extra outputs is set at runtime via `ulog_config_t`. See [Runtime Configuration](#runtime-configuration).

#### File Output

One or more file pointers where the log will be written can be provided to the library by using the `ulog_add_fp()` function. The data written to the file output is of the following format (with the full time stamp):

```txt
2047-03-11 20:18:26 TRACE src/main.c:11: Hello world
```

Any messages below the given `level` are ignored. If the library failed to add a
file pointer a value less-than-zero is returned.

```c
FILE *fp = fopen("log.txt", "w");
if (fp) {
    ulog_add_fp(fp, LOG_INFO);
}
```

#### Custom Output

One or more callback functions which are called with the log data can be provided to the library by using the `ulog_add_callback()` function. Yo ucan use `ulog_event_to_cstr` to convert the `ulog_Event` structure to a string.

```c
void arduino_callback(ulog_Event *ev, void *arg) {
    static char buffer[128];
    int result = ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    if (result == 0) {
        Serial.println(buffer);
    }
}

. . .

ulog_add_callback(arduino_callback, NULL, LOG_INFO);
```

### Custom Log Prefix

Sets a custom prefix function. The function is called with the log level and should return a string that will be printed right before the log level. It can be used to add custom data to the log messages, e.g. millisecond time.

The maximum size of the custom prefix string is set by `ULOG_CUSTOM_PREFIX_SIZE` at compile-time.
- If `ULOG_CUSTOM_PREFIX_SIZE` is `0` or not defined, this feature is disabled.
- If `ULOG_CUSTOM_PREFIX_SIZE > 0`, a prefix of up to this size can be generated.

Whether a custom prefix is used and its maximum runtime size (up to the compile-time limit) is configured via `ulog_config_t`. See [Runtime Configuration](#runtime-configuration).

```c
void update_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    snprintf(prefix, prefix_size, ", %d ms", millis());
}

. . .

ulog_set_prefix_fn(prefix_fn);

```

The output will be:

```txt
19:51:42, 105 ms ERROR src/main.c:38: Error message
````

### Timestamp

The availability of timestamping is controlled by the `ULOG_HAVE_TIME` compile-time flag. If not defined, timestamp functionality is compiled out.
If defined, whether timestamps are actually included in log messages is controlled at runtime by `g_ulog_config.time_enabled`. See [Runtime Configuration](#runtime-configuration).

The time to the file output will be written with the date, while time to the console will be written with the time only.

```txt
log.txt:

2021-03-11 20:18:26 TRACE src/main.c:11: Hello world

console:

20:18:26 TRACE src/main.c:11: Hello world
```

### Other Customization

The following defines can be used to customize the library's output:

- `ULOG_NO_COLOR`: If defined, ANSI color escape codes are completely compiled out. Otherwise, color can be enabled/disabled at runtime.
- `ULOG_HIDE_FILE_STRING`: If defined, file/line information is completely compiled out. Otherwise, it can be enabled/disabled at runtime.
- `ULOG_SHORT_LEVEL_STRINGS`: If defined, sets the default to use short level strings (e.g., "T", "I"). This can be changed at runtime.
- `ULOG_USE_EMOJI`: If defined, sets the default to use emojis for log levels. This overrides `ULOG_SHORT_LEVEL_STRINGS` by default and can be changed at runtime. (Note: Requires terminal and font support for emojis).

These compile-time flags primarily set the *default* behavior or *availability* of features. Most can be further configured at runtime.

## Runtime Configuration

microlog's behavior is primarily controlled at runtime through the `ulog_config_t` structure and the `ulog_init()` function.

### `ulog_config_t` Structure

This structure holds all runtime configurable options:

```c
typedef struct {
    bool time_enabled;          // Enable/disable timestamps (if ULOG_HAVE_TIME is defined)
    bool color_enabled;         // Enable/disable colored output (if ULOG_NO_COLOR is not defined)
    int  custom_prefix_size;    // Max size for custom prefix (if ULOG_CUSTOM_PREFIX_SIZE > 0)
    bool file_string_enabled;   // Enable/disable file:line prefix (if ULOG_HIDE_FILE_STRING is not defined)
    bool short_level_strings;   // Use short "T" or long "TRACE" level strings
    bool emoji_levels;          // Use emoji for levels (overrides short_level_strings if true)
    int  extra_outputs;         // Number of active extra outputs (up to ULOG_EXTRA_OUTPUTS)
    int  topics_num;            // Number of topics or -1 for dynamic (up to ULOG_TOPICS_NUM for static)
    bool topics_dynamic_alloc;  // True if topics are dynamically allocated (ULOG_TOPICS_NUM = -1)
    int  level;                 // Current global log verbosity level (LOG_TRACE, LOG_DEBUG, etc.)
    bool quiet;                 // Suppress output to stdout/stderr
} ulog_config_t;
```

### `ulog_init()` Function

```c
void ulog_init(const ulog_config_t *config);
```

- **Purpose**: Initializes the logger with a specific configuration. It's automatically called with `NULL` (default configuration) on the first logging attempt if not explicitly called by the user beforehand.
- **Parameters**:
    - `config`: A pointer to a `ulog_config_t` structure.
        - If `NULL`, `g_ulog_config` is initialized with default values, which are then overridden by compile-time flags (e.g., `ULOG_HAVE_TIME` enables time, `ULOG_NO_COLOR` disables color).
        - If not `NULL`, the provided configuration is copied to `g_ulog_config`. After copying, restrictive compile-time flags are re-applied (e.g., if `ULOG_NO_COLOR` is defined, `g_ulog_config.color_enabled` will be forced to `false` even if the user provided `true`). Similarly, sized features like `custom_prefix_size` cannot exceed their compile-time limits.

### Interaction of Compile-Time Flags and Runtime Configuration

1.  **Base Defaults**: The library has internal base default values for all settings (e.g., color enabled, time disabled, log level TRACE).
2.  **Compile-Time Flags**: These modify the base defaults when `ulog_init(NULL)` is called (either automatically or by the user).
    - Some flags *disable* a feature entirely (e.g., `ULOG_NO_COLOR` removes color code, `ULOG_HAVE_TIME` not being defined removes timestamp code). If a feature is compiled out, it cannot be enabled at runtime.
    - Other flags set the *initial state* or *maximum capacity* (e.g., `ULOG_SHORT_LEVEL_STRINGS` sets the default level string format, `ULOG_EXTRA_OUTPUTS` sets the max number of callbacks).
3.  **User-Supplied `ulog_config_t` (via `ulog_init`)**: If the user calls `ulog_init()` with their own configuration, these settings are applied. However, they are still constrained by compile-time flags (e.g., trying to enable color via `ulog_config_t` will have no effect if `ULOG_NO_COLOR` was defined).
4.  **Setter Functions**: Functions like `ulog_set_level()`, `ulog_set_quiet()` can modify the active `g_ulog_config` *after* initialization. They also trigger default initialization if `ulog_init` hasn't been called yet.

**Example of `ulog_init`:**

```c
#include "ulog.h"

int main() {
    // Option 1: Automatic default initialization (influenced by compile-time flags)
    log_info("This uses default/compile-time settings.");

    // Option 2: Explicit default initialization
    ulog_init(NULL); 
    log_info("Still default/compile-time settings.");

    // Option 3: Custom initialization
    ulog_config_t my_config;
    my_config.level = LOG_DEBUG;
    my_config.color_enabled = false; // Attempt to disable color
#ifdef ULOG_HAVE_TIME
    my_config.time_enabled = true;   // Attempt to enable time
#else
    my_config.time_enabled = false;
#endif
    // ... set other fields ...
    my_config.custom_prefix_size = 0; // Disable custom prefix for this example
    my_config.file_string_enabled = true;
    my_config.short_level_strings = true;
    my_config.emoji_levels = false;
    my_config.extra_outputs = 0;
#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM == -1 // If compiled for dynamic topics
    my_config.topics_num = -1; 
    my_config.topics_dynamic_alloc = true;
#else // static or disabled
    my_config.topics_num = 0;
    my_config.topics_dynamic_alloc = false;
#endif
    my_config.quiet = false;

    ulog_init(&my_config);

    log_debug("This uses custom settings (DEBUG level, no color if not forced by ULOG_NO_COLOR, time if ULOG_HAVE_TIME).");

    // Further changes with setters
    ulog_set_level(LOG_INFO);
    log_info("Log level changed to INFO.");

    return 0;
}
```

## Contributing

Contributions are welcome! The library design is described in [design.md](doc/design.md).

If you want to contribute feel free to open an issue or create Pull Request with your changes.

## License

This library is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](LICENSE) for details.

## Credits

Based on <https://github.com/rxi/log.c.git>
