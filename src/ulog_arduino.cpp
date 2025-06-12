#ifdef ARDUINO

#include "ulog_arduino.h" // The new header file
#include <stdio.h>        // For vsnprintf
#include <stdarg.h>       // For va_list, va_start, va_end

static Print* ulog_arduino_client = NULL;
static char ulog_buffer[128]; // Buffer for formatted log messages

void ulog_arduino_init(Print* client) {
    ulog_arduino_client = client;
    // Set any default ulog configurations suitable for Arduino
    ulog_set_quiet(true); // Disable original stdout logging
    #ifdef ULOG_NO_COLOR
        // Ensure colors are off if ULOG_NO_COLOR is defined,
        // which is typical for Arduino.
    #else
        // If ULOG_NO_COLOR is not defined, explicitly disable for Arduino
        // by modifying ulog's internal state if possible,
        // or ensure ulog_event_to_cstr doesn't produce color codes.
        // For now, we rely on ULOG_NO_COLOR being defined in ulog.h for Arduino.
    #endif
}

void ulog_arduino(int level, const char *file, int line, const char *topic, const char *fmt, ...) {
    if (!ulog_arduino_client) {
        // Initialize with default if not already done
        // This is a fallback, ideally ulog_arduino_init is called in setup()
        ulog_arduino_init(&ULOG_ARDUINO_PRINT);
    }

    // Create a ulog_Event
    ulog_Event ev;
    ev.level = level;
    ev.file = file;
    ev.line = line;
    #if FEATURE_TOPICS // Only include topic if the feature is enabled
    // Attempt to get topic_id. If not found and dynamic allocation is off, this might be an issue.
    // For Arduino, it's safer if topics are pre-added or dynamic allocation is enabled and managed.
    if (topic != NULL) {
        ev.topic = ulog_get_topic_id(topic);
        if (ev.topic == TOPIC_NOT_FOUND) {
            #if CFG_TOPICS_DINAMIC_ALLOC == true
                // Dynamically add topic if enabled
                ev.topic = ulog_add_topic(topic, true); // Assuming new topics are enabled by default
            #else
                // Topic not found and dynamic allocation is disabled.
                // Log without topic or skip logging for this message.
                // For simplicity, we'll log without the topic.
                ev.topic = -1; // Indicate no topic / invalid topic
            #endif
        }
    } else {
        ev.topic = -1; // No topic
    }
    #endif
    // ev.time will be handled by ulog_event_to_cstr if FEATURE_TIME is enabled

    // Format the message using vsnprintf first (as ulog_event_to_cstr needs a va_list for the message part)
    // This approach is slightly different from how ulog_log works internally with ulog_Event.message and ulog_Event.message_format_args
    // We will format the user message into a temporary buffer first.

    char user_message_buffer[100]; // Temporary buffer for user's formatted message
    va_list args;
    va_start(args, fmt);
    vsnprintf(user_message_buffer, sizeof(user_message_buffer), fmt, args);
    va_end(args);

    ev.message = user_message_buffer; // Point to the formatted user message.
    // For ulog_event_to_cstr, the message_format_args would ideally be empty or NULL
    // as user_message_buffer is already formatted.
    // However, ulog_event_to_cstr doesn't directly use message_format_args if ev.message is pre-formatted.
    // Let's ensure ev.message_format_args is NULL.
    // va_list dummy_args; // Not straightforward to set to NULL or make empty
    // ev.message_format_args = dummy_args; // This is problematic.

    // Revisit: The ulog_Event structure expects `message` to be a format string
    // and `message_format_args` to be the va_list for it.
    // The current ulog_event_to_cstr is designed to take these and then internally call vprint.
    // To use ulog_event_to_cstr effectively here, we might need to adjust it or
    // pass the original fmt and args.

    // Corrected approach for ulog_Event:
    ev.message = fmt; // Pass the original format string
    va_start(ev.message_format_args, fmt); // Pass the original va_list

    // Use ulog_event_to_cstr to format the full log entry
    // Make sure ULOG_NO_COLOR is defined for Arduino builds in ulog.h
    // or that ulog_event_to_cstr is modified to respect an Arduino environment.
    int result = ulog_event_to_cstr(&ev, ulog_buffer, sizeof(ulog_buffer));

    va_end(ev.message_format_args); // End va_list after use

    if (result == 0) {
        ulog_arduino_client->println(ulog_buffer);
    } else {
        // Handle error: buffer too small or other issue
        ulog_arduino_client->println(F("Log formatting error or buffer too small."));
    }
}

#endif // ARDUINO
