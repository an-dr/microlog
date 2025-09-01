# Core Organization

As the library is a single file, the code is organized into sections. Each section is responsible for a specific functionality or feature and ideally rely only on the core functionality.

Typical feature section structure is as follows:

- Section name, prefix, and other section dependencies
- `#if ULOG_HAS_FEATURE_NAME`
- Private functions, objects, etc.
- Public function implementations
- `#else  // ULOG_HAS_FEATURE_NAME`
- Disabled public warning stubs
- Disabled private functions, objects, etc.
- `#endif  // ULOG_HAS_FEATURE_NAME`

The example is shown below:

```c
/* ============================================================================
   Feature: Prefix (`prefix_*`, depends on: Print, Prefix Config)
============================================================================ */
#if ULOG_HAS_PREFIX

// Private
// ================
typedef struct {
    ulog_prefix_fn function;
    char prefix[ULOG_BUILD_PREFIX_SIZE];
} prefix_data_t;

static prefix_data_t prefix_data = {
    .function = NULL,
    .prefix   = {0},
};

static void prefix_print(print_target *tgt, ulog_event *ev) {
    if (prefix_data.function == NULL || !prefix_config_is_enabled()) {
        return;
    }
    prefix_data.function(ev, prefix_data.prefix, ULOG_BUILD_PREFIX_SIZE);
    print_to_target(tgt, "%s", prefix_data.prefix);
}

// Public
// ================

void ulog_prefix_set_fn(ulog_prefix_fn function) {
    prefix_data.function = function;
}

#else  // ULOG_HAS_PREFIX

// Disabled Public
// ================

#if ULOG_HAS_WARN_NOT_ENABLED
void ulog_prefix_set_fn(ulog_prefix_fn function) {
    (void)(function);
    warn_not_enabled("ULOG_BUILD_PREFIX_SIZE");
}
#endif  // ULOG_HAS_WARN_NOT_ENABLED

// Disabled Private
// ================

#define prefix_print(tgt, ev) (void)(tgt), (void)(ev)
#endif  // ULOG_HAS_PREFIX
```

For code style and naming conventions, see [style.md](style.md).
