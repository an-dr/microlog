// clang-format off
/*
Example of using ulog library in C/C++

It demonstrates basic logging functionality using C interface.
The output to console looks like this (with possible variations based on
build configuration):

    16:20:14 | MsgID:000 | TRACE main.cpp:97: Trace message 1
    16:20:14 | MsgID:001 | DEBUG main.cpp:99: Debug message 0x2
    16:20:14 | MsgID:002 | INFO  main.cpp:100: Info message 3.000000
    [Output msg count:0] 16:20:14 | MsgID:002 | INFO  main.cpp:100: Info message 3.000000
    16:20:14 | MsgID:003 | WARN  main.cpp:101: Warning message 4
    [Output msg count:1] 16:20:14 | MsgID:003 | WARN  main.cpp:101: Warning message 4
    16:20:14 | MsgID:004 | ERROR main.cpp:102: Error message Five
    [Output msg count:2] 16:20:14 | MsgID:004 | ERROR main.cpp:102: Error message Five
    16:20:14 | MsgID:005 | FATAL main.cpp:103: Fatal message 6
    [Output msg count:3] 16:20:14 | MsgID:005 | FATAL main.cpp:103: Fatal message 6

    16:20:14 | MsgID:006 | DEBUG [Bluetooth] main.cpp:115: Bluetooth message 1
    16:20:14 | MsgID:007 | WARN  [Audio] main.cpp:118: Audio message 2
    [Output msg count:4] 16:20:14 | MsgID:007 | WARN  [Audio] main.cpp:118: Audio message 2
    16:20:14 | MsgID:008 | ERROR [Serial] main.cpp:119: Serial message 1
    [Output msg count:5] 16:20:14 | MsgID:008 | ERROR [Serial] main.cpp:119: Serial message 1
    16:20:14 | MsgID:009 | FATAL [Serial] main.cpp:120: Serial message 2
    [Output msg count:6] 16:20:14 | MsgID:009 | FATAL [Serial] main.cpp:120: Serial message 2

    16:20:14 | MsgID:010 | DEBUG  | [Audio] main.cpp:144: Message for debugging
    [Output msg count:7] 16:20:14 | MsgID:010 | DEBUG  | [Audio] main.cpp:144: Message for debugging
    16:20:14 | MsgID:011 | INFO   | [Bluetooth] main.cpp:145: General information
    [Output msg count:8] 16:20:14 | MsgID:011 | INFO   | [Bluetooth] main.cpp:145: General information
    16:20:14 | MsgID:012 | NOTICE | [Serial] main.cpp:146: Important notice
    [Output msg count:9] 16:20:14 | MsgID:012 | NOTICE | [Serial] main.cpp:146: Important notice
    16:20:14 | MsgID:013 | WARN   | [Audio] main.cpp:147: Warning message
    [Output msg count:10] 16:20:14 | MsgID:013 | WARN   | [Audio] main.cpp:147: Warning message
    16:20:14 | MsgID:014 | ERR    | [Audio] main.cpp:148: Error message
    [Output msg count:11] 16:20:14 | MsgID:014 | ERR    | [Audio] main.cpp:148: Error message
    16:20:14 | MsgID:015 | CRIT   | [Bluetooth] main.cpp:149: Critical condition
    [Output msg count:12] 16:20:14 | MsgID:015 | CRIT   | [Bluetooth] main.cpp:149: Critical condition
    16:20:14 | MsgID:016 | ALERT  | [Serial] main.cpp:150: Alert: action must be taken immediately
    [Output msg count:13] 16:20:14 | MsgID:016 | ALERT  | [Serial] main.cpp:150: Alert: action must be taken immediately
    16:20:14 | MsgID:017 | EMERG  | [Audio] main.cpp:151: Emergency: system is unusable
    [Output msg count:14] 16:20:14 | MsgID:017 | EMERG  | [Audio] main.cpp:151: Emergency: system is unusable
*/
// clang-format on

#include <cstdio>
#include "ulog.h"

// This prefix function can be used to add any custom information calculated
//  at log time, e.g. request ID, user ID, additional time metrics, etc.
// In this example we use it to add ms
void user_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev;              // Unused in this example
    static int count = 0;  // count represents fake milliseconds
    snprintf(prefix, prefix_size, " | MsgID:%03d | ", count++);
}

