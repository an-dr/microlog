# Core Organization

As the library is a single file, the code is organized into sections. Each section is responsible for a specific functionality or feature and ideally rely only on the core functionality.

Typical feature section structure is as follows:

- Section name
- `#if FEATURE_NAME`
- Private functions, objects, etc.
- Public function implementations
- `#else  // FEATURE_NAME`
- Disabled private functions, objects, etc.
- `#endif  // FEATURE_NAME`

The example is shown below:

```c
/* ============================================================================
   Feature: Custom Prefix
============================================================================ */
#if FEATURE_CUSTOM_PREFIX

// Private
// ================

typedef struct {
    bool enabled;                                // Is the custom prefix enabled
    ulog_PrefixFn function;                      // Custom prefix function
    char custom_prefix[CFG_CUSTOM_PREFIX_SIZE];  // Custom prefix
} feature_custom_prefix_t;

static feature_custom_prefix_t feature_custom_prefix = {
    .enabled       = true,
    .function      = NULL,
    .custom_prefix = {0},
};

static void print_prefix(log_target *tgt, ulog_Event *ev) {
    if (feature_custom_prefix.function && feature_custom_prefix.enabled) {
        feature_custom_prefix.function(ev, feature_custom_prefix.custom_prefix,
                                       CFG_CUSTOM_PREFIX_SIZE);
        print(tgt, "%s", feature_custom_prefix.custom_prefix);
    }
}

// Public
// ================

void ulog_set_prefix_fn(ulog_PrefixFn function) {
    feature_custom_prefix.function = function;
}

void ulog_enable_prefix(bool enable) {
    feature_custom_prefix.enabled = enable;
}

// Disabled Private
// ================
#else  // FEATURE_CUSTOM_PREFIX
#define print_prefix(tgt, ev) (void)(tgt), (void)(ev)
#endif  // FEATURE_CUSTOM_PREFIX

```
