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

    ulog_set_level(ULOG_DEBUG);

    /* Extra Outputs =============================== */

#if ULOG_FEATURE_EXTRA_OUTPUTS

    FILE *fp = fopen("example.log", "w");
    ulog_add_fp(fp, ULOG_INFO);
    ulog_add_callback(custom_callback,
                      (void *)"     - Custom Callback: ", ULOG_INFO);

#endif  // ULOG_FEATURE_EXTRA_OUTPUTS

    /* Custom Prefix ==================================== */

#if ULOG_FEATURE_CUSTOM_PREFIX
    ulog_set_prefix_fn(update_prefix);
#endif  // ULOG_FEATURE_CUSTOM_PREFIX

    /* Core Logging ===================================== */
    // ulog_set_quiet(true);
    log_emerg("Emergency message %d", 0);
    log_alert("Alert message 0x%x", 1);
    log_crit("Critical message %f", 2.0);
    log_err("Error message %c", '3');
    log_warning("Warning message %s", "Four");
    log_notice("Notice message %s", "Five");
    log_info("Info message %s", "Six");
    log_debug("Debug message %s", "Seven");

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

    ulog_set_level(ULOG_INFO);
    ulog_set_topic_level("Bluetooth", ULOG_WARNING);
    ulog_set_topic_level("Indication", ULOG_DEBUG);
    logt_info("Bluetooth", "Bluetooth message 2");
    // logt_info("Indication",
    //          "Indication message 2 (level lower than global level)");

#endif  // ULOG_FEATURE_TOPICS

    return 0;
}
