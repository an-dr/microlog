#include <cstdio>
#include "ulog.h"

void update_prefix(ulog_event *ev, char *prefix, size_t prefix_size) {
    static int count = 0;
    snprintf(prefix, prefix_size, ", %d ms ", count++);
}

void custom_callback(ulog_event *ev, void *arg) {
    printf("%s", (const char *)arg);
    static char buffer[128];
    ulog_event_to_cstr(ev, buffer, sizeof(buffer));
    printf("%s\n", buffer);
}

int main(int argc, char *argv[]) {

    printf("\n");

    ulog_level_set(ULOG_LEVEL_TRACE);

    /* Extra Outputs =============================== */

#if ULOG_FEATURE_EXTRA_OUTPUTS

    FILE *fp = fopen("example.log", "w");
    ulog_user_callback_add_fp(fp, ULOG_LEVEL_INFO);
    ulog_user_callback_add(custom_callback,
                      (void *)"     - Custom Callback: ", ULOG_LEVEL_INFO);

#endif  // ULOG_FEATURE_EXTRA_OUTPUTS

    /* Custom Prefix ==================================== */

#if ULOG_FEATURE_CUSTOM_PREFIX
    ulog_prefix_set_fn(update_prefix);
#endif  // ULOG_FEATURE_CUSTOM_PREFIX

    /* Core Logging ===================================== */
    // ulog_set_quiet(true);
    log_trace("Trace message %d", 1);
    log_debug("Debug message 0x%x", 2);
    log_info("Info message %f", 3.0);
    log_warn("Warning message %c", '4');
    log_error("Error message %s", "Five");
    log_fatal("Fatal message %s", "6");

    /* Topics =========================================== */
    printf("\n");

#if ULOG_FEATURE_TOPICS

    ulog_topic_add("Bluetooth", true);
    ulog_topic_add("Audio", false);
    ulog_topic_add("Serial", false);

    // logt_fatal("Serial", "Serial message 0");

    ulog_topic_enable_all();
    logt_trace("Bluetooth", "Bluetooth message 1");
    logt_debug("Indication", "Indication message 1");
    logt_info("Audio", "Audio message");
    logt_warn("Serial", "Serial message 1");
    logt_error("Serial", "Serial message 2");

    ulog_topic_disable("Serial");
    // logt_warn("Serial", "Serial message 3 (disabled)");

    ulog_level_set(ULOG_LEVEL_INFO);
    ulog_topic_level_set("Bluetooth", ULOG_LEVEL_WARN);
    ulog_topic_level_set("Indication", ULOG_LEVEL_DEBUG);
    logt_info("Bluetooth", "Bluetooth message 2");
    // logt_info("Indication",
    //          "Indication message 2 (level lower than global level)");

#endif  // ULOG_FEATURE_TOPICS

    return 0;
}
