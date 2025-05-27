#pragma once

#include "ulog.h"

#define UT_LOG_BUFFER_SIZE 256

void ut_callback(ulog_Event *ev, void *arg);

int ut_callback_get_message_count();
char *ut_callback_get_last_message();
void ut_callback_reset();
