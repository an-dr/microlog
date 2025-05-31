#include "ut_callback.h"
#include <string.h>
#include "ulog.h"

static int processed_message_count                  = 0;
static char last_message_buffer[UT_LOG_BUFFER_SIZE] = {0};

// Custom log callback for tests
void ut_callback(ulog_Event *ev, void *arg) {
    (void)arg;  // Userdata is now 'arg', mark as unused if not used.

    if (!ev || !ev->message) {
        last_message_buffer[0] = '\0';
        return;
    }

    // Clear the last message buffer before writing a new message
    memset(last_message_buffer, 0, sizeof(last_message_buffer));
    vsnprintf(last_message_buffer, sizeof(last_message_buffer), ev->message,
              ev->message_format_args);


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
