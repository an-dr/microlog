// LogTopics.ino
// Demonstrates using log topics with Microlog on Arduino.
// IMPORTANT: To use topics, you must define ULOG_TOPICS_NUM
// BEFORE including ulog_arduino.h or ulog.h.
// For example, in your platformio.ini or by modifying ulog.h for Arduino.
// If ULOG_TOPICS_NUM is 0 (default for Arduino), topic logging will not work as expected.

// Option 1: Define ULOG_TOPICS_NUM in your build environment (e.g., platformio.ini build_flags)
// build_flags = -DULOG_TOPICS_NUM=3

// Option 2: Define it here before any includes (less common for .ino, but possible if structure allows)
// #define ULOG_TOPICS_NUM 3 // Example: enable 3 static topics

#include <ulog_arduino.h>

// Define topic names (must match what's used in logt_arduino_xxx macros)
const char* TOPIC_SENSOR = "Sensor";
const char* TOPIC_NETWORK = "Network";
const char* TOPIC_SYSTEM = "System";

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect.
  }

  ulog_arduino_init();
  ulog_set_level(LOG_TRACE); // Global log level

  #if ULOG_TOPICS_NUM > 0
    Serial.println("LogTopics example started. ULOG_TOPICS_NUM is defined.");

    // Add topics to the ulog system.
    // ulog_add_topic returns an ID, but for basic usage with logt_arduino_xxx,
    // you just need to ensure the topic name string matches.
    // The ulog_arduino log function will try to add/find it if dynamic allocation is on,
    // or find it if static.
    // For static allocation (default if ULOG_TOPICS_NUM > 0 but not -1),
    // topics are typically added implicitly by their first use if using ulog_log directly,
    // or need to be "declared" if there's a fixed list.
    // With logt_arduino_xxx, ulog_arduino.cpp handles looking up or adding the topic.

    // If using static topics (ULOG_TOPICS_NUM is a positive integer):
    // It's good practice to "declare" topics to ensure they get registered if not using dynamic allocation.
    // This step might be optional if ulog_arduino.cpp's ulog_add_topic call (if any for static) handles it.
    // For now, assume ulog_arduino.cpp will try to resolve topic names.
    ulog_add_topic(TOPIC_SENSOR, true);  // Name, initially enabled
    ulog_add_topic(TOPIC_NETWORK, true);
    ulog_add_topic(TOPIC_SYSTEM, true);

    // You can set individual topic levels
    // ulog_set_topic_level(TOPIC_SENSOR, LOG_DEBUG);
    // ulog_set_topic_level(TOPIC_NETWORK, LOG_INFO);

    // You can also disable/enable topics dynamically
    // ulog_disable_topic(TOPIC_SYSTEM);
    // logt_arduino_info(TOPIC_SYSTEM, "This message from TOPIC_SYSTEM will not appear if disabled.");
    // ulog_enable_topic(TOPIC_SYSTEM);


  #else // ULOG_TOPICS_NUM
    Serial.println("LogTopics example: ULOG_TOPICS_NUM is not defined or is 0.");
    Serial.println("Topic-based logging will be disabled or may not work as expected.");
    Serial.println("Please define ULOG_TOPICS_NUM (e.g., to 3 or 5) in your build configuration");
    Serial.println("or by modifying ulog.h for Arduino before including it.");
  #endif // ULOG_TOPICS_NUM
}

void loop() {
  #if ULOG_TOPICS_NUM > 0
    logt_arduino_debug(TOPIC_SENSOR, "Temperature: %d C, Humidity: %d %%", random(20, 30), random(40, 60));
    logt_arduino_info(TOPIC_NETWORK, "Packet sent. Length: %d bytes", random(50, 150));

    if (millis() % 10000 < 1000) { // Occasionally log a system message
        logt_arduino_warn(TOPIC_SYSTEM, "System check. Uptime: %lu s", millis()/1000);
    }

    // Example of a message that might be filtered by topic level if set
    // logt_arduino_trace(TOPIC_SENSOR, "Raw sensor data...");

  #else // ULOG_TOPICS_NUM
    Serial.println("Topic logging is effectively disabled in this configuration.");
    // The logt_arduino_xxx macros will still compile, but the topic part might be ignored or cause issues
    // if ulog_arduino.cpp cannot resolve/handle the topic when ULOG_TOPICS_NUM is 0.
    // (Based on current ulog_arduino.cpp, it will try to use topic ID -1 if not found and dynamic add fails)
    logt_arduino_info("GenericTopic", "This message uses a topic macro, but topics are disabled.");

  #endif // ULOG_TOPICS_NUM

  delay(2000);
}
