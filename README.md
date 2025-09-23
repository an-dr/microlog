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

|<img src="doc/README/demo0.png" width="800">|
|-|
|Picture 1 - default configuration: no time, long default levels, source location, no topics, no colors|

...but in can be very minimalistic :

|<img src="doc/README/demo1.png" width="800">|
|-|
|Picture 2 - short levels, no colors, no time, no source location|

... or feature-rich:

|<img src="doc/README/demo2.png" width="800"> |
|-|
|Picture 3 - time, custom prefix for MsgID, custom syslog levels, topics, source location, colors|

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
```

**Option 3 - Meson Package**:

- Download a Meson Package from [Releases](https://github.com/an-dr/microlog/releases)
- Copy the content to `MyMesonProject/subprojects`
- Add to your dependencies:

```meson
add_global_arguments('-DULOG_BUILD_COLOR=1', language: ['cpp', 'c']) # configuration

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
target_compile_definitions( microlog
        INTERFACE
        ULOG_BUILD_COLOR=1) # configuration
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

## Changelog

See [CHANGELOG.md](CHANGELOG.md) for details.

## License

This library is free software; you can redistribute it and/or modify it under the terms of the MIT license. See [LICENSE](LICENSE) for details.

## Credits

Based on [https://github.com/rxi/log.c.git](https://github.com/rxi/log.c.git)
