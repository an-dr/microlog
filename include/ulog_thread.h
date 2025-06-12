#ifndef ULOG_THREAD_H
#define ULOG_THREAD_H

#include "ulog_core.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*ulog_LockFn)(bool lock, void *lock_arg);
void ulog_set_lock(ulog_LockFn function, void *lock_arg);

#ifdef __cplusplus
}
#endif

#endif // ULOG_THREAD_H
