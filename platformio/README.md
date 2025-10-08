# PlatformIO Support for MicroLog

This directory contains PlatformIO-specific files for the microlog library.

## Quick Start

### Option 1: Use from PlatformIO Registry (Recommended)

Once published, you can simply add the library to your `platformio.ini`:

```ini
[env:your_board]
platform = your_platform
board = your_board
framework = arduino
lib_deps = 
    microlog
```

### Option 2: Use from Git Repository

Add to your `platformio.ini`:

```ini
[env:your_board]
platform = your_platform
board = your_board
framework = arduino
lib_deps = 
    https://github.com/an-dr/microlog.git
```

### Option 3: Manual Installation

1. Download the library files
2. Place them in your project's `lib/microlog/` directory
3. Include the header in your code: `#include "ulog.h"`

## Configuration

Microlog is highly configurable through preprocessor definitions. Add them to your `platformio.ini`:

```ini
build_flags = 
    -DULOG_BUILD_COLOR=1              ; Enable color output
    -DULOG_BUILD_TIME=1               ; Enable timestamps
    -DULOG_BUILD_SOURCE_LOCATION=1    ; Show file:line info
    -DULOG_BUILD_LEVEL_SHORT=1        ; Use short level names
    -DULOG_BUILD_TOPICS_NUM=5         ; Enable 5 topics
    -DULOG_BUILD_PREFIX_SIZE=32       ; Set prefix buffer size
```

### Common Configuration Options

| Define | Description | Default |
|--------|-------------|---------|
| `ULOG_BUILD_COLOR` | Enable colored output | 0 |
| `ULOG_BUILD_TIME` | Include timestamps | 0 |
| `ULOG_BUILD_SOURCE_LOCATION` | Show file and line | 1 |
| `ULOG_BUILD_LEVEL_SHORT` | Use short level names (DBG, INF, etc.) | 0 |
| `ULOG_BUILD_TOPICS_NUM` | Number of supported topics | 0 |
| `ULOG_BUILD_PREFIX_SIZE` | Custom prefix buffer size | 0 |

## Memory-Constrained Devices

For devices with limited memory (like Arduino Uno), use a minimal configuration:

```ini
build_flags = 
    -DULOG_BUILD_COLOR=0
    -DULOG_BUILD_TIME=0
    -DULOG_BUILD_SOURCE_LOCATION=1
    -DULOG_BUILD_LEVEL_SHORT=1
```

## Usage Examples

### Basic Usage

```cpp
#include <Arduino.h>
#include "ulog.h"

void setup() {
    Serial.begin(115200);
    ulog_info("System started");
}

void loop() {
    ulog_debug("Loop iteration");
    delay(1000);
}
```

### Advanced Usage with Topics

```cpp
#include <Arduino.h>
#include "ulog.h"

void setup() {
    Serial.begin(115200);
    
    // Add topics for different subsystems
    ulog_topic_add("WIFI", ULOG_OUTPUT_ALL, true);
    ulog_topic_add("SENSOR", ULOG_OUTPUT_ALL, true);
    
    ulog_info("System initialized");
}

void loop() {
    ulog_topic_info("WIFI", "Connection status: OK");
    ulog_topic_debug("SENSOR", "Reading temperature...");
    delay(1000);
}
```

## Platform-Specific Notes

### ESP32/ESP8266

- Full feature support
- Recommended to enable colors and timestamps
- Plenty of memory for all features

### Arduino (AVR)

- Limited memory - use minimal configuration
- Disable colors and timestamps if needed
- Consider using short level names

### STM32

- Good memory availability
- Most features supported
- RTOS compatibility via locking extensions

## Troubleshooting

### Common Issues

1. **Compilation errors about undefined functions**
   - Make sure the library is properly included in your project
   - Check that you're using the correct header: `#include "ulog.h"`

2. **No output visible**
   - Ensure Serial.begin() is called before logging
   - Check your build flags configuration
   - Verify the log level is appropriate

3. **Memory issues on small devices**
   - Reduce configuration options
   - Disable features like colors, timestamps, and topics
   - Use `ULOG_BUILD_LEVEL_SHORT=1`

### Getting Help

- Check the main [README.md](../README.md) for general usage
- Review [doc/features.md](../doc/features.md) for detailed configuration
- Look at the examples in the `examples/` directory
- Open an issue on [GitHub](https://github.com/an-dr/microlog/issues)

## Building the Package

To build the PlatformIO package from source:

```bash
pwsh scripts/build_platformio.ps1
```

The package will be created in `install/platformio/microlog/`.

