# Library Design

This document outlines the design of the library and key concepts.

## Overview

All logging is happening via `ulog_log` function. `logt_*` and `log_*` macros are using `ulog_log`.

## Logging Mechanism

1. `ulog_log` generate and event `ulog_Event`. The event contains all the info about the loggin message - level, message, file, line, function, etc.
2. The event is passed to `process_callback(...)` function. This function is responsible for iterating over all registered callbacks and calling them with the event.
3. There are 2 main internal callbacks and N user callbacks:
   - `callback_stdout(...)` - it is responsible for printing the log message to the console.
   - `callback_file(...)` - this callback is responsible for writing the log message to a file. To enable it user must use `ulog_add_fp(...)` function to register a file pointer.
   - User callbacks are configurable via `ULOG_EXTRA_OUTPUTS` define and `ulog_add_callback(...)` function. User can register any number of callbacks and they will be called in the order they were registered.
4. `callback_stdout(...)` and `callback_file(...)` will use `print_formatted_message(...)` function to format the message and print it. The function logic is configurable via a set of defines.
5. `print_formatted_message(...)` accepts a `ulog_Event` and a `log_target` objects. The target can be a stream or a buffer.
6. The actual printing is done via `vprint(...)` and `print(...)` functions accepting a `log_target` and a formatted string using a printf-like interface. It uses `vsnprintf(...)` and `vfprintf(...)` to print the message to the target.

The proces is shown in the diagram below:

![design](design/design.drawio.svg)

## Code organization

The code is organized into two files. The source files contsists of sections

- Core Functionality
- Core Functionality: Debug Levels
- Core Functionality: Logger configuration
- Core Functionality: Thread Safety
- Feature: Color
- Feature: Custom Prefix
- Feature: Extra Outputs
- Feature: Log Topics
- Feature: Log Topics - Dynamic Allocation
- Feature: Log Topics - Static Allocation
- Feature: Time
- Main Logger Object
- Output Printing
- Prototypes

Here is a diagrams shows section dependencies

![sections](design/sections.drawio.svg)
