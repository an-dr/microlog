# microlog - Extensible and configurable logging library for embedded and desktop applications

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Latest](https://img.shields.io/github/v/tag/an-dr/microlog?style=flat&filter=v*&label=Release)](https://github.com/an-dr/microlog/tags)
[![Tests](https://github.com/an-dr/microlog/actions/workflows/workflow-tests.yml/badge.svg?branch=main)](https://github.com/an-dr/microlog/actions/workflows/workflow-tests.yml)

![main_pic](doc/README/diagram.drawio.svg)

- **Easy to use** - simple API, works out of the box
- **Advanced filtering** and log levels per **topic** or **output**
- **Thread-safety** via external locking injection
- **Customization** - only data you need
- **Extensible** - add your own features via public API, set of predefined extensions
- **Support for embedded systems** static allocation, small size
- **Easy to install** two files for copy-paste and support for many build systems
- **For everyone** - C and C++ support, works with any compiler, any platform, commercial or open-source use

In the default configuration it looks like this:

| `<img src="doc/README/demo0.png" width="800">`                                                         |
| ------------------------------------------------------------------------------------------------------ |
| Picture 1 - default configuration: no time, long default levels, source location, no topics, no colors |

...but in can be very minimalistic :

| `<img src="doc/README/demo1.png" width="800">`                   |
| ---------------------------------------------------------------- |
| Picture 2 - short levels, no colors, no time, no source location |

... or feature-rich:

| `<img src="doc/README/demo2.png" width="800">`                                                   |
| ------------------------------------------------------------------------------------------------ |
| Picture 3 - time, custom prefix for MsgID, custom syslog levels, topics, source location, colors |

The project is based on the following core principles:

- Universal for embedded and desktop applications
- No feature - no code for compilation
- Shallow learning curve, works out of box
- No dependencies
- Two files for core functionality.
- Extensions as recipes for your own features.

## Table of Contents

- [microlog - Extensible and configurable logging library for embedded and desktop applications](#microlog---extensible-and-configurable-logging-library-for-embedded-and-desktop-applications)
    - [Table of Contents](#table-of-contents)
    - [Quick Start](#quick-start)
        - [1. Install](#1-install)
        - [2. Use](#2-use)
        - [3. Extend](#3-extend)
    - [Advanced Usage](#advanced-usage)
    - [Contributing](#contributing)
    - [Comparison with log.c](#comparison-with-logc)
        - [Core Differences](#core-differences)
        - [Key Capabilities Unique to microlog](#key-capabilities-unique-to-microlog)
    - [Changelog](#changelog)
    - [License](#license)
    - [Credits](#credits)

## Quick Start

### 1. Install

**Option 1 - Sources**:

- Download a Source Package from [Releases](https://github.com/an-dr/microlog/releases)
- Add sources to your system manually

**Option 2 - CMake Package (recommended CMake > 3.15.0)**:

- Download a CMake Package from [Releases](https://github.com/an-dr/microlog/releases)
- Specify the install location:
  - Specify package storage `cmake -B./build -DCMAKE_PREFIX_PATH="~/MyCmakePackages"` or
  - Set `microlog_DIR` variable with path to the package `microlog_DIR=~/microlog-1.2.3-cmake`
- Use in your project:

```cmake
find_package(microlog 1.2.3 REQUIRED)

add_executable(example_package example.cpp)
target_link_libraries(example_package PRIVATE microlog::microlog)

target_compile_definitions(microlog PRIVATE ULOG_BUILD_COLOR=1) # configuration

# Or use a user-defined configuration header `ulog_config.h`:
# target_compile_definitions(microlog PRIVATE ULOG_BUILD_CONFIG_HEADER_ENABLED=1)
# target_include_directories(microlog PRIVATE path/to/directory/containing/ulog_config.h)
```

**Option 3 - Meson Package**:

- Download a Meson Package from [Releases](https://github.com/an-dr/microlog/releases)
- Copy the content to `MyMesonProject/subprojects`
- Add to your dependencies:

```meson
add_global_arguments('-DULOG_BUILD_COLOR=1', language: ['cpp', 'c']) # configuration

# Or use a user-defined configuration header `ulog_config.h`:
# add_global_arguments('-DULOG_BUILD_CONFIG_HEADER_ENABLED=1', language: ['cpp', 'c'])
# And add include directory where ulog_config.h is located:
# add_global_arguments('-Ipath/to/directory/containing/ulog_config.h', language: ['cpp', 'c'])

exe = executable(
    meson.project_name(),
    src,
    include_directories: include,
    dependencies: dependency('microlog'),
)
```

**Option 4 - Meson Wrap File**:

- Download the wrap file from [Releases](https://github.com/an-dr/microlog/releases)
- Place `microlog.wrap` in your `MyMesonProject/subprojects/` directory
- Add to your dependencies as in Option 3

**Option 5 - CPM:**

- Download CPM (https://github.com/cpm-cmake/CPM.cmake)
- Add microlog to your projects CMAKE file:

```cmake
include(cpm/CPM.cmake)
CPMAddPackage("gh:an-dr/microlog@6.4.5")

target_link_libraries(${PROJECT_NAME} PUBLIC microlog::microlog)
target_compile_definitions( microlog PRIVATE ULOG_BUILD_COLOR=1) # configuration
      
# Or use a user-defined configuration header `ulog_config.h`:
# target_compile_definitions(microlog PRIVATE ULOG_BUILD_CONFIG_HEADER_ENABLED=1)
# target_include_directories(microlog PRIVATE path/to/directory/containing/ulog_config.h)
```

### 2. Use

```cpp
#include "ulog.h"

int main() {
    ulog_info("Hello, World");
    return 0;
}
```

Output:

```log
INFO  src/main.cpp:4: Hello, World
```

### 3. Extend

Add missing functionalities via API or use predefined extensions. See [Extensions documentation](extensions/README.md).

## Advanced Usage

[User Manual in `doc/features.md`](doc/features.md) - detailed information about the available features.

[Extensions](extensions/README.md) - Optional add-ons that use only the public API to enhance functionality.

[See the example for more features in action: `example/main.cpp`](example/main.cpp).

## Contributing

Contributions are welcome! Please read the [CONTRIBUTING.md](CONTRIBUTING.md) for details, I tried to keep it simple.

## Comparison with log.c

microlog started as a fork of [rxi/log.c](https://github.com/rxi/log.c) (~150 lines, 3.3k stars) but evolved into a fundamentally different architecture (~2,500 lines) optimized for embedded systems and advanced filtering.

### Core Differences

**rxi/log.c**: Minimalist runtime-only config. Fixed ~5KB footprint, 6 levels, simple callbacks.
**microlog**: Compile-time feature selection. Configurable 200 bytes to ~15KB, 8 renameable levels, multi-dimensional filtering.

| Feature                   | log.c                                      | microlog                                                                       |
| ------------------------- | ------------------------------------------ | ------------------------------------------------------------------------------ |
| **Philosofy**             | Simple-to-use,  Minimalist for average use | Simple-to-use, extensible and configurable for diverse needs                   |
| **Configuration**         | Color, verbosity                           | ✅ Compile-time feature selection + optional runtime                            |
| **Runtime Configuration** | Only verbosity                             | ✅ Verbosity, color, time, prefix, source location, topics (Disablable feature) |
| **Zero-Overhead Disable** | Args are evaluated                         | ✅ True no-op with `ULOG_BUILD_DISABLED`                                        |
| **Log Levels**            | 6 fixed                                    | 8, runtime renameable (e.g., syslog)                                           |
| **Filtering**             | Global + per-callback                      | Per-output + per-topic + global                                                |
| **Topics/Subsystems**     | Manual prefixes                            | ✅ Full support with filtering per topic and routing                            |
| **Output Routing**        | All outputs get all logs                   | ✅ Route topics to specific or all outputs                                      |
| **Memory**                | Static                                     | ✅ Static or dynamic (user choice)                                              |
| **Build Systems**         | Manual integration                         | ✅ CMake, Meson, CPM packages                                                   |
| **Platform Helpers**      | DIY                                        | ✅ FreeRTOS, Zephyr, ThreadX, pthread, Win32                                    |

### Key Capabilities Unique to microlog

**1. Multi-Dimensional Filtering** - Per-output AND per-topic levels:

```c
ulog_topic_add("Credentials", secure_file_only, ULOG_LEVEL_TRACE);
ulog_topic_add("Network", ULOG_OUTPUT_ALL, ULOG_LEVEL_DEBUG);
ulog_output_level_set(ULOG_OUTPUT_STDOUT, ULOG_LEVEL_ERROR); // Console: errors only
```

**2. True Zero-Overhead Disable** - Arguments not evaluated:

```c
ulog_info("Result: %d", expensive()); // With ULOG_BUILD_DISABLED=1 → ((void)0)
```

**3. Custom Log Levels** - Redefine all 8 at runtime:

```c
ulog_level_descriptor syslog = {ULOG_LEVEL_7,
    {"DEBUG", "INFO", "NOTICE", "WARN", "ERR", "CRIT", "ALERT", "EMERG"}};
ulog_level_set_new_levels(&syslog);
```

**4. Fine-Grained Code Control** - 12+ build flags to strip features:

```
-DULOG_BUILD_SOURCE_LOCATION 0  // Remove file:line
-DULOG_BUILD_TIME 0              // Remove timestamps
-DULOG_BUILD_COLOR 0             // Remove ANSI codes
...
```

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for details.

## License

This library is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](LICENSE) for details.

## Credits

Based on [https://github.com/rxi/log.c.git](https://github.com/rxi/log.c.git)
