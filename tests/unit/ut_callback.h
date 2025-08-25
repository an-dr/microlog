#pragma once

#include "ulog.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UT_LOG_BUFFER_SIZE 256

void ut_callback(ulog_event *ev, void *arg);

int ut_callback_get_message_count();
char *ut_callback_get_last_message();
void ut_callback_reset();

#ifdef __cplusplus
}
#endif
