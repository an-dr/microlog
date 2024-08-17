#include <stdio.h>
#include "ulog.h"


void update_prefix(ulog_Event *ev, char *prefix, size_t prefix_size) {
    static int count = 0;
    snprintf(prefix, prefix_size, "[Custom: %d ms]", count++);
}


int main(int argc, char *argv[])
{
    
    ulog_set_level(LOG_TRACE);

#if FEATURE_EXTRA_DESTS
    printf("Extra destinations: %d\n", ULOG_EXTRA_DESTINATIONS);
    FILE *fp = fopen("manual_test.log", "w");
    ulog_add_fp(fp, LOG_TRACE);
    ulog_add_fp(stdout, LOG_ERROR);
#endif
    
#if FEATURE_CUSTOM_PREFIX
    ulog_set_prefix_fn(update_prefix);
#endif
    
    log_trace("Trace message");
    log_debug("Debug message");
    log_info("Info message");
    log_warn("Warning message");
    log_error("Error message");
    log_fatal("Fatal message");
    return 0;
    
}
