/* 
Example of using ulog library in C/C++

It demonstrates basic logging functionality using C interface.
The output to console looks like this (with possible variations based on 
build configuration):

18:50:32 [MsgID:000] TRACE main.cpp:71: Trace message 1
18:50:32 [MsgID:001] DEBUG main.cpp:75: Debug message 0x2
18:50:32 [MsgID:002] INFO  main.cpp:76: Info message 3.000000
[Output msg count:0] 18:50:32 [MsgID:002] INFO  main.cpp:76: Info message 3.000000
18:50:32 [MsgID:003] WARN  main.cpp:77: Warning message 4
[Output msg count:1] 18:50:32 [MsgID:003] WARN  main.cpp:77: Warning message 4
18:50:32 [MsgID:004] ERROR main.cpp:78: Error message Five
[Output msg count:2] 18:50:32 [MsgID:004] ERROR main.cpp:78: Error message Five
18:50:32 [MsgID:005] FATAL main.cpp:79: Fatal message 6
[Output msg count:3] 18:50:32 [MsgID:005] FATAL main.cpp:79: Fatal message 6

18:50:32 [MsgID:006] DEBUG [Bluetooth] main.cpp:89: Bluetooth message 1
18:50:32 [MsgID:007] WARN  [Audio] main.cpp:94: Audio message 2
[Output msg count:4] 18:50:32 [MsgID:007] WARN  [Audio] main.cpp:94: Audio message 2
18:50:32 [MsgID:008] ERROR [Serial] main.cpp:95: Serial message 1
[Output msg count:5] 18:50:32 [MsgID:008] ERROR [Serial] main.cpp:95: Serial message 1
18:50:32 [MsgID:009] FATAL [Serial] main.cpp:96: Serial message 2
[Output msg count:6] 18:50:32 [MsgID:009] FATAL [Serial] main.cpp:96: Serial message 2
*/

#include <cstdio>
#include "ulog.h"

// This prefix function can be used to add any custom information calculated
//  at log time, e.g. request ID, user ID, additional time metrics, etc.
// In this example we use it to add ms
void user_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    (void)ev; // Unused in this example
    static int count = 0; // count represents fake milliseconds
    snprintf(prefix, prefix_size, " [MsgID:%03d] ", count++);
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
    (void)argc;
    (void)argv;

    printf("\n");

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

    ulog_topic_debug( "Bluetooth", "Bluetooth message 1");  
    // Short alias: ulog_t_debug("Bluetooth", "Bluetooth message 1");
    
    ulog_t_warn("Audio", "Audio message 2");
    ulog_t_error("Serial", "Serial message 1");
    ulog_t_fatal("Serial", "Serial message 2");

    ulog_topic_disable("Serial");
    ulog_t_warn("Serial", "Serial message 3 (disabled)");

    ulog_output_level_set_all(ULOG_LEVEL_INFO);
    ulog_topic_level_set("Bluetooth", ULOG_LEVEL_WARN);

    ulog_t_info("Bluetooth", "Bluetooth message 2 (lower than topic level)");
    ulog_t_debug("Serial", "Serial message 3 (lower than global level)");

    return 0;
}