// This handler can be used to get a log string
// and send it to any other output, e.g. network, GUI, etc.
// In this example we just print it to the console with appending
// data sent as a void pointer ("User Output"). We format the data like
// this: "[User Output:N] <log string>"
void user_output(ulog_event *ev, void *arg) {
    // Print argument data
    static int count = 0;
    printf("[%s:%d] ", (const char *)arg, count++);

    // Print log string
    static char buffer[128];
    ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

// Simple lock function with simple locking
// This is just an example, real locking should use mutexes or similar
// mechanisms to ensure thread safety.
// The lock function should return ULOG_STATUS_OK on success,
// other values on error.
ulog_status user_lock(bool lock, void *arg) {
    (void)(arg);
    static bool is_locked = false;
    if (lock) {
        if (is_locked) {
            return ULOG_STATUS_BUSY;  // Already locked
        }
        is_locked = true;
    } else {
        if (!is_locked) {
            return ULOG_STATUS_BUSY;  // Not locked
        }
        is_locked = false;
    }
    return ULOG_STATUS_OK;
}

int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("\n");

    ulog_lock_set_fn(user_lock, nullptr);  // Set user lock function
    ulog_output_level_set_all(ULOG_LEVEL_TRACE);

    /* Extra Outputs =============================== */

    FILE *fp = fopen("example.log", "w");
    ulog_output_add_file(fp, ULOG_LEVEL_INFO);
    ulog_output_add(user_output, (void *)"Output msg count", ULOG_LEVEL_INFO);

    /* Prefix ==================================== */
    ulog_prefix_set_fn(user_prefix);

    /* Core Logging ===================================== */
    ulog_trace("Trace message %d", 1);

    ulog_debug("Debug message 0x%x", 2);
    ulog_info("Info message %f", 3.0);
    ulog_warn("Warning message %c", '4');
    ulog_error("Error message %s", "Five");
    ulog_fatal("Fatal message %s", "6");

    /* Topics =========================================== */
    printf("\n");

    ulog_topic_add("Bluetooth", ULOG_OUTPUT_ALL, true);
    ulog_topic_add("Serial", ULOG_OUTPUT_ALL, false);
    ulog_topic_add("Audio", ULOG_OUTPUT_ALL, false);
    ulog_t_warn("Audio", "Audio message 1 (disabled)");

    ulog_topic_enable_all();

    ulog_topic_debug("Bluetooth", "Bluetooth message 1");
    // Short alias: ulog_t_debug("Bluetooth", "Bluetooth message 1");

    ulog_t_warn("Audio", "Audio message 2");
    ulog_t_error("Serial", "Serial message 1");
    ulog_t_fatal("Serial", "Serial message 2");

    ulog_topic_disable("Serial");
    ulog_t_warn("Serial", "Serial message 3 (disabled)");

    ulog_output_level_set_all(ULOG_LEVEL_TRACE);
    ulog_topic_level_set("Bluetooth", ULOG_LEVEL_WARN);

    ulog_t_info("Bluetooth", "Bluetooth message 2 (lower than topic level)");
    ulog_t_debug("Serial", "Serial message 3 (lower than global level)");

    /* Custom Log Levels ============================ */
    printf("\n");

    static ulog_level_descriptor syslog_levels = {
        .max_level = ULOG_LEVEL_7,
        .names = {"DEBUG  |", "INFO   |", "NOTICE |", "WARN   |", "ERR    |", "CRIT   |",
                  "ALERT  |", "EMERG  |"}};
    ulog_level_set_new_levels(&syslog_levels);

    ulog_topic_enable_all();
    ulog_topic_level_set("Audio", ULOG_LEVEL_0);
    ulog_topic_level_set("Bluetooth", ULOG_LEVEL_0);
    ulog_topic_level_set("Serial", ULOG_LEVEL_0);
    ulog_t_log(ULOG_LEVEL_0, "Audio", "Message for debugging");
    ulog_t_log(ULOG_LEVEL_1, "Bluetooth", "General information");
    ulog_t_log(ULOG_LEVEL_2, "Serial", "Important notice");
    ulog_t_log(ULOG_LEVEL_3, "Audio", "Warning message");
    ulog_t_log(ULOG_LEVEL_4, "Audio", "Error message");
    ulog_t_log(ULOG_LEVEL_5, "Bluetooth", "Critical condition");
    ulog_t_log(ULOG_LEVEL_6, "Serial", "Alert: action must be taken immediately");
    ulog_t_log(ULOG_LEVEL_7, "Audio", "Emergency: system is unusable");

    return 0;
}
