#include <cstdio>
#include "ulog.h"

void update_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    static int count = 0;
    snprintf(prefix, prefix_size, ".%03d ms ", count++);
}

void user_output(ulog_event *ev, void *arg) {
    static int count = 0;
    printf("[%s:%d] ", (const char *)arg, count++);
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
    ulog_output_add(user_output,
                    (void *)"User Output", ULOG_LEVEL_INFO);

    /* Prefix ==================================== */
    ulog_prefix_set_fn(update_prefix);

    /* Core Logging ===================================== */
    log_trace("Trace message %d", 1);
    log_debug("Debug message 0x%x", 2);
    log_info("Info message %f", 3.0);
    log_warn("Warning message %c", '4');
    log_error("Error message %s", "Five");
    log_fatal("Fatal message %s", "6");

    /* Topics =========================================== */
    printf("\n");

    ulog_topic_add("Bluetooth", true);
    ulog_topic_add("Serial", false);
    ulog_topic_add("Audio", false);
    ulog_topic_warn("Audio", "Audio message 1 (disabled)");
    
    ulog_topic_enable_all();
    
    ulog_topic_debug("Bluetooth", "Bluetooth message 1");
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
