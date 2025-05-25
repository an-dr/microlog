#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <time.h> // Required for time_t if ULOG_HAVE_TIME is defined

#include "ulog.h"

// Helper function to check for substring
void check_substring(const char* str, const char* sub, const char* test_name) {
    printf("Checking for '%s' in '%s'\n", sub, str);
    assert(strstr(str, sub) != NULL && test_name);
}

void check_not_substring(const char* str, const char* sub, const char* test_name) {
    printf("Checking for absence of '%s' in '%s'\n", sub, str);
    assert(strstr(str, sub) == NULL && test_name);
}

void test_basic_functionality() {
    printf("Running Test 1: ulog_event_to_cstr basic functionality...\n");
    ulog_Event event;
    char buffer[256];
    memset(&event.message_format_args, 0, sizeof(event.message_format_args)); // Initialize directly

    event.message = "Hello, World!"; // Pre-formatted or no-format string
    event.file = "test_formatting.c";
    event.level = LOG_INFO;
    event.line = __LINE__;
    // event.tag = "BasicTest"; // Tag is not part of ulog_Event directly
#ifdef ULOG_HAVE_TIME
    event.time = NULL; // No timestamp for basic test unless ULOG_HAVE_TIME is forced
#endif

    ulog_event_to_cstr(&event, buffer, sizeof(buffer));

    // Check for level string (default is full)
    check_substring(buffer, "[INFO]", "Test 1: Level string");
    // Check for tag - this test will change as tag is not directly set.
    // If no topic is set, no tag string (e.g. "[BasicTest]") should appear.
    // Let's assume for default tests, we don't check for a specific tag unless topics are involved.
    check_not_substring(buffer, "[BasicTest]", "Test 1: Tag string should be absent by default");
    // Check for message
    check_substring(buffer, "Hello, World!", "Test 1: Message content");
    // Check for file and line (default is present)
    char file_line_info[64];
    sprintf(file_line_info, "test_formatting.c:%d", event.line);
    check_substring(buffer, file_line_info, "Test 1: File and line");

    printf("Test 1: Passed.\n\n");
}

#ifdef ULOG_HAVE_TIME
void test_time_formatting() {
    printf("Running Test 2: Time formatting (ULOG_HAVE_TIME)...\n");
    ulog_Event event;
    char buffer[256];
    time_t current_time = time(NULL); // Get current time for the event
    // va_list is already part of the event, just ensure it's zeroed if not used with actual args
    memset(&event.message_format_args, 0, sizeof(event.message_format_args));


    event.message = "Time test: 123"; // Pre-formatted
    event.file = "test_formatting.c";
    event.level = LOG_DEBUG;
    event.line = __LINE__;
    // event.tag = "TimeTest";
    event.time = localtime(&current_time); // ulog_Event expects struct tm*

    ulog_event_to_cstr(&event, buffer, sizeof(buffer));

    // Check for a timestamp pattern, e.g., "HH:MM:SS"
    // This is a simple check; a more robust check might use regex or parse the date
    // For now, we check for colons which are typical in time formats.
    // Example: "01:23:45"
    char time_str_part[10];
    strftime(time_str_part, sizeof(time_str_part), "%H:%M:%S", localtime(&current_time));
    check_substring(buffer, time_str_part, "Test 2: Timestamp presence");
    
    check_substring(buffer, "[DEBUG]", "Test 2: Level string");
    check_not_substring(buffer, "[TimeTest]", "Test 2: Tag string should be absent by default");
    check_substring(buffer, "Time test: 123", "Test 2: Message content");

    printf("Test 2: Passed.\n\n");
}
#endif

void test_file_string_presence() {
    printf("Running Test 3a: File string presence (default)...\n");
    ulog_Event event;
    char buffer[256];
    memset(&event.message_format_args, 0, sizeof(event.message_format_args));

    event.message = "File string test";
    event.file = "test_formatting.c"; // Intentionally using the current file name
    event.level = LOG_WARN;
    event.line = 123; // Dummy line number
    // event.tag = "FileStrTest";
#ifdef ULOG_HAVE_TIME
    event.time = NULL;
#endif

    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    
    char file_line_info[64];
    sprintf(file_line_info, "%s:%d", event.file, event.line);
    check_substring(buffer, file_line_info, "Test 3a: File and line info");
    printf("Test 3a: Passed.\n\n");
}

#ifdef ULOG_HIDE_FILE_STRING
void test_file_string_absence() {
    printf("Running Test 3b: File string absence (ULOG_HIDE_FILE_STRING)...\n");
    ulog_Event event;
    char buffer[256];
    memset(&event.message_format_args, 0, sizeof(event.message_format_args));

    event.message = "No file string test";
    event.file = "anyfile.c"; // This should not appear
    event.level = LOG_ERROR;
    event.line = 456;    // This should not appear
    // event.tag = "NoFileStrTest";
#ifdef ULOG_HAVE_TIME
    event.time = NULL;
#endif

    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    
    check_not_substring(buffer, "anyfile.c:456", "Test 3b: File and line info hidden");
    check_substring(buffer, "[ERROR]", "Test 3b: Level string");
    check_not_substring(buffer, "[NoFileStrTest]", "Test 3b: Tag string should be absent");
    check_substring(buffer, "No file string test", "Test 3b: Message content");
    printf("Test 3b: Passed.\n\n");
}
#endif

