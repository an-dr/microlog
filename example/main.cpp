/* 
Example of using ulog library in C/C++

It demonstrates basic logging functionality using C interface.
The output to console looks like this (with possible variations based on 
build configuration):

    18:35:28.000 ms TRACE main.cpp:71: Trace message 1
    18:35:28.001 ms DEBUG main.cpp:75: Debug message 0x2
    18:35:28.002 ms INFO  main.cpp:76: Info message 3.000000
    [User Output:0] 18:35:28.004 ms INFO  main.cpp:76: Info message 3.000000
    18:35:28.005 ms WARN  main.cpp:77: Warning message 4
    [User Output:1] 18:35:28.007 ms WARN  main.cpp:77: Warning message 4
    18:35:28.008 ms ERROR main.cpp:78: Error message Five
    [User Output:2] 18:35:28.010 ms ERROR main.cpp:78: Error message Five
    18:35:28.011 ms FATAL main.cpp:79: Fatal message 6
    [User Output:3] 18:35:28.013 ms FATAL main.cpp:79: Fatal message 6

    18:35:28.014 ms DEBUG [Bluetooth] main.cpp:91: Bluetooth message 1
    18:35:28.015 ms WARN  [Audio] main.cpp:96: Audio message 2
    [User Output:4] 18:35:28.017 ms WARN  [Audio] main.cpp:96: Audio message 2
    18:35:28.018 ms ERROR [Serial] main.cpp:97: Serial message 1
    [User Output:5] 18:35:28.020 ms ERROR [Serial] main.cpp:97: Serial message 1
    18:35:28.021 ms FATAL [Serial] main.cpp:98: Serial message 2
    [User Output:6] 18:35:28.023 ms FATAL [Serial] main.cpp:98: Serial message 2
*/

#include <cstdio>
#include "ulog.h"

// This prefix function can be used to add any custom information calculated
//  at log time, e.g. request ID, user ID, additional time metrics, etc.
// In this example we use it to add ms
void user_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    static int count = 0; // count represents fake milliseconds
    snprintf(prefix, prefix_size, ".%03d ms ", count++);
}

// This callback can be used to get a log string 
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

int main(int argc, char *argv[]) {

    printf("\n");

    ulog_output_level_set_all(ULOG_LEVEL_TRACE);

    /* Extra Outputs =============================== */

    FILE *fp = fopen("example.log", "w");
    ulog_output_add_file(fp, ULOG_LEVEL_INFO);
    ulog_output_add(user_output, (void *)"User Output", ULOG_LEVEL_INFO);

    /* Prefix ==================================== */
    ulog_prefix_set_fn(user_prefix);

    /* Core Logging ===================================== */
    ulog_trace("Trace message %d", 1);  
    // `log_*` also works as an alias, e.g.:
    // log_trace("Trace message %d", 1);
    
    ulog_debug("Debug message 0x%x", 2);
    ulog_info("Info message %f", 3.0);
    ulog_warn("Warning message %c", '4');
    ulog_error("Error message %s", "Five");
    ulog_fatal("Fatal message %s", "6");

    /* Topics =========================================== */
    printf("\n");

    ulog_topic_add("Bluetooth", true);
    ulog_topic_add("Serial", false);
    ulog_topic_add("Audio", false);
    ulog_topic_warn("Audio", "Audio message 1 (disabled)");

    ulog_topic_enable_all();

    ulog_topic_debug( "Bluetooth", "Bluetooth message 1");  
    // `logt_*` also works as an alias, e.g.: 
    // logt_debug("Bluetooth", "Bluetooth message 1");
    
    ulog_topic_info("Indication", "Indication message 1 (only in dynamic topics)");
    ulog_topic_warn("Audio", "Audio message 2");
    ulog_topic_error("Serial", "Serial message 1");
    ulog_topic_fatal("Serial", "Serial message 2");

    ulog_topic_disable("Serial");
    ulog_topic_warn("Serial", "Serial message 3 (disabled)");

    ulog_output_level_set_all(ULOG_LEVEL_INFO);
    ulog_topic_level_set("Bluetooth", ULOG_LEVEL_WARN);

    ulog_topic_info("Bluetooth", "Bluetooth message 2 (lower than topic level)");
    ulog_topic_debug("Serial", "Serial message 3 (lower than global level)");

    return 0;
}
