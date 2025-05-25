#include <stdio.h>
#include "ulog.h"

// Example custom prefix function (optional)
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
void update_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    static int count = 0;
    // Example: Add a millisecond counter to the prefix if custom prefix is enabled
    snprintf(prefix, prefix_size, "[PRE-%03d] ", count++);
}
#endif

// Example custom callback function (optional)
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
void custom_callback(ulog_Event *ev, void *arg) {
    printf("%s", (const char *) arg); // Print the argument passed to ulog_add_callback
    static char buffer[128];
    ulog_event_to_cstr(ev, buffer, sizeof(buffer)); // Format the log event into a string
    printf("%s\n", buffer);
}
#endif

int main(int argc, char *argv[]) {
    (void)argc; // Unused
    (void)argv; // Unused
    
    printf("\n--- Running ulog example with default initialization ---\n");
    // ulog_init(NULL) is called automatically on first log if not called explicitly.
    // This initializes ulog with default values, potentially overridden by compile-time flags.
    // For example, if ULOG_NO_COLOR is defined, color will be disabled.
    // If ULOG_HAVE_TIME is defined, time will be enabled by default.
    log_info("This message uses default/compile-time config.");

    // Demonstrate setting level after default init
    ulog_set_level(LOG_DEBUG);
    log_debug("This is a debug message after setting level to DEBUG.");
    ulog_set_level(LOG_TRACE); // Reset for further examples

    printf("\n--- Demonstrating ulog_init() with a custom configuration ---\n");
    ulog_config_t custom_cfg;
    // Start with current config (which is default + compile-time flags at this point if ulog_log was called)
    // or initialize from scratch if ulog_init was not called yet by a log function.
    // For a clean slate based on library defaults before compile-time flags, you could do:
    // ulog_init(NULL); // ensure defaults are set
    // custom_cfg = g_ulog_config; // then copy. Or manually set all fields.
    
    // Let's manually set all fields for clarity of this example
    custom_cfg.level = LOG_INFO;
    custom_cfg.quiet = false;
    
    // Time: Enable if ULOG_HAVE_TIME is defined, otherwise it will be forced off by ulog_init
#ifdef ULOG_HAVE_TIME
    custom_cfg.time_enabled = true; 
    printf("      (Example custom config: time explicitly enabled - requires ULOG_HAVE_TIME)\n");
#else
    custom_cfg.time_enabled = false;
    printf("      (Example custom config: time explicitly disabled - ULOG_HAVE_TIME not defined)\n");
#endif

    // Color: Disable, unless ULOG_NO_COLOR is defined (which forces it off)
#ifdef ULOG_NO_COLOR
    custom_cfg.color_enabled = false;
    printf("      (Example custom config: color forced off by ULOG_NO_COLOR)\n");
#else
    custom_cfg.color_enabled = false; 
    printf("      (Example custom config: color explicitly disabled)\n");
#endif

    // File String: Keep default (true), unless ULOG_HIDE_FILE_STRING is defined
#ifdef ULOG_HIDE_FILE_STRING
    custom_cfg.file_string_enabled = false;
    printf("      (Example custom config: file string forced off by ULOG_HIDE_FILE_STRING)\n");
#else
    custom_cfg.file_string_enabled = true;
#endif
    
    custom_cfg.short_level_strings = true; // Use 'I' instead of 'INFO'
    custom_cfg.emoji_levels = false;      // Don't use emojis

    // Custom Prefix, Extra Outputs, Topics - demonstrate with compile-time limits
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    custom_cfg.custom_prefix_size = ULOG_CUSTOM_PREFIX_SIZE; // Use full available
#else
    custom_cfg.custom_prefix_size = 0;
#endif
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
    custom_cfg.extra_outputs = ULOG_EXTRA_OUTPUTS; // Use full available
#else
    custom_cfg.extra_outputs = 0;
#endif
#if defined(ULOG_TOPICS_NUM)
    #if ULOG_TOPICS_NUM > 0 // Static
        custom_cfg.topics_num = ULOG_TOPICS_NUM;
        custom_cfg.topics_dynamic_alloc = false;
    #elif ULOG_TOPICS_NUM == -1 // Dynamic
        custom_cfg.topics_num = -1; // Enable dynamic topics
        custom_cfg.topics_dynamic_alloc = true;
    #else // Disabled
        custom_cfg.topics_num = 0;
        custom_cfg.topics_dynamic_alloc = false;
    #endif
#else
    custom_cfg.topics_num = 0;
    custom_cfg.topics_dynamic_alloc = false;
#endif

    ulog_init(&custom_cfg); // Initialize with our custom settings

    printf("--- Logging with custom configuration ---\n");
    log_trace("This TRACE message should NOT be visible (level is INFO).");
    log_debug("This DEBUG message should NOT be visible (level is INFO).");
    log_info("Info message (short levels, no color, time if compiled).");
    log_warn("Warning message (short levels, no color, time if compiled).");

    printf("\n--- Modifying config after ulog_init() using setters ---\n");
    ulog_set_level(LOG_DEBUG); // Change log level
    ulog_set_quiet(true);      // Make it quiet
    log_info("This INFO message should NOT be printed (quiet mode).");
    ulog_set_quiet(false);     // Disable quiet mode
    log_debug("This DEBUG message IS visible (level changed, quiet off).");

    // Setup optional features if compiled
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
    printf("\n--- Setting up Extra Outputs (if ULOG_EXTRA_OUTPUTS > 0) ---\n");
    FILE *fp = fopen("example.log", "w");
    if (fp) {
        ulog_add_fp(fp, LOG_INFO); // Add file output for INFO level and above
        printf("      (example.log will contain INFO messages from now on)\n");
    }
    // Add a custom callback if less than max extra outputs are used by file pointers
    if (g_ulog_config.extra_outputs == 0 || (fp && g_ulog_config.extra_outputs < ULOG_EXTRA_OUTPUTS)) {
         ulog_add_callback(custom_callback, "     - Custom Callback: ", LOG_DEBUG);
         printf("      (Custom callback added for DEBUG messages from now on)\n");
    }
#endif

#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    printf("\n--- Setting Custom Prefix (if ULOG_CUSTOM_PREFIX_SIZE > 0) ---\n");
    ulog_set_prefix_fn(update_prefix);
#endif

    printf("\n--- Core Logging with all features potentially active ---\n");
    // Note: Compile-time flags like ULOG_NO_COLOR or ULOG_HAVE_TIME=0 can prevent
    // color or time from being shown, even if enabled in ulog_config_t.
    // Similarly, ULOG_CUSTOM_PREFIX_SIZE=0 will prevent custom prefix from working.
    log_trace("Trace message %d", 1);
    log_debug("Debug message 0x%x", 2);
    log_info("Info message %f", 3.0); // This will go to example.log if extra outputs active
    log_warn("Warning message %c", '4');
    log_error("Error message %s", "Five");
    log_fatal("Fatal message %s", "6"); // This will go to example.log
    
#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM != 0
    printf("\n--- Topic Logging (if ULOG_TOPICS_NUM is defined and non-zero) ---\n");
    ulog_set_level(LOG_TRACE); // Set global level to TRACE for topic demo

    // Topics are added and enabled/disabled via API.
    // Their initial state (enabled/disabled) can be set when adding.
    // Their log level also defaults to TRACE unless set otherwise.
    if (g_ulog_config.topics_num != 0) { // Check if topics are usable at runtime
        ulog_add_topic("Bluetooth", true); // Add and enable
        ulog_add_topic("Audio", false);    // Add but disable initially
        ulog_add_topic("Serial", true);
        ulog_add_topic("Network", true);
        ulog_set_topic_level("Network", LOG_ERROR); // Only ERROR and FATAL for Network

        logt_trace("Bluetooth", "Bluetooth trace message.");
        logt_info("Audio", "This Audio INFO message should NOT be visible (topic disabled).");
        ulog_enable_topic("Audio");
        logt_info("Audio", "This Audio INFO message IS NOW visible (topic enabled).");
        logt_debug("Serial", "Serial debug message.");
        logt_info("Network", "Network info (SHOULD NOT be visible due to topic level).");
        logt_fatal("Network", "Network fatal (SHOULD be visible).");

        printf("--- Disabling all topics except 'Bluetooth' ---\n");
        ulog_disable_all_topics();
        ulog_enable_topic("Bluetooth");
        logt_info("Bluetooth", "Bluetooth info (all others disabled).");
        logt_info("Audio", "Audio info (SHOULD NOT be visible).");
        logt_info("Serial", "Serial info (SHOULD NOT be visible).");
    } else {
        printf("      (Topic logging disabled at runtime or compile-time)\n");
    }
#endif
    
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
    if (fp) {
        fclose(fp);
    }
#endif
    printf("\nExample finished.\n");
    return 0;
}
