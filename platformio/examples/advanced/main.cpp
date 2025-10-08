#include <Arduino.h>
#include "ulog.h"

// Advanced configuration with topics
#define ULOG_BUILD_COLOR 1
#define ULOG_BUILD_TIME 1
#define ULOG_BUILD_SOURCE_LOCATION 1
#define ULOG_BUILD_TOPICS_NUM 5  // Support for 5 topics
#define ULOG_BUILD_PREFIX_SIZE 32
#define ULOG_BUILD_LEVEL_SHORT 1

// Custom prefix function to add message ID
static int msg_id = 0;
static char prefix_buffer[32];

const char* custom_prefix() {
    snprintf(prefix_buffer, sizeof(prefix_buffer), "MsgID:%03d", msg_id++);
    return prefix_buffer;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect
    }
    
    // Set custom prefix function
    ulog_prefix_set_fn(custom_prefix);
    
    // Add topics for different subsystems
    ulog_topic_add("WIFI", ULOG_OUTPUT_ALL, true);
    ulog_topic_add("SENSOR", ULOG_OUTPUT_ALL, true);
    ulog_topic_add("MQTT", ULOG_OUTPUT_ALL, true);
    ulog_topic_add("SYSTEM", ULOG_OUTPUT_ALL, true);
    
    ulog_info("Advanced microlog PlatformIO example started");
    ulog_topic_info("SYSTEM", "All subsystems initialized");
}

void loop() {
    static unsigned long last_wifi_check = 0;
    static unsigned long last_sensor_read = 0;
    static unsigned long last_mqtt_send = 0;
    static int sensor_value = 0;
    
    unsigned long now = millis();
    
    // Simulate WiFi status checks every 5 seconds
    if (now - last_wifi_check > 5000) {
        last_wifi_check = now;
        ulog_topic_debug("WIFI", "Checking WiFi connection status");
        ulog_topic_info("WIFI", "WiFi signal strength: %d dBm", random(-40, -80));
    }
    
    // Simulate sensor readings every 2 seconds
    if (now - last_sensor_read > 2000) {
        last_sensor_read = now;
        sensor_value = random(20, 30);
        ulog_topic_trace("SENSOR", "Reading temperature sensor");
        ulog_topic_info("SENSOR", "Temperature: %d°C", sensor_value);
        
        if (sensor_value > 28) {
            ulog_topic_warn("SENSOR", "Temperature high: %d°C", sensor_value);
        }
    }
    
    // Simulate MQTT publishing every 10 seconds
    if (now - last_mqtt_send > 10000) {
        last_mqtt_send = now;
        ulog_topic_debug("MQTT", "Preparing to send data");
        ulog_topic_info("MQTT", "Publishing sensor data to cloud");
        
        // Simulate occasional connection issues
        if (random(0, 10) > 8) {
            ulog_topic_error("MQTT", "Failed to publish data - connection timeout");
        }
    }
    
    // System monitoring
    static unsigned long last_system_check = 0;
    if (now - last_system_check > 15000) {
        last_system_check = now;
        ulog_topic_debug("SYSTEM", "Free heap: %d bytes", ESP.getFreeHeap());
        
        if (ESP.getFreeHeap() < 10000) {
            ulog_topic_warn("SYSTEM", "Low memory warning");
        }
    }
    
    delay(100);
}
