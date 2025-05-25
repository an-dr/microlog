# Library Design

This document outlines the design of the library and key concepts.

## Overview

All logging is happening via `ulog_log` function (or its variants like `log_info`, `log_debug`, etc.). Topic-specific logging macros (`logt_*`) also resolve to `ulog_log`.

## Configuration

The logger's behavior is primarily controlled by a global `ulog_config_t g_ulog_config` struct. This struct is initialized by the `ulog_init()` function.

- **Initialization**:
    - If `ulog_init()` is called explicitly by the user with a `ulog_config_t` pointer, that configuration is used.
    - If `ulog_init()` is called with `NULL`, or if it's called automatically (on the first logging attempt if no prior explicit initialization occurred), it initializes `g_ulog_config` with library defaults.
- **Compile-Time Flags**: These flags influence the initial state of `g_ulog_config` during the default initialization process. For example, `ULOG_HAVE_TIME` will cause `g_ulog_config.time_enabled` to be `true` by default. Some flags can entirely compile out features (e.g., `ULOG_NO_COLOR`), in which case the corresponding `g_ulog_config` field might be present but will be forced to a non-functional state (e.g., `g_ulog_config.color_enabled = false`).
- **Setter Functions**: Functions like `ulog_set_level()` and `ulog_set_quiet()` modify fields within `g_ulog_config` after initialization. They also trigger the default initialization if the library hasn't been initialized yet.

The precedence for configuration is:
1. Library base defaults.
2. Compile-time flags modify these defaults during `ulog_init(NULL)`.
3. A user-supplied configuration via `ulog_init(&my_config)` will largely take precedence, but restrictive compile-time flags (like `ULOG_NO_COLOR`) can still override user settings.
4. Setter functions (`ulog_set_level()`, etc.) can modify the configuration at any point after initialization.

## Logging Mechanism

1.  When `ulog_log` (or a related macro) is called:
    a.  It first checks if `ulog_init()` has been run. If not, `ulog_init(NULL)` is called to perform default initialization.
    b.  It then checks the current global log level (`g_ulog_config.level`) and topic-specific levels (if topics are enabled via `g_ulog_config.topics_num` and the log call is for a specific topic). If the message doesn't meet the required level, the function may return early.
    c.  An `ulog_Event` struct is generated. This event contains all information about the logging message: the message itself, level, file, line, potential topic ID, etc.
2.  The event is passed to `process_callback(...)`. This function iterates over all registered callbacks (stdout, and any custom file or function callbacks enabled via `g_ulog_config.extra_outputs` and added via `ulog_add_fp` or `ulog_add_callback`) and calls them if the event's log level meets the callback's configured level.
3.  The standard callbacks (`callback_stdout` for console, `callback_file` for files) use `write_formatted_message(...)` to format the log message.
4.  `write_formatted_message(...)` consults the `g_ulog_config` struct to determine how to format the message (e.g., include time, color, custom prefix, file/line info).
    - For example, if `g_ulog_config.time_enabled` is true (and `ULOG_HAVE_TIME` was defined at compile time), time information will be added.
    - If `g_ulog_config.color_enabled` is true (and `ULOG_NO_COLOR` was not defined), color escape codes will be used.
5.  `write_formatted_message(...)` accepts a `ulog_Event` and a `log_target` object. The target can be a stream (like `stdout` or a file) or a character buffer (for `ulog_event_to_cstr`).
6.  The actual printing/formatting to the target is done via helper functions like `print_time_sec`, `print_level`, `print_message`, etc., which ultimately use `vsnprintf(...)` or `vfprintf(...)`.

The process is shown in the diagram below. Note that "Defines" in the diagram now primarily refer to compile-time flags that set the initial state of `g_ulog_config` or control feature availability at compile time, rather than directly controlling all logic at every step. The runtime logic heavily relies on `g_ulog_config`.

![design](design.drawio.svg)
