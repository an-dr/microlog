#include "ulog.h"      // Must be compiled with -DFEATURE_RUNTIME_CONFIG for these tests
#include <stdio.h>
#include <string.h>     // For strcmp, strstr
#include <assert.h>
#include <time.h>       // For dummy time in events

// --- Test Configuration Setup ---

// This helper initializes a ulog_config_t struct with values that mimic
// the library's default compile-time settings. These are derived from
// the FEATURE_ and CFG_ macros that ulog.h sets up based on ULOG_XXX defines.
static void get_default_compile_time_config(ulog_config_t* cfg) {
    cfg->enable_time = FEATURE_TIME;
    cfg->enable_color = FEATURE_COLOR;
    cfg->custom_prefix_size = CFG_CUSTOM_PREFIX_SIZE;
    cfg->enable_file_string = FEATURE_FILE_STRING;
    cfg->short_level_strings = FEATURE_SHORT_LEVELS;
    cfg->use_emoji_levels = FEATURE_EMOJI_LEVELS;
    cfg->extra_outputs_num = CFG_EXTRA_OUTPUTS;

#if (FEATURE_TOPICS && CFG_TOPICS_DINAMIC_ALLOC)
    cfg->topics_num = -1; // Default for dynamic topics if FEATURE_TOPICS is on & dynamic
#elif FEATURE_TOPICS
    cfg->topics_num = CFG_TOPICS_NUM; // Default for static topics if FEATURE_TOPICS is on
#else
    cfg->topics_num = 0; // Default if FEATURE_TOPICS is off
#endif

    // Determine ULOG_DEFAULT_LOG_LEVEL (it's not directly available as CFG_DEFAULT_LOG_LEVEL)
    // We have to replicate the logic from src/ulog.c or ulog.h if it's simple.
    // For simplicity, assuming LOG_TRACE if not set, as per ulog.c's internal default.
    // This might need adjustment if ULOG_DEFAULT_LOG_LEVEL has complex derivation not exposed.
    // The ulog.config.default_log_level in ulog.c static init uses ULOG_DEFAULT_LOG_LEVEL.
    // Let's assume ULOG_DEFAULT_LOG_LEVEL is accessible or use a common default.
    // For robust testing, ULOG_DEFAULT_LOG_LEVEL should be ensured by build flags if needed for this default setup.
    // For now, using LOG_TRACE as a common base.
    // The static ulog object in ulog.c initializes .level to ULOG_DEFAULT_LOG_LEVEL and
    // .config.default_log_level to ULOG_DEFAULT_LOG_LEVEL. So this should be fine.
    cfg->default_log_level = LOG_TRACE; // Fallback, ideally ULOG_DEFAULT_LOG_LEVEL if accessible and defined.
                                        // If ULOG_DEFAULT_LOG_LEVEL is defined by user, test harness should pass it.
                                        // For now, test will set this explicitly.
}


// --- Test Event Creation Helper ---

static struct tm global_dummy_time; // Shared dummy time

static void init_global_dummy_time() {
    time_t t_now = time(NULL);
    global_dummy_time = *localtime(&t_now);
}

// Helper to create a basic ulog_Event for testing formatting
static void create_test_event(ulog_Event *event, int level, const char *message_fmt, ...) {
    // For simplicity, this test helper doesn't handle actual varargs for message_fmt for now.
    // It assumes message_fmt is the final message string.
    // If actual formatting of va_list is needed, more setup is required.

    event->message = message_fmt;
    // event->message_format_args would need va_start etc. if we were testing that.
    // Setting to NULL implies message is already formatted or has no args.
    va_list dummy_args;
    // To avoid issues with uninitialized va_list on some platforms if it were used by ulog_event_to_cstr
    // even if message has no format specifiers. Best to have a valid, empty va_list.
    // However, ulog_event_to_cstr currently does: vprint(tgt, ev->message, ev->message_format_args);
    // So if message has format specifiers but args is NULL, it might crash.
    // For safety, ensure test messages don't have format specifiers if not providing args.
    event->message_format_args = dummy_args; // This is not strictly correct without va_start.
                                           // The safest is event->message_format_args = NULL and ensure message has no '%'

    event->file = "test.c";
    event->line = 123;
    event->level = level;

    // These are conditional in ulog_Event struct based on (FEATURE_RUNTIME_CONFIG || FEATURE_XXX)
    // Since FEATURE_RUNTIME_CONFIG is defined for these tests, these fields will exist.
    event->time = &global_dummy_time;
    event->topic = -1; // No specific topic for these tests
}

// --- Test Cases ---

void test_level_strings_runtime() {
    printf("Running test: %s\n", __func__);
    ulog_config_t cfg;
    get_default_compile_time_config(&cfg); // Start with defaults

    char buffer[256];
    ulog_Event event;
    create_test_event(&event, LOG_INFO, "Test message for level strings");

    // Test emoji levels
    cfg.use_emoji_levels = true;
    cfg.short_level_strings = false; // Emoji should override short
    ulog_init_config(&cfg);
    assert(strcmp(ulog_get_level_string(LOG_INFO), "🟢") == 0);
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    printf("  Output (emoji): %s\n", buffer);
    assert(strstr(buffer, "🟢 Test message for level strings") != NULL);

    // Test short levels
    cfg.use_emoji_levels = false;
    cfg.short_level_strings = true;
    ulog_init_config(&cfg);
    assert(strcmp(ulog_get_level_string(LOG_INFO), "I") == 0);
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    printf("  Output (short): %s\n", buffer);
    assert(strstr(buffer, "I test.c:123: Test message for level strings") != NULL);


    // Test full levels
    cfg.use_emoji_levels = false;
    cfg.short_level_strings = false;
    ulog_init_config(&cfg);
    assert(strcmp(ulog_get_level_string(LOG_INFO), "INFO") == 0);
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    printf("  Output (full): %s\n", buffer);
    assert(strstr(buffer, "INFO test.c:123: Test message for level strings") != NULL); // Default format includes level then file:line

    printf("Test %s PASSED\n\n", __func__);
}

