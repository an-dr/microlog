#include <cstdio>
#include "ulog.h"

void update_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    static int count = 0;
    snprintf(prefix, prefix_size, ", %d ms ", count++);
}

void custom_callback(ulog_Event *ev, void *arg) {
    printf("%s", (const char *)arg);
    static char buffer[128];
    ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

int main(int argc, char *argv[]) {

    printf("\n");

    ulog_set_level(LOG_DEBUG);

    /* Extra Outputs =============================== */

#if ULOG_FEATURE_EXTRA_OUTPUTS

    FILE *fp = fopen("example.log", "w");
    ulog_add_fp(fp, LOG_INFO);
    ulog_add_callback(custom_callback,
                      (void *)"     - Custom Callback: ", LOG_INFO);

#endif  // ULOG_FEATURE_EXTRA_OUTPUTS

    /* Custom Prefix ==================================== */

#if ULOG_FEATURE_CUSTOM_PREFIX
    ulog_set_prefix_fn(update_prefix);
#endif  // ULOG_FEATURE_CUSTOM_PREFIX

    /* Core Logging ===================================== */
    // ulog_set_quiet(true);
    log_debug("Trace message %d", 1);
    log_debug("Debug message 0x%x", 2);
    log_info("Info message %f", 3.0);
    log_warning("Warning message %c", '4');
    log_err("Error message %s", "Five");
    log_emerg("Fatal message %s", "6");

    /* Topics =========================================== */
    printf("\n");

#if ULOG_FEATURE_TOPICS

    ulog_add_topic("Bluetooth", true);
    ulog_add_topic("Audio", false);
    ulog_add_topic("Serial", false);

    // logt_fatal("Serial", "Serial message 0");

    ulog_enable_all_topics();
    logt_debug("Bluetooth", "Bluetooth message 1");
    logt_debug("Indication", "Indication message 1");
    logt_info("Audio", "Audio message");
    logt_warning("Serial", "Serial message 1");
    logt_err("Serial", "Serial message 2");

    ulog_disable_topic("Serial");
    // logt_warning("Serial", "Serial message 3 (disabled)");

    ulog_set_level(LOG_INFO);
    ulog_set_topic_level("Bluetooth", LOG_WARNING);
    ulog_set_topic_level("Indication", LOG_DEBUG);
    logt_info("Bluetooth", "Bluetooth message 2");
    // logt_info("Indication",
    //          "Indication message 2 (level lower than global level)");

#endif  // ULOG_FEATURE_TOPICS

    return 0;
}
