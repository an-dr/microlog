# Microlog Code Style and Naming Conventions

## General Style

- **Indentation:** 4 spaces, no tabs.
- **Braces:** K&R style (`if (cond) { ... }`).
- **Line Length:** 80 characters max.
- **Comments:** Use `//` for single-line, `/* ... */` for block comments. Doxygen-style for public APIs (in the header). Doxygen-style for private functions (in the source).
- **Whitespace:** No trailing whitespace. One blank line between functions.

## Naming Conventions

### Types

- **Structs, Enums, Unions, Typedefs:** Always a typedef - `snake_case` with prefix `feature_name_` and the `_t` suffix. Example:

```c
typedef struct {
    char *data;
    unsigned int curr_pos;
    size_t size;
} print_buffer;

typedef union {
    print_buffer buffer;
    FILE *stream;
} print_target_descriptor;
```

- **Enum Constants:** `ALL_CAPS` with prefix.
    - Example: `typedef enum { LOG_TARGET_BUFFER, LOG_TARGET_STREAM } log_target_t;`

### Functions

- **Public Functions:** `ulog_` prefix, `snake_case`.
    - Example: `ulog_set_level`, `ulog_log`
- **Private (static) Functions:** `snake_case`, with prefix `feature_name_`.
    - Example: `color_print_start`, `color_print_end`, `levels_print`

### Variables

- **Global/Static Variables:** NOT ALLOWED. Create a structure to hold state if needed.
- **Local Variables:** `snake_case`, short is widely used or very tiny scope, otherwise full meaningful words.
    - Example: `tgt`, `ev`, `buf`, `format`, `out_topic_id`
- **Constants/Macros:** `ALL_CAPS` with underscores.
    - Example: `ULOG_DEFAULT_LOG_LEVEL`, `FEATURE_TOPICS_CFG_NUM`

### Parameters

- **Function Parameters:** `snake_case`.
    - Example: `int level`, `const char *topic_name`

### Other Conventions and Recommendations

- Avoid implicit type conversions, as they can cause unexpected behavior.
- Use `if (something)` for boolean checks.
- Use explicit comparisons like `if (something < 0)` when needed.
- Write conditions clearly to show your intent.
- Use auto formatting tools like `clang-format` to maintain consistency.
