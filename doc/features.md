# Features

- [Features](#features)
    - [Core Features](#core-features)
        - [Static Configuration](#static-configuration)
        - [Logging, Levels and Outputs](#logging-levels-and-outputs)
        - [Events](#events)
        - [Lock](#lock)
        - [Cleanup](#cleanup)
    - [Optional Features](#optional-features)
        - [Disable](#disable)
        - [Configuration Header](#configuration-header)
        - [Topics](#topics)
        - [Extra Outputs](#extra-outputs)
            - [File Output](#file-output)
            - [User Defined Output](#user-defined-output)
        - [Prefix](#prefix)
        - [Time](#time)
        - [Color](#color)
        - [Source Location](#source-location)
        - [Level Style](#level-style)
        - [Dynamic Configuration](#dynamic-configuration)
            - [Topics Configuration](#topics-configuration)
            - [Prefix Configuration](#prefix-configuration)
            - [Time Configuration](#time-configuration)
            - [Color Configuration](#color-configuration)
            - [Source Location Configuration](#source-location-configuration)
            - [Level Configuration](#level-configuration)

This document describes the features of the logging library. There are optional and core features.

**Core Features** - are mandatory features requred by the library for the normal operation:

- **Print** - provides formatted printing to streams and buffers (not exposed to the users and used by other features internally)
- **Outputs** - printing endpoints logic and stdout output
- **Levels** - severity filters per **output**
- **Events** - containers to distribute logging info across **outputs**
- **Lock** - logic to inject external thread-safety mechanism
- **Logging** - logic that generates an **event** and dispatch it to **outputs**
- **Static Configuration** - compile-time logic to enable/disable optional features and configure core features

**Optional Features** - are configurable optional features that can extend the core library capabilities:

- **Color** - add ANSI colors to the output
- **Time** - add time stamps
- **Prefix** - add custom data after the time stamp
- **Extra Outputs** - additional user-defined outputs, including files
- **Source Location** - prints `file:line` location of a logging call
- **Level Style** - full or short severity level name
- **Topics** - label based message filtering
- **Dynamic Configuration** - run-time configuration of all features
- **Warnings Stubs for Non-Enabled Features** - generate stubs for disabled features with warning message or just fail linking if the function is disabled.

## Core Features

### Static Configuration

Part of features are configured compile-time. You can use defines in the compiler options, e.g. `-DULOG_BUILD_COLOR=1`.

For CMake projects, you can use the `add_compile_definitions` function.

```cmake
target_compile_definitions(microlog PRIVATE ULOG_BUILD_COLOR=1)
```

For Meson projects, you can use the `meson` command.

```meson
add_global_arguments('-DULOG_BUILD_COLOR=1', language: 'c')
```

Note: For meson, you might want to adjust the compiler argument  `-fmacro-prefix-map=OLD_PATH=NEW_PATH` to to get the right file paths, e.g. for meson:

```meson
add_global_arguments('-fmacro-prefix-map=../=',language: 'c')
```

See also: [Configuration Header](#configuration-header) feature.

The full list of build options for static configuration is shown bellow:

| Build Option                     | Default                    | Purpose                                 |
| -------------------------------- | -------------------------- | --------------------------------------- |
| ULOG_BUILD_COLOR                 | 0                          | Compile color code paths                |
| ULOG_BUILD_PREFIX_SIZE           | 0                          | Prefix buffer logic                     |
| ULOG_BUILD_EXTRA_OUTPUTS         | 0                          | Extra output backends                   |
| ULOG_BUILD_SOURCE_LOCATION       | 1                          | File\:line output                       |
| ULOG_BUILD_LEVEL_SHORT           | 0                          | Print levels with short names, e.g. 'E' |
| ULOG_BUILD_TIME                  | 0                          | Timestamp support                       |
| ULOG_BUILD_TOPICS_MODE           | ULOG_BUILD_TOPICS_MODE_OFF | Topic allocation mode                   |
| ULOG_BUILD_TOPICS_STATIC_NUM     | 0                          | Number of static topics (0 = disabled)  |
| ULOG_BUILD_DYNAMIC_CONFIG        | 0                          | Runtime toggles                         |
| ULOG_BUILD_WARN_NOT_ENABLED      | 1                          | Warning stubs                           |
| ULOG_BUILD_CONFIG_HEADER_ENABLED | 0                          | Use external configuration header       |
| ULOG_BUILD_CONFIG_HEADER_NAME    | "ulog_config.h"            | Configuration header name               |

### Logging, Levels and Outputs

There are 8 log severity levels (by ascending severity): `ULOG_LEVEL_0` ... `ULOG_LEVEL7`. To log a message there is a general macro:

```c
ulog(ulog_level level, const char *fmt, ...);
```

By default these generic levels are aliased with:

- `ULOG_LEVEL_TRACE` - for tracing the execution path (`ULOG_LEVEL_0`)
- `ULOG_LEVEL_DEBUG` - for debug information (`ULOG_LEVEL_1`)
- `ULOG_LEVEL_INFO` - for general information (`ULOG_LEVEL_2`)
- `ULOG_LEVEL_WARN` - for important information (`ULOG_LEVEL_3`)
- `ULOG_LEVEL_ERROR` - for information about recoverable errors (`ULOG_LEVEL_4`)
- `ULOG_LEVEL_FATAL` - for information about condition causing the system failure (`ULOG_LEVEL_5`)
- Leveles `ULOG_LEVEL_6` and `ULOG_LEVEL_7` are not used by default

The library provides also default level macros for logging:

```c
ulog_trace(const char *fmt, ...);
ulog_debug(const char *fmt, ...);
ulog_info(const char *fmt, ...);
ulog_warn(const char *fmt, ...);
ulog_error(const char *fmt, ...);
ulog_fatal(const char *fmt, ...);
```

Each function takes a printf format string followed by additional arguments:

```c
ulog_info("Info message %f", 3.0)
```

The user can also define custom levels by using the `ulog_level_set_new_levels(ulog_level_descriptor *levels)` function. The default levels can be restored by calling `ulog_level_reset_levels()`. E.g.

```cpp
static ulog_level_descriptor syslog_levels = {
    .max_level = ULOG_LEVEL_7,  // allow 0..7
    .names     = {"DEBUG", "INFO", "NOTICE", "WARN", "ERR", "CRIT", "ALERT", "EMERG"},
};

#define LOG_NOTICE  ULOG_LEVEL_2
// ...
ulog_level_set_new_levels(&syslog_levels);
ulog(LOG_NOTICE, "This is a notice message");
// Output: NOTICE src/main.c:12: This is a notice message
ulog_level_reset_levels();

ulog(ULOG_LEVEL_2, "This is a default level message");
// Output: INFO src/main.c:14: This is a default level message

```

The default log level is `ULOG_LEVEL_TRACE`, such that nothing is ignored. And by default there is only one available output - **stdout**. To configure its severity the user can use these two functions:

```c
ulog_output_level_set(ULOG_OUTPUT_STDOUT, ULOG_LEVEL_INFO);

// or

ulog_output_level_set_all(ULOG_LEVEL_TRACE);
```

In this case the stdout-printed line will be:

```txt
INFO  src/main.c:66: Info message 3.000000
```

### Events

The events care information depending on the static configuration. The whole list of possible data:

- Message
- Message format arguments
- Topic
- Time
- File
- Line
- Level

The data is accessible via getters (see header file for details):

- `ulog_event_get_message(...)`
- `ulog_event_get_topic(...)`
- `ulog_event_get_time(...)`
- `ulog_event_get_file(...)`
- `ulog_event_get_line(...)`
- `ulog_event_get_level(...)`

### Lock

If the log will be written to from multiple threads a lock function can be set. To do this use the `ulog_lock_set_fn()` function.

The lock function must match the `ulog_lock_fn` type and return non-ULOG_STATUS_OK value on error:

```c
typedef ulog_status (*ulog_lock_fn)(bool lock, void *udata);
```

The function is passed the boolean `true` if the lock should be acquired or `false` if the lock should be released and the given `udata` value.

```c

ulog_status lock_function(bool lock, void *lock_arg) {
    pthread_mutex_t *mutex = (pthread_mutex_t *) lock_arg; // retrieve the mutex
    int result = -1;
    if (lock) {
        result = pthread_mutex_lock(mutex);
    } else {
        result = pthread_mutex_unlock(mutex);
    }
    return (result == 0) ? ULOG_STATUS_OK : ULOG_STATUS_ERROR;
}

. . .

pthread_mutex_t mutex; // We pass the mutex as the lock_arg
pthread_mutex_init(&mutex, NULL);
. . .
ulog_lock_set_fn(lock_function, mutex);

```

For platform-specific convenience helpers (pthread, Windows, FreeRTOS, ThreadX, Zephyr, CMSIS‑RTOS2, macOS unfair lock) and the syslog level extension, see `extensions/README.md`.

### Cleanup

If the library is used with dynamic features (e.g. topics, extra outputs, dynamic configuration) it is recommended to call the `ulog_cleanup()` function before program exit to free all allocated resources.

```c
. . .
ulog_cleanup();
```

The clean up can be also used to remove all topics and outputs if needed during the program execution even if the allocation mode is static.

## Optional Features

### Disable

- Static configuration options: `ULOG_BUILD_DISABLED`
- Values (bool): `0/1`
- Default: `0`.

This feature allow to disable all logging calls at compile time. To do this define `ULOG_BUILD_DISABLED=1` in the compiler options.

When the feature is enabled all logging macros are replaced with empty stubs or return negative status codes. E.g.

| Function                    | Return Value When Disabled |
| --------------------------- | -------------------------- |
| ulog_cleanup                | `ULOG_STATUS_DISABLED`     |
| ulog_color_config           | `ULOG_STATUS_DISABLED`     |
| ulog_event_get_file         | `""`                       |
| ulog_event_get_level        | `ULOG_LEVEL_0`             |
| ulog_event_get_line         | `-1`                       |
| ulog_event_get_message      | `ULOG_STATUS_DISABLED`     |
| ulog_event_get_time         | `NULL`                     |
| ulog_event_get_topic        | `ULOG_TOPIC_ID_INVALID`    |
| ulog_event_to_cstr          | `ULOG_STATUS_DISABLED`     |
| ulog_level_config           | `ULOG_STATUS_DISABLED`     |
| ulog_level_reset_levels     | `ULOG_STATUS_DISABLED`     |
| ulog_level_set_new_levels   | `ULOG_STATUS_DISABLED`     |
| ulog_level_to_string        | `"?"`                      |
| ulog_lock_set_fn            | `ULOG_STATUS_DISABLED`     |
| ulog_log                    | `(void)0`                  |
| ulog_output_add             | `ULOG_OUTPUT_INVALID`     |
| ulog_output_add_file        | `ULOG_OUTPUT_INVALID`     |
| ulog_output_level_set       | `ULOG_STATUS_DISABLED`     |
| ulog_output_level_set_all   | `ULOG_STATUS_DISABLED`     |
| ulog_output_remove          | `ULOG_STATUS_DISABLED`     |
| ulog_prefix_config          | `ULOG_STATUS_DISABLED`     |
| ulog_prefix_set_fn          | `ULOG_STATUS_DISABLED`     |
| ulog_source_location_config | `ULOG_STATUS_DISABLED`     |
| ulog_time_config            | `ULOG_STATUS_DISABLED`     |
| ulog_topic_add              | `ULOG_TOPIC_ID_INVALID`    |
| ulog_topic_config           | `ULOG_STATUS_DISABLED`     |
| ulog_topic_get_id           | `ULOG_TOPIC_ID_INVALID`    |
| ulog_topic_level_set        | `ULOG_STATUS_DISABLED`     |
| ulog_topic_remove           | `ULOG_STATUS_DISABLED`     |

### Configuration Header

- Static configuration options: `ULOG_BUILD_CONFIG_HEADER_ENABLED`, `ULOG_BUILD_CONFIG_HEADER_NAME`
- Values (bool, string): `0/1`, `ANY`
- Default: `0`, `"ulog_config.h"`

As an alternative to defining build options individually via compiler flags, you can define `ULOG_BUILD_CONFIG_HEADER_ENABLED=1` to include a single header file named `ulog_config.h` that contains all configuration options. This approach simplifies configuration management by centralizing all build options in one file.

When `ULOG_BUILD_CONFIG_HEADER_ENABLED` is defined:

- The library will include `ulog_config.h`
  - You can overwrite it with `ULOG_BUILD_CONFIG_HEADER_NAME="my_ulog_conf.h"`
- All other `ULOG_BUILD_*` macros must be defined in this header file
- Defining other `ULOG_BUILD_*` macros via compiler flags will cause a compilation error

Example `ulog_config.h`:

```c
#pragma once

// Define all build options in one place
#define ULOG_BUILD_COLOR 1
#define ULOG_BUILD_PREFIX_SIZE 64
#define ULOG_BUILD_EXTRA_OUTPUTS 8
#define ULOG_BUILD_SOURCE_LOCATION 1
#define ULOG_BUILD_LEVEL_SHORT 0
#define ULOG_BUILD_TIME 1
#define ULOG_BUILD_TOPICS_MODE ULOG_BUILD_TOPICS_MODE_STATIC
#define ULOG_BUILD_TOPICS_STATIC_NUM 10
#define ULOG_BUILD_DYNAMIC_CONFIG 0
#define ULOG_BUILD_WARN_NOT_ENABLED 1

```

Usage with CMake:

```cmake
target_compile_definitions(microlog PRIVATE ULOG_BUILD_CONFIG_HEADER_ENABLED=1)
target_include_directories(microlog PRIVATE path/to/config/directory)
```

This approach is particularly useful when you have multiple configurations or want to keep configuration separate from build scripts.

### Topics

 - Static configuration options: `ULOG_BUILD_TOPICS_MODE`, `ULOG_BUILD_TOPICS_STATIC_NUM`
- Values (enum, int): `ULOG_BUILD_TOPICS_MODE_OFF`, `ULOG_BUILD_TOPICS_MODE_STATIC`, `ULOG_BUILD_TOPICS_MODE_DYNAMIC`, `0...UINT_MAX`
- Default: `ULOG_BUILD_TOPICS_MODE_OFF`, `0`.

The feature is controlled by `ULOG_BUILD_TOPICS_MODE`. It allows to filter log messages by subsystems, e.g. "network", "storage", etc. Use `ULOG_BUILD_TOPICS_MODE_STATIC` with `ULOG_BUILD_TOPICS_STATIC_NUM` for a fixed number of topics, or `ULOG_BUILD_TOPICS_MODE_DYNAMIC` for runtime allocation.

There are two mechanism of working with the topics:

- **Dynamic** allocation - slightly slower than static allocation
- **Static** allocation - faster

If you want to use dynamic topics, set `ULOG_BUILD_TOPICS_MODE` to `ULOG_BUILD_TOPICS_MODE_DYNAMIC`. For static allocation set `ULOG_BUILD_TOPICS_MODE` to `ULOG_BUILD_TOPICS_MODE_STATIC` and define `ULOG_BUILD_TOPICS_STATIC_NUM` to the desired number of topics.

Printing the log message with the topic is done by the set of function-like macros similar to `ulog_xxx`, but with the topic as the first argument:

```c
ulog_topic_log(ulog_level level, const char *topic_name, const char *fmt, ...);
ulog_topic_trace(const char *topic_name, const char *fmt, ...);
ulog_topic_debug(const char *topic_name, const char *fmt, ...);
ulog_topic_info(const char *topic_name, const char *fmt, ...);
ulog_topic_warn(const char *topic_name, const char *fmt, ...);
ulog_topic_error(const char *topic_name, const char *fmt, ...);
ulog_topic_fatal(const char *topic_name, const char *fmt, ...);
// or short versions:
ulog_t(ulog_level level, const char *topic_name, const char *fmt, ...);
ulog_t_trace(const char *topic_name, const char *fmt, ...);
ulog_t_debug(const char *topic_name, const char *fmt, ...);
ulog_t_info(const char *topic_name, const char *fmt, ...);
ulog_t_warn(const char *topic_name, const char *fmt, ...);
ulog_t_error(const char *topic_name, const char *fmt, ...);
ulog_t_fatal(const char *topic_name, const char *fmt, ...);

```

All topic must be added before usage via `ulog_topic_add`. At this step it is possible to select particular output for the topic (e.g. print topic `Credentials` only to a local file, or `Operator Info` only to `ULOG_OUTPUT_STDOUT`)

For example (static topics):

```c
ulog_topic_add("network", ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE); // Added and initialized to TRACE level
ulog_topic_add("storage", ULOG_OUTPUT_ALL, ULOG_LEVEL_INFO); // Added and initialized to INFO level

ulog_topic_info("network", "Connected to server");

ulog_topic_warn("storage", "No free space");
```

or dynamic topics:

```c
// by default all topics are disabled
ulog_topic_enable("storage");
ulog_topic_error("storage", "No free space");

ulog_topic_enable_all();
ulog_topic_trace("network", "Disconnected from server");
ulog_topic_fatal("video", "No signal");
```

The logging level of each topic is set to the value given as parameter. It is possible to alter this behavior by calling `ulog_topic_level_set()`. All topics below the level set by `ulog_output_level_set()` will not generate log.

For example:

```c
// Both topic logging levels are set to ULOG_LEVEL_TRACE
ulog_topic_add("network", ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);
ulog_topic_add("storage", ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);

// Both topics generate log as global logging level is set to ULOG_LEVEL_TRACE
ulog_topic_info("network", "Connected to server");
ulog_topic_warn("storage", "No free space");

ulog_output_level_set_all(ULOG_LEVEL_INFO); // All outputs are set to INFO
ulog_topic_level_set("storage", ULOG_LEVEL_WARN); // Storage is set to WARN

ulog_topic_info("storage", "No free space"); // generated
ulog_topic_info("network", "Connected to server"); // filtered out topic
ulog_topic_debug("storage", "No free space"); // filtered out level DEBUG < INFO
```

Topics can be removed by using the `ulog_topic_remove()` function.

### Extra Outputs

- Static configuration options: `ULOG_BUILD_EXTRA_OUTPUTS`
- Values (int): `0...INT32_MAX`
- Default: `0`.

The feature is controlled by the following define:

- `ULOG_BUILD_EXTRA_OUTPUTS` - The maximum number of extra logging outputs that can be added. Each extra output requires some memory. When it is 0, the only available output is STDOUT. Default is 0.

#### File Output

One or more file pointers where the log will be written can be provided to the library by using the `ulog_output_add_file()` function. The data written to the file output is of the following format (with the full time stamp):

```txt
2047-03-11 20:18:26 TRACE src/main.c:11: Hello world

vs

20:18:26 TRACE src/main.c:11: Hello world
```

To write to a file open a file and pass it to the `ulog_output_add_file` function.

```c
ulog_topic_add("Outputs", ULOG_OUTPUT_ALL, ULOG_LEVEL_TRACE);
FILE *fp = fopen("log.txt", "w");
if (fp) {
    ulog_output_id file_output = ulog_output_add_file(fp, ULOG_LEVEL_INFO);
    if (file_output != ULOG_OUTPUT_INVALID) {
        ulog_topic_info("Outputs", "File output added");
        ulog_output_level_set(file_output, ULOG_LEVEL_TRACE);
        ulog_topic_trace("Outputs", "File output level set to TRACE");
    }
    ulog_output_remove(file_output);  // For demo purposes
    fclose(fp);  // For demo purposes
}
```

Outputs can be removed by using the `ulog_output_remove()` function.

#### User Defined Output

One or more output handler functions which are called with the log data can be provided to the library by using the `ulog_output_add()` function. You can use `ulog_event_to_cstr` to convert the `ulog_event` structure to a string.

```c
void arduino_output_handler(ulog_event *ev, void *arg) {
    static char buffer[128];
    int result = ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    if (result == 0) {
        Serial.println(buffer);
    }
}

. . .

ulog_output_id ard_output = ulog_output_add(arduino_output_handler, NULL, ULOG_LEVEL_INFO);
if (ard_output != ULOG_OUTPUT_INVALID) {
    ulog_info("Will be printed to Arduino serial");
    ulog_output_remove(ard_output); // For demo purposes
}
```

WARNING: The handler function is called with the lock acquired, so if you are using logging inside the handler, it may cause a deadlocks: e.g.

```c
void faulty_output_handler(ulog_event *ev, void *arg) {
    ulog_info("This might cause a deadlock or unexpected behavior");
}
```

### Prefix

- Static configuration options: `ULOG_BUILD_PREFIX_SIZE`
- Values (int): `0...INT_MAX`
- Default: `0`.

Sets a prefix function that can be used to customize the log output. The function is called with the log event and should fill a string (`prefix`) that will be printed right before the log level. It can be used to add custom data to the log messages, e.g. millisecond time.

Requires `ULOG_BUILD_PREFIX_SIZE` to be more than 0.

```c
void prefix_handler(ulog_event *ev, char *prefix, size_t prefix_size) {
    snprintf(prefix, prefix_size, ", %03d ms", millis());
}
// . . .
ulog_prefix_set_fn(prefix_handler);
```

The output will be:

```txt
19:51:42, 005 ms ERROR src/main.c:38: Error message
```

WARNING: The handler function is called with the lock acquired, so if you are using logging inside the handler, it may cause a deadlocks. E.g.:

```c
void faulty_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    ulog_info("This might cause a deadlock or unexpected behavior");
}
```

### Time

- Static configuration options: `ULOG_BUILD_TIME`
- Values (bool): `0/1`
- Default: `0`.

Prints a time stamp in from of all log messages. Your platform must support `time.h`.

The time to the file output will be written with the date, while time to the console and other outputs will be written with the time only.

```txt
log.txt:

2021-03-11 20:18:26 TRACE src/main.c:11: Hello world

console:

20:18:26 TRACE src/main.c:11: Hello world
```

### Color

- Static configuration options: `ULOG_BUILD_COLOR`
- Values (bool): `0/1`
- Default: `0`.

Use ANSI color escape codes when printing to stdout. If the terminal supports, the output will be colorized.

- TRACE - white
- DEBUG - cyan
- INFO - green
- WARN - yellow
- ERROR - red
- FATAL - red on white background
- UNUSED levels (6,7) - yellow on red and white on red background

<img src="features/colors.png" width="600">

### Source Location

- Static configuration options: `ULOG_BUILD_SOURCE_LOCATION`
- Values (bool): `0/1`
- Default: `1`.

Hide or show the file name and line number. See output examples below:

- ULOG_BUILD_SOURCE_LOCATION=0: `TRACE Hello world`
- ULOG_BUILD_SOURCE_LOCATION=1: `TRACE src/main.c:11: Hello world`

### Level Style

- Static configuration options: `ULOG_BUILD_LEVEL_SHORT`
- Values (bool): `0/1`
- Default: `0`.

Allows to use short level strings, e.g. "T" for "TRACE", "I" for "INFO":

- ULOG_BUILD_LEVEL_SHORT=0: `TRACE src/main.c:11: Hello world`
- ULOG_BUILD_LEVEL_SHORT=1: `T src/main.c:11: Hello world`

### Dynamic Configuration

- Static configuration options: `ULOG_BUILD_DYNAMIC_CONFIG`
- Values (bool): `0/1`
- Default: `0`.

Most of the library features are configured compile time to reduce the code size and complexity. However, if the code size is not a concern, you can enable Dynamic Config by defining `ULOG_BUILD_DYNAMIC_CONFIG=1`. When the feature is enables all other features are enabled too in some default mode described in bellow. All Dynamic Config functions named like: `ulog_FEATURE_config`. The default configuration is following:

| Build Config                | Default Value                  |
| --------------------------- | ------------------------------ |
| ULOG_BUILD_PREFIX_SIZE      | 64                             |
| ULOG_BUILD_EXTRA_OUTPUTS    | 8                              |
| ULOG_BUILD_TIME             | 1                              |
| ULOG_BUILD_SOURCE_LOCATION  | 1                              |
| ULOG_BUILD_COLOR            | 1                              |
| ULOG_BUILD_LEVEL_SHORT      | 0                              |
| ULOG_BUILD_TOPICS_MODE      | ULOG_BUILD_TOPICS_MODE_DYNAMIC |
| ULOG_BUILD_WARN_NOT_ENABLED | 0                              |

#### Topics Configuration

If Dynamic Config enabled topics are created runtime in the **dynamic allocation mode**.

Configuration functions:

- `ulog_status ulog_topic_config(bool enabled)` - show or hide topics in the log output when using `ulog_topic_xxx` macros. Returns `ULOG_STATUS_OK` on success, `ULOG_STATUS_BUSY` if the logger is currently locked, or `ULOG_STATUS_ERROR` if the feature is not compiled in.

Example output with and without topics:

```c
ulog_topic_info("WORLD", "Hello");
```

- enabled=true:  `INFO  [WORLD] src/main.c:13: Hello`
- enabled=false: `INFO  src/main.c:13: Hello`

#### Prefix Configuration

If Dynamic Config enabled, `ULOG_BUILD_PREFIX_SIZE` is set to 64, so the prefix will be limited to: 63 characters + 1 null terminator.

Functions to configure the prefix:

- `ulog_status ulog_prefix_config(bool enabled)` - enable or disable prefix in the log output. Returns `ULOG_STATUS_OK` / `ULOG_STATUS_BUSY` / `ULOG_STATUS_ERROR` (feature disabled).

#### Time Configuration

Functions to configure the timestamp:

- `ulog_status ulog_time_config(bool enabled)` - enable or disable time in the log output. Returns `ULOG_STATUS_OK` / `ULOG_STATUS_BUSY` / `ULOG_STATUS_ERROR` (feature disabled).

#### Color Configuration

Functions to configure:

- `ulog_status ulog_color_config(bool enabled)` - enable or disable ANSI color escape codes when printing to stdout. Returns `ULOG_STATUS_OK` / `ULOG_STATUS_BUSY` / `ULOG_STATUS_ERROR` (feature disabled).

#### Source Location Configuration

Functions to configure:

- `ulog_status ulog_source_location_config(bool enabled)` - show or hide file and line in the log output. Returns `ULOG_STATUS_OK` / `ULOG_STATUS_BUSY` / `ULOG_STATUS_ERROR` (feature disabled).

#### Level Configuration

Functions to configure:

- `ulog_status ulog_level_config(ulog_level_config_style style)` - enable or disable short level strings (e.g. "T" for "TRACE", "I" for "INFO"). Returns `ULOG_STATUS_OK` / `ULOG_STATUS_BUSY` / `ULOG_STATUS_ERROR` (feature disabled).

The `style` argument can be one of the following values:

- `ULOG_LEVEL_CONFIG_STYLE_DEFAULT`: long level strings (default)
- `ULOG_LEVEL_CONFIG_STYLE_SHORT`: short level strings
