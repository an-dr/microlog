#include <Arduino.h>
#include "ulog.h"

// Example configuration for microlog
// You can customize these defines based on your needs
#define ULOG_BUILD_COLOR 1
#define ULOG_BUILD_TIME 1
#define ULOG_BUILD_SOURCE_LOCATION 1

void setup() {
    // Initialize Serial communication
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Initialize microlog
    ulog_info("Microlog PlatformIO example started");
}

void loop() {
    static int counter = 0;
    
    // Demonstrate different log levels
    ulog_trace("Trace message #%d", counter);
    ulog_debug("Debug message #%d", counter);
    ulog_info("Info message #%d", counter);
    ulog_warn("Warning message #%d", counter);
    
    if (counter % 10 == 0) {
        ulog_error("Error message #%d", counter);
    }
    
    if (counter % 20 == 0) {
        ulog_fatal("Fatal message #%d", counter);
    }
    
    counter++;
    delay(1000);
}
