# AGENTS.md - AI Assistant Guide for Microlog

This document helps AI coding assistants (Claude, GitHub Copilot, ChatGPT, etc.) understand the microlog project structure and contribute effectively.

## Project Overview

**microlog** is an extensible and configurable logging library for embedded and desktop applications in C/C++. It emphasizes:
- Universal design for embedded and desktop
- Zero-cost abstraction: no feature = no compiled code
- Two-file core (ulog.h + ulog.c)
- Extension-based architecture
- Static and dynamic allocation modes

**Base Repository**: [https://github.com/an-dr/microlog](https://github.com/an-dr/microlog)
**License**: MIT
**Language**: C (C++ compatible)
**Build Systems**: CMake, Meson, CPM

## Project Structure

```
microlog/
├── src/
│   └── ulog.c              # Single-file implementation (~3000 lines, section-based)
├── include/
│   └── ulog.h              # Public API header
├── extensions/             # Optional add-ons using only public API
├── tests/
│   ├── unit/              # Unit tests (doctest framework)
│   └── integration/       # Package integration tests
├── example/               # Usage examples
├── doc/                   # Documentation
└── scripts/               # Build and release scripts (PowerShell)
```

## Core Architecture

### Logging Flow

```
ulog_info("message")
  → ulog_log()
  → Creates ulog_event
  → output_handle_all()
  → Iterates handlers:
      - output_stdout_handler()
      - output_file_handler()
      - User custom handlers
  → log_print_event()
  → vprint()/print() to targets
```

See [doc/design.md](doc/design.md) for detailed diagrams.

### Code Organization

The library uses a **section-based architecture** in a single file ([src/ulog.c](src/ulog.c)):

```c
/* ============================================================================
   Feature: <Name> (`prefix_*`, depends on: <Dependencies>)
============================================================================ */
#if ULOG_HAS_FEATURE_NAME
// Private functions and data
// Public function implementations
#else  // ULOG_HAS_FEATURE_NAME
// Disabled stubs (with warnings if ULOG_BUILD_WARN_NOT_ENABLED=1)
#endif  // ULOG_HAS_FEATURE_NAME
```

Each section is self-contained and depends only on core functionality or explicitly stated dependencies. See [doc/code.md](doc/code.md) for examples.

## Key Concepts

### Build-Time Configuration

Features are controlled via compile-time defines (no feature = no code):

| Define                      | Default                    | Purpose                               |
| --------------------------- | -------------------------- | ------------------------------------- |
| ULOG_BUILD_COLOR            | 0                          | ANSI color codes                      |
| ULOG_BUILD_TIME             | 0                          | Timestamps                            |
| ULOG_BUILD_PREFIX_SIZE      | 0                          | Custom prefix (0 = disabled)          |
| ULOG_BUILD_SOURCE_LOCATION  | 1                          | file:line output                      |
| ULOG_BUILD_LEVEL_SHORT      | 0                          | Short level names (T/D/I/W/E/F)       |
| ULOG_BUILD_TOPICS_MODE      | ULOG_BUILD_TOPICS_MODE_OFF | Topic filtering mode (STATIC/DYNAMIC) |
| ULOG_BUILD_EXTRA_OUTPUTS    | 0                          | File/custom outputs                   |
| ULOG_BUILD_DYNAMIC_CONFIG   | 0                          | Runtime toggles (enables all)         |
| ULOG_BUILD_WARN_NOT_ENABLED | 1                          | Warning stubs for disabled APIs       |

Example (CMake):
```cmake
target_compile_definitions(microlog PRIVATE ULOG_BUILD_COLOR=1)
```

### Log Levels

8 levels (ascending severity): `ULOG_LEVEL_0` ... `ULOG_LEVEL_7`

Default aliases:
- `ULOG_LEVEL_TRACE` (0) - Execution tracing
- `ULOG_LEVEL_DEBUG` (1) - Debug info
- `ULOG_LEVEL_INFO` (2) - General information
- `ULOG_LEVEL_WARN` (3) - Warnings
- `ULOG_LEVEL_ERROR` (4) - Recoverable errors
- `ULOG_LEVEL_FATAL` (5) - Critical failures
- Levels 6-7 are user-defined

Custom levels can be set via `ulog_level_set_new_levels()`.

### Topics

Optional subsystem-based filtering (e.g., "network", "storage"):

- **Static allocation**: `ULOG_BUILD_TOPICS_MODE=ULOG_BUILD_TOPICS_MODE_STATIC` + `ULOG_BUILD_TOPICS_STATIC_NUM=N` (fixed count)
- **Dynamic allocation**: `ULOG_BUILD_TOPICS_MODE=ULOG_BUILD_TOPICS_MODE_DYNAMIC` (heap)

Usage:
```c
ulog_topic_add("network", ULOG_OUTPUT_ALL, true);
ulog_topic_info("network", "Connected");
```

### Outputs

- **STDOUT** (always available): `ULOG_OUTPUT_STDOUT`
- **FILE**: Add via `ulog_output_add_file(fp, level)`
- **Custom handlers**: Add via `ulog_output_add(handler_fn, arg, level)`

Each output has independent level filtering.

### Thread Safety

Inject external locks via `ulog_lock_set_fn()`:

```c
typedef ulog_status (*ulog_lock_fn)(bool lock, void *udata);
```

See `extensions/ulog_lock_*.h` for platform helpers (pthread, FreeRTOS, Windows, etc.).

## Coding Guidelines

### Style ([doc/style.md](doc/style.md))

- **Formatting**: clang-format (see [.clang-format](.clang-format))
- **Naming**:
  - Public API: `ulog_feature_action()` (e.g., `ulog_topic_add`)
  - Private functions: `feature_action()` (e.g., `topic_add_internal`)
  - Structs: `feature_data_t` (private), `ulog_feature_t` (public)
- **Sections**: Clearly marked with header comments
- **Macros**: Guard disabled features with `#if ULOG_HAS_FEATURE`

### Testing

- **Unit tests**: [tests/unit/](tests/unit/) (doctest framework)
- **Integration tests**: [tests/integration/](tests/integration/) (CMake/Meson packages)
- Run via: `./scripts/run_tests.ps1` (PowerShell)

Update tests when modifying features!

## Extensions ([extensions/README.md](extensions/README.md))

Extensions use **only the public API** and are not part of distributed packages. Users copy-paste and adapt them.

**Available**:
- `ulog_syslog.h/.c` - RFC 5424 severity levels
- `ulog_lock_pthread.h/.c` - POSIX threads
- `ulog_lock_freertos.h/.c` - FreeRTOS
- `ulog_lock_cmsis.h/.c` - CMSIS-RTOS2
- `ulog_lock_threadx.h/.c` - Azure ThreadX
- `ulog_lock_win.h/.c` - Windows Critical Sections
- `ulog_generic_interface.h` - Generic logger interface (header-only)

**Creating extensions**:
1. Create `extensions/ulog_<name>.h/.c`
2. Use only `ulog.h` API
3. Provide enable/disable functions returning `ulog_status`
4. Document in header and update `extensions/README.md`

## Common Tasks for AI Assistants

### Adding a New Feature

1. **Check dependencies**: Does it need new build options?
2. **Update header** ([include/ulog.h](include/ulog.h)):
   - Add public API declarations
   - Add macros if needed
3. **Update source** ([src/ulog.c](src/ulog.c)):
   - Add section with `#if ULOG_HAS_FEATURE_NAME`
   - Implement private functions
   - Implement public functions
   - Add disabled stubs in `#else` block
4. **Add tests** in [tests/unit/](tests/unit/)
5. **Update docs** ([doc/features.md](doc/features.md))
6. **Update CHANGELOG.md**

### Modifying Existing Features

1. **Read the section** in [src/ulog.c](src/ulog.c) (search for feature name)
2. **Check build config** - what needs to be enabled?
3. **Understand dependencies** - section header lists them
4. **Modify carefully** - sections should remain encapsulated
5. **Update tests and docs**

### Debugging Build Issues

- **Feature not compiling?** Check `ULOG_BUILD_*` defines
- **Linker errors?** Feature disabled but called (check `ULOG_HAS_*` guards)
- **Runtime warnings?** `ULOG_BUILD_WARN_NOT_ENABLED=1` shows when disabled features are called

### Working with Tests

- Unit tests use doctest (C++ framework)
- Test files in [tests/unit/](tests/unit/)
- Run: `meson test -C build` or `ctest --test-dir build`
- CI runs on every PR ([.github/workflows/workflow-tests.yml](.github/workflows/workflow-tests.yml))

### Release Process

1. Update [version](version) file
2. Update [CHANGELOG.md](CHANGELOG.md)
3. Run build scripts in [scripts/](scripts/) to generate packages
4. CI workflow [.github/workflows/workflow-release.yml](.github/workflows/workflow-release.yml) creates release

## Important Constraints

### DO NOT:
- ❌ Add dependencies (library is dependency-free except standard C)
- ❌ Break two-file core principle (extensions are separate)
- ❌ Add features that compile by default (use opt-in defines)
- ❌ Break backward compatibility without major version bump
- ❌ Add platform-specific code to core (use extensions)

### DO:
- ✅ Keep features encapsulated in sections
- ✅ Use `ULOG_HAS_*` guards for all feature code
- ✅ Provide disabled stubs with `ULOG_BUILD_WARN_NOT_ENABLED`
- ✅ Document in headers and [doc/features.md](doc/features.md)
- ✅ Add tests for new functionality
- ✅ Follow existing code style (run clang-format)
- ✅ Consider embedded constraints (static allocation, size)

## Example Scenarios

### User wants colored timestamps

**Current**: `ULOG_BUILD_COLOR=1` colors level names, `ULOG_BUILD_TIME=1` adds timestamps
**Request**: Color timestamps differently

**Approach**:
1. Check Color section in [src/ulog.c](src/ulog.c)
2. Modify `color_print()` to accept time parameter
3. Update `log_print_event()` to pass time to color handler
4. Add configuration option if needed
5. Test with different terminals

### Porting to new RTOS

**Extension approach**:
1. Study existing lock extensions in [extensions/](extensions/)
2. Create `extensions/ulog_lock_mynewrtos.h/.c`
3. Implement:
   ```c
   ulog_status ulog_lock_mynewrtos_enable(void);
   ulog_status ulog_lock_mynewrtos_disable(void);
   ```
4. Document usage in header
5. Submit PR (optional)

## Quick Reference

### Key Files
- [README.md](README.md) - Project overview
- [CONTRIBUTING.md](CONTRIBUTING.md) - Contribution guidelines
- [doc/features.md](doc/features.md) - Feature documentation
- [doc/design.md](doc/design.md) - Architecture
- [doc/code.md](doc/code.md) - Code organization
- [doc/style.md](doc/style.md) - Coding standards
- [CHANGELOG.md](CHANGELOG.md) - Version history

### API Overview
```c
// Basic logging
ulog_trace/debug/info/warn/error/fatal("message %d", value);

// With topics (requires ULOG_BUILD_TOPICS_MODE != ULOG_BUILD_TOPICS_MODE_OFF)
ulog_topic_info("network", "Connected");

// Configuration
ulog_output_level_set(ULOG_OUTPUT_STDOUT, ULOG_LEVEL_INFO);
ulog_lock_set_fn(my_lock_fn, lock_data);
ulog_time_set_fn(my_time_fn, time_data);
ulog_prefix_set_fn(my_prefix_fn);

// Outputs (requires ULOG_BUILD_EXTRA_OUTPUTS > 0)
ulog_output_id id = ulog_output_add_file(fp, level);
ulog_output_id id = ulog_output_add(handler, arg, level);
ulog_output_remove(id);

// Cleanup
ulog_cleanup();  // Free all dynamic resources
```

## Resources

- **Repository**: https://github.com/an-dr/microlog
- **Issues**: https://github.com/an-dr/microlog/issues
- **Releases**: https://github.com/an-dr/microlog/releases
- **Credits**: Based on [rxi/log.c](https://github.com/rxi/log.c)

## Version

This AGENTS.md reflects microlog v7.0.0 architecture. Check [CHANGELOG.md](CHANGELOG.md) for updates.

---

**For AI Assistants**: When in doubt, prefer conservative changes. The library values simplicity, portability, and zero-cost abstractions. Always test across embedded and desktop scenarios. When suggesting changes, explain the embedded impact (code size, RAM usage, static vs dynamic allocation).
