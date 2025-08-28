# Library Design

This document outlines the design of the library and key concepts.

## Overview

All logging is happening via `ulog_log` function. `logt_*` and `log_*` macros are using `ulog_log`.

## Logging Mechanism

1. `ulog_log` generate and event `ulog_event`. The event contains all the info about the loggin message - level, message, file, line, function, etc.
2. The event is passed to `output_handle_all(...)` function. This function is responsible for iterating over all registered callbacks and calling them with the event.
3. There are 2 main internal callbacks and N user callbacks:
   - `output_stdout_callback(...)` - it is responsible for printing the log message to the console.
   - `cb_file(...)` - this callback is responsible for writing the log message to a file. To enable it user must use `ulog_output_add_file(...)` function to register a file pointer.
   - User callbacks are configurable via `ULOG_EXTRA_OUTPUTS` define and `ulog_output_add(...)` function. User can register any number of callbacks and they will be called in the order they were registered.
4. `output_stdout_callback(...)` and `cb_file(...)` will use `log_print_event(...)` function to format the message and print it. The function logic is configurable via a set of defines.
5. `log_print_event(...)` accepts a `ulog_event` and a `print_target` objects. The target can be a stream or a buffer.
6. The actual printing is done via `vprint(...)` and `print(...)` functions accepting a `print_target` and a formatted string using a printf-like interface. It uses `vsnprintf(...)` and `vfprintf(...)` to print the message to the target.

The proces is shown in the diagram below:

![design](design/design.drawio.svg)

## Code organization

The code is organized into two files. The source file consists of sections. Each section might rely on functionality of a previous section. Sections are meant to be encapsulated and not to rely on each other or some shared data - if possible. See [code.md](code.md) for more details.
