<!-- Extensions documentation -->

# Microlog Extensions

This document describes optional extensions that live under the `extensions/` folder. They are **opt-in**: nothing is compiled into the core unless you explicitly add the corresponding source file(s) to your build.

## Contents

- [Microlog Extensions](#microlog-extensions)
    - [Contents](#contents)
    - [Level Extensions](#level-extensions)
        - [Syslog Levels (`ulog_syslog`)](#syslog-levels-ulog_syslog)
    - [Lock Extensions](#lock-extensions)
        - [When to Use a Lock](#when-to-use-a-lock)
        - [Common Notes \& Caveats](#common-notes--caveats)
        - [POSIX / pthread (`ulog_lock_pthread`)](#posix--pthread-ulog_lock_pthread)
        - [Windows Critical Section (`ulog_lock_win`)](#windows-critical-section-ulog_lock_win)
        - [FreeRTOS Mutex (`ulog_lock_freertos`)](#freertos-mutex-ulog_lock_freertos)
        - [ThreadX Mutex (`ulog_lock_threadx`)](#threadx-mutex-ulog_lock_threadx)
        - [Zephyr k\_mutex (`ulog_lock_zephyr`)](#zephyr-k_mutex-ulog_lock_zephyr)
        - [CMSIS‑RTOS2 Mutex (`ulog_lock_cmsis`)](#cmsisrtos2-mutex-ulog_lock_cmsis)
        - [macOS `os_unfair_lock` (`ulog_lock_macos`)](#macos-os_unfair_lock-ulog_lock_macos)
        - [Custom Lock Function](#custom-lock-function)
        - [Disabling Locking](#disabling-locking)
    - [Choosing a Helper](#choosing-a-helper)
    - [Adding Your Own Extension](#adding-your-own-extension)
- [Extensions](#extensions)

---

## Level Extensions

### Syslog Levels (`ulog_syslog`)

Provides RFC 5424 style severity names without altering core headers. Compile `extensions/ulog_syslog.c` and include the header:

```c
#include "extensions/ulog_syslog.h"

int main(void) {
    ulog_syslog_enable();
    ulog(ULOG_SYSLOG_INFO, "Started");
    ulog_syslog_disable(); // restore defaults
}
```

Mapping (default numeric level in parentheses): `DEBUG(0)`, `INFO(1)`, `NOTICE(2)`, `WARN(3)`, `ERR(4)`, `CRIT(5)`, `ALERT(6)`, `EMERG(7)`.

---

## Lock Extensions

The core exposes `ulog_lock_set_fn(ulog_lock_fn fn, void *arg)` allowing you to inject any thread-safety primitive. The following helpers wrap common platform APIs so you do not have to write adapters.

### When to Use a Lock

Enable a lock if multiple threads, tasks, or ISRs (with proper exclusion) may log concurrently and you require atomic event dispatch. If logging originates from signal handlers / ISRs ensure the underlying primitive is safe (often it is not — consider a lock-free ring buffer output pattern instead for hard real-time).

### Common Notes & Caveats

| Aspect            | Detail                                                                                                                         |
| ----------------- | ------------------------------------------------------------------------------------------------------------------------------ |
| Reentrancy        | Output & prefix callbacks run under the lock. Avoid calling `ulog_*` inside them (may deadlock with non-reentrant primitives). |
| Performance       | Choose the lightest primitive sufficient for correctness: spin for tiny sections on SMP, mutex / critical section otherwise.   |
| Failures          | If the lock function returns non-`ULOG_STATUS_OK` on acquire, the log call is dropped silently.                                |
| Critical Sections | Keep them short: formatting + routing only. Heavy I/O should be buffered externally.                                           |

### POSIX / pthread (`ulog_lock_pthread`)

Files: `ulog_lock_pthread.[ch]` (on UNIX / macOS). Use existing or allow creation.

```c
#include "extensions/ulog_lock_pthread.h"
static pthread_mutex_t log_mutex;
ulog_lock_pthread_init_enable(&log_mutex); // init + enable
// ... later
ulog_lock_pthread_enable(&log_mutex);      // enable existing
```

### Windows Critical Section (`ulog_lock_win`)

Files: `ulog_lock_win.[ch]` (on `_WIN32`).

```c
#include "extensions/ulog_lock_win.h"
static CRITICAL_SECTION log_cs;
ulog_lock_win_init_enable(&log_cs);
```

### FreeRTOS Mutex (`ulog_lock_freertos`)

Define `ULOG_LOCK_WITH_FREERTOS` and compile `ulog_lock_freertos.[ch]`.

```c
#define ULOG_LOCK_WITH_FREERTOS
#include "extensions/ulog_lock_freertos.h"
ulog_lock_freertos_create_enable();
```

Or reuse existing `SemaphoreHandle_t`:

```c
SemaphoreHandle_t m = xSemaphoreCreateMutex();
ulog_lock_freertos_enable(m);
```

### ThreadX Mutex (`ulog_lock_threadx`)

Define `ULOG_LOCK_WITH_THREADX`.

```c
#define ULOG_LOCK_WITH_THREADX
#include "extensions/ulog_lock_threadx.h"
static TX_MUTEX log_mutex;
ulog_lock_threadx_create_enable(&log_mutex);
```

### Zephyr k_mutex (`ulog_lock_zephyr`)

Define `ULOG_LOCK_WITH_ZEPHYR`.

```c
#define ULOG_LOCK_WITH_ZEPHYR
#include "extensions/ulog_lock_zephyr.h"
static struct k_mutex log_mutex;
ulog_lock_zephyr_init_enable(&log_mutex);
```

### CMSIS‑RTOS2 Mutex (`ulog_lock_cmsis`)

Define `ULOG_LOCK_WITH_CMSIS`.

```c
#define ULOG_LOCK_WITH_CMSIS
#include "extensions/ulog_lock_cmsis.h"
ulog_lock_cmsis_create_enable();
```

Or enable an existing mutex:

```c
ulog_lock_cmsis_enable(existing_id);
```

### macOS `os_unfair_lock` (`ulog_lock_macos`)

Files: `ulog_lock_macos.[ch]` (Apple platforms).

```c
#include "extensions/ulog_lock_macos.h"
static os_unfair_lock log_lock = OS_UNFAIR_LOCK_INIT;
ulog_lock_macos_unfair_enable(&log_lock);
```

### Custom Lock Function

You can always bypass helpers:

```c
static ulog_status my_lock(bool lock, void *arg) {
    (void)arg; // implement acquire/release
    return ULOG_STATUS_OK;
}
ulog_lock_set_fn(my_lock, NULL);
```

### Disabling Locking

Call `ulog_lock_set_fn(NULL, NULL);` (or the helper `ulog_lock_disable()` if you included the spin header).

---

## Choosing a Helper

| Environment                         | Recommended                                              |
| ----------------------------------- | -------------------------------------------------------- |
| Single-core bare metal (short logs) | Spin lock or no lock if interrupts disabled around calls |
| RTOS tasks (moderate contention)    | Native mutex helper for that RTOS                        |
| High contention multicore           | OS mutex / unfair lock (macOS)                           |

---

## Adding Your Own Extension

1. Create `extensions/ulog_<name>.h/.c`.
2. Expose a small enable/disable API returning `ulog_status`.
3. Keep core dependency limited to `ulog.h`.
4. Document usage in this file.

Feel free to submit a PR with additional platforms or higher-level outputs.

# Extensions
