# Extensions

The [`extensions`](../extensions/) folder contains optional add-ons that use only the public API. They are intented to enhance the functionality of the core library without modifying its source code and are suppose to be modified or extended independently.

Extensions are not built or installed by default. To use an extension, copy its source file(s) to your project, modify them as needed, and include them in your build system. Some extensions may require specific configuration options to be set.

- [Extensions](#extensions)
    - [ulog\_syslog](#ulog_syslog)

## ulog_syslog

`ulog_syslog` – Provides RFC 5424 style syslog severity names (DEBUG, INFO, NOTICE, WARN, ERR, CRIT, ALERT, EMERG) plus helper macros. Enable it at runtime; no core source modifications required.

Usage:

```c
#include "ulog.h"
#include "ulog_syslog.h"

int main(void) {
    ulog_syslog_enable(ULOG_SYSLOG_STYLE_LONG); // or ULOG_SYSLOG_STYLE_SHORT
    ulog_sl_notice("System starting: version=%d", 1);
    ulog_sl_error("Failure: code=%d", -5);
    ulog_sl_t_debug("Core", "Debugging info");
    ulog_syslog_disable(); // Back to default TRACE..FATAL set
}
```

Output:

```log
NOTICE src/main.c:6: System starting: version=1
ERROR  src/main.c:7: Failure: code=-5
DEBUG  [Core] src/main.c:8: Debugging info
```
