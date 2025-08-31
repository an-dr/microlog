# Features

- [Features](#features)
    - [Core](#core)
    - [Dynamic Config](#dynamic-config)
    - [Log Verbosity](#log-verbosity)
    - [Thread-safety](#thread-safety)
    - [Log Topics](#log-topics)
    - [Log Topics - Dynamic Config](#log-topics---dynamic-config)
    - [Outputs](#outputs)
        - [File Output](#file-output)
        - [User Defined Output](#user-defined-output)
    - [Outputs - Dynamic Config](#outputs---dynamic-config)
    - [Custom Log Prefix](#custom-log-prefix)
    - [Custom Log Prefix - Dynamic Config](#custom-log-prefix---dynamic-config)
    - [Timestamp](#timestamp)
    - [Timestamp - Dynamic Config](#timestamp---dynamic-config)
    - [Other Customization](#other-customization)
    - [Other Customization - Dynamic Config](#other-customization---dynamic-config)
    - [Feature Flags](#feature-flags)

This document describes the features of the logging library.

## Core

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

Part of features are configured compile-time. You can use defines in the compiler options, e.g. `-DULOG_BUILD_COLOR=1`.

For CMake projects, you can use the `add_compile_definitions` function.

```cmake
add_compile_definitions(ULOG_BUILD_COLOR=1)
```

For Meson projects, you can use the `meson` command.

```meson
add_global_arguments('-DULOG_BUILD_COLOR=1', language: 'c')
```

Note: You might want to adjust the compiler argument  `-fmacro-prefix-map=OLD_PATH=NEW_PATH` to to get the right file paths, e.g. for meson:

```meson
add_global_arguments('-fmacro-prefix-map=../=',language: 'c')
```

## Dynamic Config

Most of the library features are configured compile time to reduce the code size and complexity. However, if the code size is not a concern, you can enable Dynamic Config by defining `ULOG_FEATURE_DYNAMIC_CONFIG`. When the feature is enables all other features are enabled too in some default mode described in bellow. All Dynamic Config functions are prefixed with `ulog_configure_*`. The default configuration is following:

| Feature       | Default Configuration              |
| ------------- | ---------------------------------- |
| Prefix        | ☑️ visible, ULOG_BUILD_PREFIX_SIZE: 64 |
| Extra Outputs | ULOG_BUILD_EXTRA_OUTPUTS : 8             |
| Time          | ☑️ visible                       |
| File String   | ☑️ visible                       |
| Color         | ☑️ visible                       |
| Short Levels  | ⬜ disabled                        |
| Topics        | ☑️ visible, dynamic allocation   |

## Log Verbosity

The default log level is `ULOG_LEVEL_TRACE`, such that nothing is ignored.

To modify it use the `ulog_output_level_set()` function.

The example below shows how to configure log level to `ULOG_LEVEL_INFO`:

```c
ulog_output_level_set(ULOG_OUTPUT_STDOUT, ULOG_LEVEL_INFO);
```

To get the name of the log level use `ulog_level_to_string`:

```c
const char *level = ulog_level_to_string(ULOG_LEVEL_INFO);
ptrintf("Level: %s\n", level);
```

## Thread-safety

If the log will be written to from multiple threads a lock function can be set. To do this use the `ulog_lock_set_fn()` function.
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
ulog_lock_set_fn(lock_function, mutex);
```

## Log Topics

The feature is controlled by `ULOG_BUILD_TOPICS_NUM`. It allows to filter log messages by subsystems, e.g. "network", "storage", etc.

There are two mechanism of working with the topics:

- `dynamic` - slower, but new topic will be added automatically
- `static` - faster, but you need to define all topic using `ulog_topic_add`

If you want to use dynamic topics, you need to define `ULOG_BUILD_TOPICS_NUM` to be -1. Otherwise, you need to define the number of topics for static allocation.

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

ulog_topic_add("network", true); // enabled by default
ulog_topic_add("storage", false); // disabled by default

logt_info("network", "Connected to server");

ulog_topic_enable("storage");
logt_warn("storage", "No free space");

. . .

// Dynamic topics

// by default all topics are disabled
ulog_topic_enable("storage");
logt_error("storage", "No free space");

ulog_topic_enable_all(); 
logt_trace("network", "Disconnected from server");
logt_fatal("video", "No signal");
```

By default, the logging level of each topic is set to `ULOG_LEVEL_TRACE`. It is possible to alter this behavior by calling `ulog_topic_level_set()`. All topics below the level set by `ulog_output_level_set()` (`ULOG_LEVEL_TRACE` by default) will not generate log.

For example:

```c
// By default, both topic logging levels are set to ULOG_LEVEL_TRACE
ulog_topic_add("network", true);
ulog_topic_add("storage", true);

// Both topics generate log as global logging level is set to ULOG_LEVEL_TRACE
logt_info("network", "Connected to server");
logt_warn("storage", "No free space");

ulog_output_level_set(ULOG_LEVEL_INFO);
ulog_topic_level_set("storage", ULOG_LEVEL_WARN);

// Only "storage" topic generates log
logt_info("network", "Connected to server");
logt_info("storage", "No free space");
```

## Log Topics - Dynamic Config

If Dynamic Config enabled topics are created runtime in the **dynamic allocation mode**.

Configuration functions:

- `void ulog_topic_config(bool enabled)` - will show or hide topics in the log output when printing using `logt_xxx` macros.

## Outputs

The feature is controlled by the following define:

- `ULOG_BUILD_EXTRA_OUTPUTS` - The maximum number of extra logging outputs that can be added. Each extra output requires some memory. When it is 0, the entire extra output code for `ulog_output_add_file()` and `ulog_output_add()` is not compiled. Default is 0.

### File Output

One or more file pointers where the log will be written can be provided to the library by using the `ulog_output_add_file()` function. The data written to the file output is of the following format (with the full time stamp):

```txt
2047-03-11 20:18:26 TRACE src/main.c:11: Hello world
```

Any messages below the given `level` are ignored. If the library failed to add a
file pointer a value less-than-zero is returned.

```c
FILE *fp = fopen("log.txt", "w");
if (fp) {
    ulog_output_add_file(fp, ULOG_LEVEL_INFO);
}
```

### User Defined Output

One or more callback functions which are called with the log data can be provided to the library by using the `ulog_output_add()` function. You can use `ulog_event_to_cstr` to convert the `ulog_event` structure to a string.

```c
void arduino_callback(ulog_event *ev, void *arg) {
    static char buffer[128];
    int result = ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    if (result == 0) {
        Serial.println(buffer);
    }
}

. . .

ulog_output_add(arduino_callback, NULL, ULOG_LEVEL_INFO);
```

## Outputs - Dynamic Config

In the Dynamic Config mode, `ULOG_BUILD_EXTRA_OUTPUTS` is set to 8.

## Custom Log Prefix

Sets a prefix function. The function is called with the log level and should return a string that will be printed right before the log level. It can be used to add custom data to the log messages, e.g. millisecond time.

Requires `ULOG_BUILD_PREFIX_SIZE` to be more than 0.

```c
void update_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    snprintf(prefix, prefix_size, ", %d ms", millis());
}

. . .

ulog_prefix_set_fn(prefix_fn);

```

The output will be:

```txt
19:51:42, 105 ms ERROR src/main.c:38: Error message
```

## Custom Log Prefix - Dynamic Config

If Dynamic Config enabled, `ULOG_FEATURE_PREFIX_CFG_SIZE` is set to 64.

Functions to configure the prefix:

- `void ulog_prefix_config(bool enabled)` - will enable or disable prefix in the log output.

## Timestamp

The feature is controlled by `ULOG_BUILD_TIME=<0/1>`. You platform must support time.h.

The time to the file output will be written with the date, while time to the console will be written with the time only.

```txt
log.txt:

2021-03-11 20:18:26 TRACE src/main.c:11: Hello world

console:

20:18:26 TRACE src/main.c:11: Hello world
```

## Timestamp - Dynamic Config

Functions to configure the timestamp:

- `void ulog_time_config(bool enabled)` - will enable or disable time in the log output.

## Other Customization

The following defines can be used to customize the library's output:

- `ULOG_BUILD_COLOR=<0/1>` - Use ANSI color escape codes when printing to stdout.
- `ULOG_BUILD_SOURCE_LOCATION=<0/1>` - Hide or show the file name and line number.
- `ULOG_BUILD_LEVEL_STR_STYLE=<ULOG_LEVEL_STYLE_LONG/ULOG_LEVEL_STYLE_SHORT>` - Use short level strings, e.g. "T" for "TRACE", "I" for "INFO".

## Other Customization - Dynamic Config

Configuration functions:

- `void ulog_color_config(bool enabled)` - will enable or disable ANSI color escape codes when printing to stdout.
- `void ulog_source_location_config(bool enabled)` - will show or hide file and line in the log output.
- `void ulog_configure_short_level_strings(bool enabled)` - will enable or disable short level strings, e.g. "T" for "TRACE", "I" for "INFO".
- `void ulog_time_config(bool enabled)` - will enable or disable time in the log output.

## Feature Flags

All feature states can be read using the following public macros. The macros are defined based on the compilation options described above, please do not modify them directly!

| Knob                        | Type | Default                 | Allowed values                                                        | Description                                                                                                                                                  |
| --------------------------- | ---- | ----------------------- | --------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `ULOG_CFG_COLOR`            | bool | `1`                     | `0` = disabled<br /> `1` = enabled                                    | Compile color output support                                                                                                                                 |
| `ULOG_CFG_PREFIX_SIZE`      | uint | `0`                     | `0` = disabled<br /> `>0` = buffer size                               | Reserve space for log prefix                                                                                                                                 |
| `ULOG_CFG_EXTRA_OUTPUTS`    | bool | `0`                     | `0` / `1`                                                             | Enable additional output backends                                                                                                                            |
| `ULOG_CFG_SOURCE_LOCATION`  | bool | `1`                     | `0` / `1`                                                             | Include file\:line in output                                                                                                                                 |
| `ULOG_CFG_LEVEL_STR_STYLE`  | enum | `ULOG_LEVEL_STYLE_LONG` | `ULOG_LEVEL_STYLE_LONG` (0),<br />`ULOG_LEVEL_STYLE_SHORT` (1)        | Selects long or short level strings                                                                                                                          |
| `ULOG_CFG_TIME`             | bool | `0`                     | `0` / `1`                                                             | Include timestamp in log messages                                                                                                                            |
| `ULOG_CFG_TOPICS_NUM`       | uint | `0`                     | `0` = disabled,<br />`>0` = number of topics,<br />`-1` = unlimited   | Enable topic filtering with given number of slots.<br />If the value is -1 - the topics will be dynamically allocated. Also, it is a bit slower.             |
| `ULOG_CFG_DYNAMIC_CONFIG`   | bool | `0`                     | `0` / `1`                                                             | Allow runtime enable/disable of features.<br />It configures all other build arguments to specific values. <br />See the Dynamic Config section for details. |
| `ULOG_CFG_WARN_NOT_ENABLED` | bool | `1`                     | `0` / `1`                                                             | Compile warning stubs for disabled features                                                                                                                  |
