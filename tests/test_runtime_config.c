#include "ulog.h"      // Include the header for the library being tested
#include <stdio.h>     // For printf
#include <assert.h>    // For assert
#include <string.h>    // For strcmp, strstr
#include <stdbool.h>   // For bool type

// Make g_ulog_config accessible in this test file for white-box testing
// This relies on g_ulog_config being non-static in ulog.c
extern ulog_config_t g_ulog_config;

// Helper to reset ulog's initialized state for isolated tests
// This is a bit of a hack; ideally, ulog would have a deinit or reset function.
// For now, we'll re-initialize. This also means `s_ulog_initialized` in ulog.c
// should not prevent re-initialization if a config is passed.
// The current ulog_init handles this: if (s_ulog_initialized && config != NULL) allows re-init.
void reset_ulog_for_test() {
    ulog_config_t empty_cfg = {0}; // Minimal config to force re-init path
    ulog_init(&empty_cfg); // Apply some known state
    ulog_init(NULL);       // Then apply defaults
}

// Helper function to compare two ulog_config_t structures
bool are_configs_equal(const ulog_config_t* cfg1, const ulog_config_t* cfg2) {
    if (cfg1->level != cfg2->level) return false;
    if (cfg1->quiet != cfg2->quiet) return false;
    if (cfg1->time_enabled != cfg2->time_enabled) return false;
    if (cfg1->color_enabled != cfg2->color_enabled) return false;
    if (cfg1->custom_prefix_size != cfg2->custom_prefix_size) return false;
    if (cfg1->file_string_enabled != cfg2->file_string_enabled) return false;
    if (cfg1->short_level_strings != cfg2->short_level_strings) return false;
    if (cfg1->emoji_levels != cfg2->emoji_levels) return false;
    if (cfg1->extra_outputs != cfg2->extra_outputs) return false;
    if (cfg1->topics_num != cfg2->topics_num) return false;
    if (cfg1->topics_dynamic_alloc != cfg2->topics_dynamic_alloc) return false;
    return true;
}

void print_config(const char* title, const ulog_config_t* cfg) {
    printf("--- %s ---\n", title);
    printf("  level: %d\n", cfg->level);
    printf("  quiet: %s\n", cfg->quiet ? "true" : "false");
    printf("  time_enabled: %s\n", cfg->time_enabled ? "true" : "false");
    printf("  color_enabled: %s\n", cfg->color_enabled ? "true" : "false");
    printf("  custom_prefix_size: %d\n", cfg->custom_prefix_size);
    printf("  file_string_enabled: %s\n", cfg->file_string_enabled ? "true" : "false");
    printf("  short_level_strings: %s\n", cfg->short_level_strings ? "true" : "false");
    printf("  emoji_levels: %s\n", cfg->emoji_levels ? "true" : "false");
    printf("  extra_outputs: %d\n", cfg->extra_outputs);
    printf("  topics_num: %d\n", cfg->topics_num);
    printf("  topics_dynamic_alloc: %s\n", cfg->topics_dynamic_alloc ? "true" : "false");
    printf("-------------------------\n");
}


