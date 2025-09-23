# Microlog Extensions

This directory contains extensions for the Microlog logging library.

The extensions are designed to be easy to understand and modify. They are not part of distributed packages (meson, CMake, etc.)! Feel free to copy-paste and adapt them to your needs. If you create a useful extension, please consider submitting a PR.

For information about interface and usage see the extension headers in the `extensions/` folder.

- [Microlog Extensions](#microlog-extensions)
    - [Level Extensions](#level-extensions)
    - [Lock Extensions](#lock-extensions)
    - [Other Extensions](#other-extensions)
    - [Adding Your Own Extension](#adding-your-own-extension)

---

## Level Extensions

This set of extensions provides alternative log level descriptors that can be enabled at runtime. They replace the default levels defined in `ulog.h`.

| Extension | Description                                                        | Main Header                                    |
| --------- | ------------------------------------------------------------------ | ---------------------------------------------- |
| Syslog    | RFC 5424 style severity names replacing the default logging levels | [`ulog_syslog.h`](../extensions/ulog_syslog.h) |

## Lock Extensions

This set of extensions provides mutex/lock helpers for various platforms / RTOSes. They allow you to enable locking in the core logger with minimal effort.

When to Use a Lock? Enable a lock if multiple threads, tasks, or ISRs (with proper exclusion) may log concurrently and you require atomic event dispatch. If logging originates from signal handlers / ISRs ensure the underlying primitive is safe (often it is not — consider a lock-free ring buffer output pattern instead for hard real-time).

| Extension | Description                          | Main Header                                                  |
| --------- | ------------------------------------ | ------------------------------------------------------------ |
| CMSIS     | CMSIS-RTOS2 mutex lock helper        | [`ulog_lock_cmsis.h`](../extensions/ulog_lock_cmsis.h)       |
| FreeRTOS  | FreeRTOS mutex lock helper           | [`ulog_lock_freertos.h`](../extensions/ulog_lock_freertos.h) |
| POSIX     | pthread mutex lock helper            | [`ulog_lock_pthread.h`](../extensions/ulog_lock_pthread.h)   |
| ThreadX   | ThreadX mutex lock helper            | [`ulog_lock_threadx.h`](../extensions/ulog_lock_threadx.h)   |
| Windows   | Windows Critical Section lock helper | [`ulog_lock_win.h`](../extensions/ulog_lock_win.h)           |

## Other Extensions

This set of extensions provides additional logging features and integrations.

| Extension                | Description                                                                                       | Main Header                                                          |
| ------------------------ | ------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------- |
| Generic Logger Interface | Provides a generic logging interface that can simplify migration from/to other logging libraries. | [`ulog_generic_interface.h`](../extensions/ulog_generic_interface.h) |

## Adding Your Own Extension

1. Create `extensions/ulog_<name>.h/.c`.
2. Expose a small enable/disable API returning `ulog_status`.
3. Keep core dependency limited to `ulog.h`.
4. Document usage in the header and add it to this file.

Feel free to submit a PR with additional platforms or higher-level outputs.
