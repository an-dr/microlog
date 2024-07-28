#include <stdio.h>
#include "ulog.h"

int main(int argc, char *argv[])
{
    FILE *fp = fopen("manual_test.log", "w");
    
    ulog_add_fp(fp, LOG_TRACE);
    ulog_add_fp(stdout, LOG_DEBUG);
    log_info("Info message");
    log_debug("Debug message");
    log_error("Error message");
    log_warn("Warning message");
    log_fatal("Fatal message");
    return 0;
    
}


// Time has to be implemented by the user

#include <sys/time.h>

long unsigned ulog_get_time(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (long unsigned)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}