void test_default_initialization() {
    printf("Running test_default_initialization...\n");
    
    // Ensure a clean state before this test by re-initializing.
    // ulog_init(NULL) will set s_ulog_initialized to true.
    // To force re-evaluation of defaults for *this specific test*, we need a way to reset s_ulog_initialized
    // or ensure ulog_init can be called multiple times to reset.
    // The current ulog_init logic: if (s_ulog_initialized && config == NULL) return;
    // So, we can't just call ulog_init(NULL) again if it's already initialized to get a fresh default setup.
    // This is a limitation for testing. For now, we assume this is the first init or rely on global state.
    // A better approach would be to have a ulog_deinit() or allow ulog_init(NULL) to always reset to defaults.
    // To ensure a clean state and that ulog_init(NULL) applies defaults based on compile flags:
    // 1. Initialize with a dummy config (if ulog was already initialized by a previous test).
    //    This ensures the subsequent ulog_init(NULL) isn't a no-op.
    // 2. Call ulog_init(NULL).
    ulog_config_t dummy_cfg = {0}; 
    ulog_init(&dummy_cfg); 
    ulog_init(NULL);       // This should now reflect compile-time flags over base defaults.
    
    ulog_config_t expected_cfg;
    // Base defaults (before compile-time flags are considered by ulog_init)
    expected_cfg.level = LOG_TRACE; 
    expected_cfg.quiet = false;

#ifdef ULOG_HAVE_TIME
    expected_cfg.time_enabled = true;
#else
    expected_cfg.time_enabled = false;
#endif
#ifdef ULOG_NO_COLOR
    expected_cfg.color_enabled = false;
#else
    expected_cfg.color_enabled = true;
#endif
#if defined(ULOG_CUSTOM_PREFIX_SIZE) && ULOG_CUSTOM_PREFIX_SIZE > 0
    expected_cfg.custom_prefix_size = ULOG_CUSTOM_PREFIX_SIZE;
#else
    expected_cfg.custom_prefix_size = 0;
#endif
#ifdef ULOG_HIDE_FILE_STRING
    expected_cfg.file_string_enabled = false;
#else
    expected_cfg.file_string_enabled = true;
#endif
#ifdef ULOG_SHORT_LEVEL_STRINGS
    expected_cfg.short_level_strings = true;
#else
    expected_cfg.short_level_strings = false;
#endif
#ifdef ULOG_USE_EMOJI
    expected_cfg.emoji_levels = true;
    expected_cfg.short_level_strings = false; // Emoji overrides short
#else
    expected_cfg.emoji_levels = false;
#endif
#if defined(ULOG_EXTRA_OUTPUTS) && ULOG_EXTRA_OUTPUTS > 0
    expected_cfg.extra_outputs = ULOG_EXTRA_OUTPUTS;
#else
    expected_cfg.extra_outputs = 0;
#endif
#ifdef ULOG_TOPICS_NUM
    #if ULOG_TOPICS_NUM > 0
        expected_cfg.topics_num = ULOG_TOPICS_NUM;
        expected_cfg.topics_dynamic_alloc = false;
    #elif ULOG_TOPICS_NUM == -1
        expected_cfg.topics_num = -1;
        expected_cfg.topics_dynamic_alloc = true;
    #else
        expected_cfg.topics_num = 0;
        expected_cfg.topics_dynamic_alloc = false;
    #endif
#else
    expected_cfg.topics_num = 0;
    expected_cfg.topics_dynamic_alloc = false;
#endif
    
    //print_config("Actual Default Config", &g_ulog_config);
    //print_config("Expected Default Config", &expected_cfg);
    assert(are_configs_equal(&g_ulog_config, &expected_cfg));

    // Behavioral check for default log level (TRACE)
    // This requires a way to capture output or check a side effect.
    // Using ulog_event_to_cstr to verify formatting.
    char buffer[256];
    ulog_Event ev_info = { .level = LOG_INFO, .file = "test.c", .line = 100, .message = "Info test" };
    ulog_event_to_cstr(&ev_info, buffer, sizeof(buffer));
    
    // Check for color (if expected)
#if !defined(ULOG_NO_COLOR)
    assert(strstr(buffer, "\x1b[") == NULL); // ulog_event_to_cstr should NOT produce color by default
#endif
    // Check for time (if expected and enabled by default)
#if defined(ULOG_HAVE_TIME)
     // Default ulog_event_to_cstr uses ULOG_TIME_SHORT, which implies time if g_ulog_config.time_enabled is true
    if (expected_cfg.time_enabled) {
      assert(strstr(buffer, ":") != NULL); // Basic check for time format HH:MM:SS
    } else {
      assert(strstr(buffer, ":") == NULL); // Should not have time if not enabled
    }
#else 
    // If ULOG_HAVE_TIME is not defined, time should not be there regardless of config
    assert(strstr(buffer, ":") == NULL);
#endif

    printf("PASS: test_default_initialization\n");
}

