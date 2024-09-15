#include <stdio.h>
#include "ulog.h"


void update_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    static int count = 0;
    snprintf(prefix, prefix_size, "> %d ms <", count++);
}

void custom_callback(ulog_Event *ev, void *arg) {
    printf("%s", (const char *) arg);
    static char buffer[128];
    ulog_event_to_cstr(ev, false, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

int main(int argc, char *argv[]) {
    printf("\n");

    ulog_set_level(LOG_TRACE);

#if FEATURE_EXTRA_DESTS
    printf("Extra destinations: %d\n", ULOG_EXTRA_DESTINATIONS);
    FILE *fp = fopen("example.log", "w");
    ulog_add_fp(fp, LOG_TRACE);
    // ulog_add_fp(stdout, LOG_ERROR);

    ulog_add_callback(custom_callback, "Custom Callback: ", LOG_TRACE);
#endif

#if FEATURE_CUSTOM_PREFIX
    ulog_set_prefix_fn(update_prefix);
#endif

    log_trace("Trace message %d", 1);
    log_debug("Debug message 0x%x", 2);
    log_info("Info message %f", 3.0);
    log_warn("Warning message");
    log_error("Error message");
    log_fatal("Fatal message");
    return 0;
}
