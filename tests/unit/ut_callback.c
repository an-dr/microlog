#include "ut_callback.h"
#include <string.h>
#include "ulog.h"

static int processed_message_count                  = 0;
static char last_message_buffer[UT_LOG_BUFFER_SIZE] = {0};

// Custom log callback for tests
void ut_callback(ulog_event *ev, void *arg) {
    (void)arg;

    if (!ev) {
        last_message_buffer[0] = '\0';
        return;
    }

    memset(last_message_buffer, 0, sizeof(last_message_buffer));
    ulog_event_to_cstr(ev, last_message_buffer, sizeof(last_message_buffer));
    processed_message_count++;
}

// Colour-enabled variant — mirrors a handler that calls ulog_event_to_cstr_colored
void ut_callback_colored(ulog_event *ev, void *arg) {
    (void)arg;

    if (!ev) {
        last_message_buffer[0] = '\0';
        return;
    }

    memset(last_message_buffer, 0, sizeof(last_message_buffer));
    ulog_event_to_cstr_colored(ev, last_message_buffer, sizeof(last_message_buffer));
    processed_message_count++;
}

int ut_callback_get_message_count() {
    return processed_message_count;
}

char *ut_callback_get_last_message() {
    return last_message_buffer;
}

void ut_callback_reset() {
    processed_message_count = 0;
    last_message_buffer[0]  = '\0';  // Clear the last message
}
