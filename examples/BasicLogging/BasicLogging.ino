// BasicLogging.ino
// Demonstrates basic logging to Serial with Microlog

// To use Microlog, you need to include ulog_arduino.h
// The Arduino build system should automatically find ulog.c and ulog_arduino.cpp
// in the library's src directory and compile them.
#include <ulog_arduino.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Initialize Microlog for Arduino.
  // This sets up Serial as the default output.
  // It also calls ulog_set_quiet(true) to prevent ulog's default stdout logging.
  ulog_arduino_init(); // Uses Serial by default

  // You can also pass a different Print object, e.g., an EthernetClient or LCD library object
  // Example: ulog_arduino_init(&myCustomPrintObject);

  // Set the global log level for the core ulog library.
  // Messages below this level will be ignored by ulog_arduino as well.
  ulog_set_level(LOG_TRACE); // Show all messages

  log_arduino_info("Microlog setup complete. Basic logging example started.");
}

void loop() {
  log_arduino_trace("This is a TRACE message from loop().");
  log_arduino_debug("This is a DEBUG message from loop(). Value: %d", 123);
  log_arduino_info("This is an INFO message. Millis: %lu", millis());
  log_arduino_warn("This is a WARNING message.");
  log_arduino_error("This is an ERROR message!");
  log_arduino_fatal("This is a FATAL message! Something critical happened.");

  delay(5000); // Log messages every 5 seconds
}
