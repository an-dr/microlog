#ifndef ULOG_PREFIX_H
#define ULOG_PREFIX_H

#include "ulog_core.h"

#ifdef __cplusplus
extern "C" {
#endif
#if FEATURE_CUSTOM_PREFIX
typedef void (*ulog_PrefixFn)(ulog_Event *ev, char *prefix, size_t prefix_size);
void ulog_set_prefix_fn(ulog_PrefixFn function);
#endif
#ifdef __cplusplus
}
#endif

#endif // ULOG_PREFIX_H