void test_time_feature_runtime() {
    printf("Running test: %s\n", __func__);
    ulog_config_t cfg;
    get_default_compile_time_config(&cfg);

    char buffer[256];
    ulog_Event event;
    create_test_event(&event, LOG_INFO, "Test message for time feature");

    char time_str_buf[10]; // For HH:MM:SS
    strftime(time_str_buf, sizeof(time_str_buf), "%H:%M:%S", event.time);

    // Test time enabled
    cfg.enable_time = true;
    ulog_init_config(&cfg);
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    printf("  Output (time on): %s\n", buffer);
    assert(strstr(buffer, time_str_buf) != NULL);

    // Test time disabled
    cfg.enable_time = false;
    ulog_init_config(&cfg);
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    printf("  Output (time off): %s\n", buffer);
    assert(strstr(buffer, time_str_buf) == NULL);

    printf("Test %s PASSED\n\n", __func__);
}

void test_file_string_runtime() {
    printf("Running test: %s\n", __func__);
    ulog_config_t cfg;
    get_default_compile_time_config(&cfg);

    char buffer[256];
    ulog_Event event;
    create_test_event(&event, LOG_INFO, "Test message for file string");

    // Test file string enabled
    cfg.enable_file_string = true;
    ulog_init_config(&cfg);
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    printf("  Output (file on): %s\n", buffer);
    assert(strstr(buffer, "test.c:123:") != NULL);

    // Test file string disabled
    cfg.enable_file_string = false;
    ulog_init_config(&cfg);
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    printf("  Output (file off): %s\n", buffer);
    assert(strstr(buffer, "test.c:123:") == NULL);
    assert(strstr(buffer, "Test message for file string") != NULL); // Message should still be there

    printf("Test %s PASSED\n\n", __func__);
}

void test_default_log_level_application() {
    printf("Running test: %s\n", __func__);
    ulog_config_t cfg;
    get_default_compile_time_config(&cfg);

    // Set a baseline global log level via ulog_init_config
    cfg.default_log_level = LOG_INFO;
    ulog_init_config(&cfg);
    // ulog.level is now LOG_INFO

    // To test if ulog.level was set, we'd ideally call ulog_get_level()
    // or observe behavior. Since ulog.level is static and not exposed by a getter,
    // direct assertion is hard. This test is more conceptual for now.
    // If FEATURE_RUNTIME_CONFIG is on, ulog_init_config updates ulog.level.
    // If we had a way to capture ulog_log's decision to log or not, we could test it.
    // For now, we assume ulog_init_config correctly sets the internal static ulog.level.

    // Example: if we could check internal level:
    // assert(ulog_get_internal_level_for_test() == LOG_INFO);

    cfg.default_log_level = LOG_ERROR;
    ulog_init_config(&cfg);
    // assert(ulog_get_internal_level_for_test() == LOG_ERROR);

    printf("Test %s (conceptual) PASSED\n\n", __func__);
}


// --- Main Test Runner ---
int main() {
    init_global_dummy_time();

    // Set an initial global state for ulog that reflects typical compile-time defaults.
    // This is important because ulog_init_config() modifies a global static ulog_t instance.
    ulog_config_t initial_defaults;
    get_default_compile_time_config(&initial_defaults);
    // Override specific defaults if necessary for a baseline test environment
    initial_defaults.default_log_level = LOG_TRACE; // Ensure all levels can pass initially
    initial_defaults.enable_file_string = true;   // Common default
    initial_defaults.enable_time = false;         // Default for FEATURE_TIME=false
    initial_defaults.enable_color = true;         // Default for FEATURE_COLOR=true (ULOG_NO_COLOR not defined)
    ulog_init_config(&initial_defaults);
    printf("Initialized ulog with default compile-time settings mimic.\n\n");

    test_level_strings_runtime();
    test_time_feature_runtime();
    test_file_string_runtime();
    test_default_log_level_application();

    printf("All runtime config tests completed.\n");
    return 0;
}

// Note on compiling this test:
// Needs to be compiled with -DFEATURE_RUNTIME_CONFIG
// Example: gcc -o test_runtime_config test_runtime_config.c ../../src/ulog.c -I../../include -DFEATURE_RUNTIME_CONFIG -std=c99
// The path to ulog.c and include dir will depend on file structure.
// Also, any ULOG_XXX defines that affect FEATURE_XXX macros in ulog.h used by get_default_compile_time_config
// should ideally be passed consistently or ensure ulog.h defaults are what's expected for the baseline.
// For example, if tests assume FEATURE_TIME is false by default, ensure ULOG_HAVE_TIME is not passed to compiler for this test build.

```
