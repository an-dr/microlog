#include <stdio.h>
#include "ulog.h"


void update_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    static int count = 0;
    snprintf(prefix, prefix_size, ", %d ms ", count++);
}

void custom_callback(ulog_Event *ev, void *arg) {
    printf("%s", (const char *) arg);
    static char buffer[128];
    ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

int main(int argc, char *argv[]) {
    
    printf("\n");

    ulog_set_level(LOG_TRACE);
    
    /* Extra Outputs =============================== */

    FILE *fp = fopen("example.log", "w");
    ulog_add_fp(fp, LOG_INFO);
    ulog_add_callback(custom_callback, "     - Custom Callback: ", LOG_INFO);

    /* Custom Prefix ==================================== */
    ulog_set_prefix_fn(update_prefix);
    // ulog_set_quiet(true);

    /* Core Logging ===================================== */
    log_trace("Trace message %d", 1);
    log_debug("Debug message 0x%x", 2);
    log_info("Info message %f", 3.0);
    log_warn("Warning message %c", '4');
    log_error("Error message %s", "Five");
    log_fatal("Fatal message %s", "6");
    
    /* Topics =========================================== */
    printf("\n");
    
    ulog_add_topic("Bluetooth", true);
    ulog_add_topic("Audio", false);
    ulog_add_topic("Serial", false);
    
    // logt_fatal("Serial", "Serial message 0");
    
    ulog_enable_all_topics();
    logt_trace("Bluetooth", "Bluetooth message");
    logt_debug("Indication", "Indication message");
    logt_info("Audio", "Audio message");
    logt_warn("Serial", "Serial message 1");
    logt_error("Serial", "Serial message 2");
    
    ulog_disable_topic("Serial");
    // logt_warn("Serial", "Serial message 3 (disabled)");
    
    return 0;
}