void test_custom_initialization() {
    printf("Running test_custom_initialization...\n");
    
    ulog_config_t custom_cfg;
    custom_cfg.level = LOG_WARN;
    custom_cfg.quiet = false; 
    custom_cfg.color_enabled = false; // Try to disable color
#ifdef ULOG_HAVE_TIME
    custom_cfg.time_enabled = true;   // Try to enable time
#else
    custom_cfg.time_enabled = false;  // Respect compile-time absence
#endif
    custom_cfg.custom_prefix_size = 0;
    custom_cfg.file_string_enabled = false; // Try to disable file string
    custom_cfg.short_level_strings = true;  // Try to enable short strings
    custom_cfg.emoji_levels = false;
    custom_cfg.extra_outputs = 0;
#if defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM > 0 // Example: use half of static topics if available
    custom_cfg.topics_num = ULOG_TOPICS_NUM / 2 > 0 ? ULOG_TOPICS_NUM / 2 : 0; 
    custom_cfg.topics_dynamic_alloc = false;
#elif defined(ULOG_TOPICS_NUM) && ULOG_TOPICS_NUM == -1 // Dynamic
    custom_cfg.topics_num = 5; // Arbitrary number for dynamic, could be -1 as well
    custom_cfg.topics_dynamic_alloc = true;
#else // Disabled
    custom_cfg.topics_num = 0;
    custom_cfg.topics_dynamic_alloc = false;
#endif

    ulog_init(&custom_cfg);

    // Verify g_ulog_config fields, respecting compile-time limitations
    assert(g_ulog_config.level == LOG_WARN);
    assert(g_ulog_config.quiet == false);
#ifdef ULOG_NO_COLOR
    assert(g_ulog_config.color_enabled == false); // Forced by compile flag
#else
    assert(g_ulog_config.color_enabled == false); // Set by custom_cfg
#endif
#ifdef ULOG_HAVE_TIME
    assert(g_ulog_config.time_enabled == true);  // Set by custom_cfg
#else
    assert(g_ulog_config.time_enabled == false); // Forced by compile flag
#endif
#ifdef ULOG_HIDE_FILE_STRING
    assert(g_ulog_config.file_string_enabled == false); // Forced by compile flag
#else
    assert(g_ulog_config.file_string_enabled == false); // Set by custom_cfg
#endif
    assert(g_ulog_config.short_level_strings == true);


    // Behavioral tests using ulog_event_to_cstr
    char buffer[256];
    ulog_Event ev_warn = { .level = LOG_WARN, .file = "test.c", .line = 200, .message = "Warn test" };
    
    // ulog_event_to_cstr formats without color and with short time by default.
    // Its behavior is modified by g_ulog_config for file_string, short_levels, emoji_levels.
    // The color/time enabled flags in g_ulog_config primarily affect direct stdout/file logging,
    // but ulog_event_to_cstr has its own defaults for color/time formatting (off/short).
    
    // Behavioral tests using ulog_event_to_cstr
    char buffer[256];
    // Create a dummy event. The content of message/file/line for this specific test
    // is less important than how the config settings affect the output string.
    ulog_Event ev_warn = { .level = LOG_WARN, .file = "test_file.c", .line = 123, .message = "Test Msg" };
    
    // Reset ulog to ensure the custom_cfg is freshly applied by the next log call (if auto-initing)
    // or by an explicit ulog_init.
    // We call ulog_init directly, so s_ulog_initialized will be true.
    ulog_init(&custom_cfg); 
                                       
    ulog_event_to_cstr(&ev_warn, buffer, sizeof(buffer));
    //printf("Custom init - ulog_event_to_cstr output: %s\n", buffer);

#ifdef ULOG_HAVE_TIME
    if (custom_cfg.time_enabled) { // Time was attempted to be enabled
      if (g_ulog_config.time_enabled) { // And successfully enabled (not overridden by compile-time ULOG_HAVE_TIME=0)
          assert(strstr(buffer, ":") != NULL); // Check for time (HH:MM:SS from ulog_event_to_cstr's default short time)
      } else { // Time was attempted but compile flag prevented it
          assert(strstr(buffer, ":") == NULL);
      }
    } else { // Time was explicitly disabled in custom_cfg
        assert(strstr(buffer, ":") == NULL);
    }
#else // ULOG_HAVE_TIME not defined
    assert(strstr(buffer, ":") == NULL); // No time if not compiled
#endif

    // ulog_event_to_cstr by default does not add color.
    // g_ulog_config.color_enabled being false ensures no color in direct console output.
    assert(strstr(buffer, "\x1b[") == NULL); 
    
    // file_string_enabled was set to false in custom_cfg
#ifdef ULOG_HIDE_FILE_STRING // If file string is compiled out, it must be false
    assert(strstr(buffer, "test_file.c:123") == NULL);
#else // Otherwise, respect custom_cfg
    assert(strstr(buffer, "test_file.c:123") == NULL);
#endif
    
    // short_level_strings was set to true
    assert(strstr(buffer, "W Test Msg") != NULL); // "W" for WARN, then message (no file/line)

    // Test quiet mode (behavioral)
    // This requires actual logging. We'll temporarily redirect stdout for a more robust check.
    // Note: This is a simplified stdout redirection for testing purposes.
    // A more robust solution would use platform-specific pipe redirection or a testing framework.
    ulog_set_quiet(true);
    // To check quiet mode, we'd ideally capture stdout.
    // For now, we assume if ulog_set_quiet(true) is called, subsequent log calls are quiet.
    // A simple behavioral hint:
    printf("  (Quiet mode test: Next WARN log should NOT appear on console if quiet mode works)\n");
    log_warn("Custom init test: Warn message (SHOULD BE QUIET)."); 
    // If we could capture stdout, we'd assert its content is empty here.
    ulog_set_quiet(false); 
    // log_warn("Custom init test: Warn message (NOT QUIET - SHOULD APPEAR)."); // This would appear

    printf("PASS: test_custom_initialization\n");
}

