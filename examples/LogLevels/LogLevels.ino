// LogLevels.ino
// Demonstrates using different log levels with Microlog

#include <ulog_arduino.h>

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  ulog_arduino_init(); // Initialize with Serial

  log_arduino_info("LogLevels example started.");
  log_arduino_info("Default log level is LOG_TRACE (shows all).");
  log_arduino_info("Messages will be logged for 5 seconds at current level, then level changes.");
}

void loop() {
  unsigned long startTime = millis();
  unsigned long currentTime;
  int currentLevelFilter;

  // Cycle through log levels
  for (currentLevelFilter = LOG_TRACE; currentLevelFilter <= LOG_FATAL; ++currentLevelFilter) {
    ulog_set_level(currentLevelFilter); // Set the global log filter level
    Serial.print("Current global log filter level set to: ");
    Serial.println(ulog_get_level_string(currentLevelFilter)); // ulog_get_level_string is from ulog.h

    startTime = millis();
    do {
      log_arduino_trace("This is a TRACE message.");
      log_arduino_debug("This is a DEBUG message. Random: %d", random(100));
      log_arduino_info("This is an INFO message. Uptime: %lu ms", millis());
      log_arduino_warn("This is a WARNING message.");
      log_arduino_error("This is an ERROR message!");
      log_arduino_fatal("This is a FATAL message!");
      delay(1000); // Wait a bit between bursts of messages
      currentTime = millis();
    } while (currentTime - startTime < 5000); // Log for 5 seconds at this level
  }

  Serial.println("Completed one cycle through log levels. Restarting cycle.");
  delay(2000);
}