#ifdef ULOG_SHORT_LEVEL_STRINGS
void test_short_level_strings() {
    printf("Running Test 4: Short level strings (ULOG_SHORT_LEVEL_STRINGS)...\n");
    ulog_Event event;
    char buffer[256];
    memset(&event.message_format_args, 0, sizeof(event.message_format_args));

    event.message = "Short level test";
    event.file = "test_formatting.c";
    event.level = LOG_INFO;
    event.line = __LINE__;
    // event.tag = "ShortLevel";
#ifdef ULOG_HAVE_TIME
    event.time = NULL;
#endif

    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    check_substring(buffer, "[I]", "Test 4: Short INFO string"); // "I" for INFO

    event.level = LOG_WARN; // Keep other fields same, just change level
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    check_substring(buffer, "[W]", "Test 4: Short WARN string"); // "W" for WARN
    
    printf("Test 4: Passed.\n\n");
}
#endif

#ifdef ULOG_USE_EMOJI
void test_emoji_level_strings() {
    printf("Running Test 5: Emoji level strings (ULOG_USE_EMOJI)...\n");
    ulog_Event event;
    char buffer[256];
    memset(&event.message_format_args, 0, sizeof(event.message_format_args));

    event.message = "Emoji level test";
    event.file = "test_formatting.c";
    event.level = LOG_INFO;
    event.line = __LINE__;
    // event.tag = "EmojiLevel";
#ifdef ULOG_HAVE_TIME
    event.time = NULL;
#endif

    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    // Exact emoji characters can be tricky with source file encodings.
    // ulog.h uses UTF-8 strings like "ℹ️" for INFO.
    // We'll check for a known part of the multi-byte sequence if direct string compare is problematic.
    // For simplicity, we assume the string literals from ulog.h are correctly handled.
    // Actual emojis from ulog.c: "ℹ️ INFO", "🔥 ERROR"
    // The ulog_event_to_cstr function formats the full string.
    // We should check for the emoji itself, which is part of the level string.
    check_substring(buffer, "ℹ️", "Test 5: Emoji INFO string");

    event.level = LOG_ERROR; // Keep other fields same
    ulog_event_to_cstr(&event, buffer, sizeof(buffer));
    check_substring(buffer, "🔥", "Test 5: Emoji ERROR string");
    
    printf("Test 5: Passed.\n\n");
}
#endif

void test_no_color_output() {
    printf("Running Test 6: No color output in ulog_event_to_cstr...\n");
    ulog_Event event;
    char buffer[256];
    memset(&event.message_format_args, 0, sizeof(event.message_format_args));

    event.message = "Color test message";
    event.file = "test_formatting.c";
    event.level = LOG_WARN; // WARN messages are typically yellow if color is enabled
    event.line = __LINE__;
    // event.tag = "ColorTest";
#ifdef ULOG_HAVE_TIME
    event.time = NULL;
#endif

    ulog_event_to_cstr(&event, buffer, sizeof(buffer));

    // Check that the ANSI escape code for yellow is NOT present
    check_not_substring(buffer, "\x1b[33m", "Test 6: No yellow ANSI code for WARN");
    // Check that the general ANSI escape prefix is NOT present
    check_not_substring(buffer, "\x1b[", "Test 6: No ANSI escape codes generally");
    // Ensure the message is still there
    check_substring(buffer, "Color test message", "Test 6: Message content present");

    printf("Test 6: Passed.\n\n");
}


int main() {
    printf("Starting unit tests for microlog formatting...\n\n");

    test_basic_functionality();
    test_file_string_presence(); // Test default behavior (file string present)
    test_no_color_output();

#ifdef ULOG_HAVE_TIME
    test_time_formatting();
#else
    printf("Skipping Test 2: Time formatting (ULOG_HAVE_TIME not defined).\n\n");
#endif

#ifdef ULOG_HIDE_FILE_STRING
    test_file_string_absence();
#else
    printf("Skipping Test 3b: File string absence (ULOG_HIDE_FILE_STRING not defined).\n\n");
#endif

#ifdef ULOG_SHORT_LEVEL_STRINGS
    test_short_level_strings();
#else
    printf("Skipping Test 4: Short level strings (ULOG_SHORT_LEVEL_STRINGS not defined).\n\n");
#endif

#ifdef ULOG_USE_EMOJI
    test_emoji_level_strings();
#else
    printf("Skipping Test 5: Emoji level strings (ULOG_USE_EMOJI not defined).\n\n");
#endif

    printf("All formatting tests completed!\n");
    return 0;
}
