# C++ Static ulog Interface

This document describes the static C++ interface for the ulog library, which provides an easy-to-use wrapper around the C API.

## Features

The static interface provides two main classes:
- `ulog::Log` - Static class for global logging operations
- `ulog::Logger` - Instance-based logger for scoped logging with topics

## Basic Usage

### Static Logging

```cpp
#include "ulog.hpp"

int main() {
    // Configure logging
    ulog::Log::enable_colors(true);
    ulog::Log::enable_timestamps(true);
    ulog::Log::set_level(ulog::Level::debug);

    // Basic logging
    ulog::Log::info("Hello, world!");
    ulog::Log::debug("Debug value: %d", 42);
    ulog::Log::warn("Warning: %s", "Something happened");
    ulog::Log::error("Error code: %d", 404);
    ulog::Log::fatal("Critical error occurred");

    return 0;
}
```

### Topic-Based Logging

```cpp
// Add topics
ulog::Log::add_topic("NETWORK", true);
ulog::Log::add_topic("DATABASE", true);

// Log with topics
ulog::Log::info("NETWORK", "Connected to %s:%d", "localhost", 8080);
ulog::Log::debug("DATABASE", "Query executed in %d ms", 150);

// Control topic visibility
ulog::Log::disable_topic("DATABASE");
ulog::Log::set_topic_level("NETWORK", ulog::Level::warn);
```

### Instance-Based Logger

```cpp
// Create loggers with specific topics and levels
ulog::Logger network_logger("NETWORK", ulog::Level::info);
ulog::Logger general_logger; // No topic, default level

// Use instance loggers
network_logger.info("Connection established");
network_logger.debug("This won't show if level is info");

general_logger.trace("Application trace");
general_logger.error("Something went wrong");

// Change logger level
network_logger.set_level(ulog::Level::debug);
```

## Configuration Options

### Global Configuration

```cpp
// Enable/disable features
ulog::Log::enable_colors(true);
ulog::Log::enable_timestamps(true);
ulog::Log::enable_source_location(true);
ulog::Log::enable_prefix(true);
ulog::Log::enable_topics(true);

// Set global log level
ulog::Log::set_level(ulog::Level::warn);
```

### Output Management

```cpp
// Add file output
FILE* log_file = fopen("app.log", "w");
int file_output = ulog::Log::add_file_output(log_file, ulog::Level::info);

// Set output-specific levels
ulog::Log::set_output_level(file_output, ulog::Level::error);

// Remove outputs when done
ulog::Log::remove_output(file_output);
fclose(log_file);
```

### Thread Safety

```cpp
#include <mutex>

std::mutex log_mutex;

void lock_function(bool lock, void* arg) {
    auto* mutex = static_cast<std::mutex*>(arg);
    if (lock) {
        mutex->lock();
    } else {
        mutex->unlock();
    }
}

// Set up thread-safe logging
ulog::Log::set_lock_function(lock_function, &log_mutex);
```

## Log Levels

The library supports six log levels in ascending order of severity:

- `ulog::Level::trace` - Most verbose level for tracing execution
- `ulog::Level::debug` - Debug information for developers  
- `ulog::Level::info` - General information messages
- `ulog::Level::warn` - Warning messages for potential issues
- `ulog::Level::error` - Error messages for failures
- `ulog::Level::fatal` - Critical errors that may terminate program

## Automatic Source Location

The C++ interface automatically captures source location information (file name and line number) using C++20's `std::source_location`. This means you don't need to manually pass `__FILE__` and `__LINE__` like in the C API.

## Status Codes

Function return values use the `ulog::Status` enum:

- `ulog::Status::ok` - Operation completed successfully
- `ulog::Status::error` - General error occurred  
- `ulog::Status::invalid_argument` - Invalid argument provided

## Requirements

- C++20 compiler (for `std::source_location`)
- The underlying ulog C library
- Appropriate build configuration for desired features

## Build Configuration

The C++ interface works with all ulog build configurations. Features like topics, dynamic configuration, and extra outputs are automatically available when the underlying C library is built with those features enabled.
