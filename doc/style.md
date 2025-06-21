# Microlog Code Style and Naming Conventions

## General Style

- **Indentation:** 4 spaces, no tabs.
- **Braces:** K&R style (`if (cond) { ... }`).
- **Line Length:** 100 characters max.
- **Comments:** Use `//` for single-line, `/* ... */` for block comments. Doxygen-style for public APIs.
- **Whitespace:** No trailing whitespace. One blank line between functions.

## Naming Conventions

### Types

- **Structs, Enums, Typedefs:** `snake_case` with `_t` suffix for typedefs.
    - Example: `typedef struct { ... } feature_color_t;`
- **Enum Constants:** `ALL_CAPS` with prefix.
    - Example: `LOG_TRACE`, `FEATURE_COLOR`

### Functions

- **Public Functions:** `ulog_` prefix, `snake_case`.
    - Example: `ulog_set_level`, `ulog_log`
- **Private (static) Functions:** `snake_case`, with prefix `_`.
    - Example: `_print_message`, `_set_topic_level`
- **Private (static) Functions within a section:** `snake_case` with `__` prefix.
    - Example: `__print_message`, `__set_topic_level`
- **Callback Functions:** `callback_` prefix.
    - Example: `callback_stdout`, `callback_file`

### Variables

- **Global/Static Variables:** `snake_case`, descriptive.
    - Example: `feature_color`, `topics`
- **Local Variables:** `snake_case`, short but meaningful.
    - Example: `tgt`, `ev`, `buf`
- **Constants/Macros:** `ALL_CAPS` with underscores.
    - Example: `ULOG_DEFAULT_LOG_LEVEL`, `CFG_TOPICS_NUM`

### Parameters

- **Function Parameters:** `snake_case`.
    - Example: `int level`, `const char *topic_name`

### Files

- **Source Files:** `snake_case.c`
- **Header Files:** `snake_case.h`

## Miscellaneous

- **Boolean Flags:** Use `bool` type, names like `enabled`, `quiet_mode`.
- **Struct Initialization:** Use designated initializers and double braces for arrays: `= {{0}}`
- **Pointer Checks:** Always check for `NULL` before dereferencing.
- **Error Codes:** Return `0` for success, `-1` for failure, or specific error codes as needed.

---

This style ensures consistency, readability, and maintainability across the microlog codebase.