void test_specific_compile_flags() {
    printf("Running test_specific_compile_flags...\n");
    ulog_config_t temp_cfg;

#if defined(ULOG_NO_COLOR) && defined(TEST_ULOG_NO_COLOR)
    printf("  Testing with ULOG_NO_COLOR defined...\n");
    ulog_init(NULL); // Initialize with defaults, ULOG_NO_COLOR should apply
    assert(g_ulog_config.color_enabled == false);
    
    temp_cfg = g_ulog_config;
    temp_cfg.color_enabled = true; // Try to enable color via custom config
    ulog_init(&temp_cfg);
    assert(g_ulog_config.color_enabled == false); // Should remain false due to ULOG_NO_COLOR
    printf("  PASS: ULOG_NO_COLOR respected.\n");
#elif !defined(ULOG_HAVE_TIME) && defined(TEST_ULOG_NO_TIME)
    printf("  Testing with ULOG_HAVE_TIME NOT defined...\n");
    ulog_init(NULL);
    assert(g_ulog_config.time_enabled == false);

    temp_cfg = g_ulog_config;
    temp_cfg.time_enabled = true; // Try to enable time
    ulog_init(&temp_cfg);
    assert(g_ulog_config.time_enabled == false); // Should remain false
    printf("  PASS: ULOG_HAVE_TIME not defined respected.\n");
#else
    printf("  (No specific compile flag combination for this test run, or TEST_... flag not set)\n");
#endif
    printf("Finished test_specific_compile_flags.\n");
}


int main() {
    printf("Starting ulog runtime configuration tests...\n\n");
    
    test_default_initialization();
    printf("\n");
    test_custom_initialization();
    printf("\n");
    
    // Instructions for running compile-flag specific tests:
    // 1. Compile this test normally (no extra TEST_ flags):
    //    gcc -I../include -L../build -o test_runtime_config test_runtime_config.c ../build/libmicrolog.a -pthread
    //    This will run default tests. test_specific_compile_flags will note no specific flags.
    // 2. To test ULOG_NO_COLOR interaction:
    //    Recompile ulog lib with: cmake .. -DULOG_NO_COLOR=ON && make
    //    Recompile test with: gcc -I../include -L../build -o test_no_color test_runtime_config.c ../build/libmicrolog.a -DTEST_ULOG_NO_COLOR -DULOG_NO_COLOR
    //    Run ./test_no_color
    // 3. To test ULOG_HAVE_TIME=OFF interaction:
    //    Recompile ulog lib with: cmake .. -DULOG_HAVE_TIME=OFF && make
    //    Recompile test with: gcc -I../include -L../build -o test_no_time test_runtime_config.c ../build/libmicrolog.a -DTEST_ULOG_NO_TIME 
    //    Run ./test_no_time
    // Note: The test file itself also needs the ULOG_NO_COLOR or lack of ULOG_HAVE_TIME for its own #ifdefs
    // if these macros directly affect test logic beyond just library behavior.
    // The current test_specific_compile_flags checks these defines.
    test_specific_compile_flags();


    printf("\nAll tests finished.\n");
    return 0;
}
