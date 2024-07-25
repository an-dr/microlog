#include <stdio.h>
#include "ulog.h"
#include "ilog.h"

int main(int argc, char *argv[])
{
    log_info("Info message");
    log_debug("Debug message");
    log_error("Error message");
    log_warn("Warning message");
    log_fatal("Fatal message");
    
    ilog_info("Info message");
    ilog_debug("Debug message");
    ilog_error("Error message");
    ilog_warn("Warning message");
    ilog_fatal("Fatal message");
    
    return 0;
    
}

long unsigned ulog_get_time(void) {
    return 0;
}
