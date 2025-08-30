- ULOG_FEATURE_COLOR            : ULOG_NO_COLOR
- ULOG_FEATURE_PREFIX           : ULOG_PREFIX_SIZE
- ULOG_FEATURE_EXTRA_OUTPUTS    : ULOG_EXTRA_OUTPUTS
- ULOG_FEATURE_SOURCE_LOCATION  : ULOG_HIDE_SOURCE_LOCATION
- ULOG_FEATURE_LEVELS_LONG      : ULOG_SHORT_LEVEL_STRINGS
- ULOG_FEATURE_LEVELS_SHORT     : ULOG_SHORT_LEVEL_STRINGS
- ULOG_FEATURE_TIME             : ULOG_HAVE_TIME
- ULOG_FEATURE_TOPICS           : ULOG_TOPICS_NUM
- ULOG_FEATURE_DYNAMIC_CONFIG   : ULOG_DYNAMIC_CONFIG
- ULOG_FEATURE_WARN_NOT_ENABLED : ULOG_WARN_NOT_ENABLED
- ULOG_FEATURE_COLOR            : ULOG_FEATURE_COLOR
- ULOG_FEATURE_PREFIX           : ULOG_FEATURE_PREFIX_SIZE=N
- ULOG_FEATURE_EXTRA_OUTPUTS    : ULOG_FEATURE_EXTRA_OUTPUTS=N
- ULOG_FEATURE_SOURCE_LOCATION  : ULOG_HIDE_SOURCE_LOCATION
- ULOG_FEATURE_LEVELS_LONG      : ULOG_SHORT_LEVEL_STRINGS
- ULOG_FEATURE_LEVELS_SHORT     : ULOG_SHORT_LEVEL_STRINGS
- ULOG_FEATURE_TIME             : ULOG_HAVE_TIME
- ULOG_FEATURE_TOPICS           : ULOG_TOPICS_NUM
- ULOG_FEATURE_DYNAMIC_CONFIG   : ULOG_DYNAMIC_CONFIG
- ULOG_FEATURE_WARN_NOT_ENABLED : ULOG_WARN_NOT_ENABLED

| Knob                          | Type | Default                 | Description                                                                 |
| ----------------------------- | ---- | ----------------------- | --------------------------------------------------------------------------- |
| `ULOG_CFG_COLOR`              | bool | 1                       | Enable ANSI color output                                                    |
| `ULOG_CFG_PREFIX_SIZE`        | uint | 0                       | Prefix buffer size (0 disables)                                             |
| `ULOG_CFG_EXTRA_OUTPUTS`      | bool | 0                       | Enable additional output backends                                           |
| `ULOG_CFG_SOURCE_LOCATION`    | bool | 1                       | Include file and line information                                           |
| `ULOG_CFG_LEVEL_STR_STYLE`    | enum | LONG                    | Style of level strings:`LONG` or `SHORT`                                    |
| `ULOG_CFG_TIME`               | bool | 0                       | Include timestamp in log output                                             |
| `ULOG_CFG_TOPICS_NUM`         | uint | 0                       | Number of topic slots (0 disables topics)                                   |
| `ULOG_CFG_DYNAMIC_CONFIG`     | bool | 0                       | Allow runtime configuration changes                                         |
| `ULOG_CFG_WARN_NOT_ENABLED`   | bool | 1                       | Compile stub functions that emit warnings when a feature is not enabled     |


| Config knob                 | Derived macro(s)            | Purpose                          |
| --------------------------- | --------------------------- | -------------------------------- |
| `ULOG_CFG_COLOR`            | `ULOG_HAS_COLOR`            | Compile ANSI color paths         |
| `ULOG_CFG_PREFIX_SIZE`      | `ULOG_HAS_PREFIX`           | Enable prefix buffer logic       |
| `ULOG_CFG_EXTRA_OUTPUTS`    | `ULOG_HAS_EXTRA_OUTPUTS`    | Extra backends compiled          |
| `ULOG_CFG_SOURCE_LOCATION`  | `ULOG_HAS_SOURCE_LOCATION`  | Include file\:line               |
| `ULOG_CFG_LEVEL_STR_STYLE`  | `ULOG_LEVEL_STR_IS_SHORT`   | Short level tags switch          |
|                             | `ULOG_LEVEL_STR_IS_LONG`    | Long level tags switch           |
| `ULOG_CFG_TIME`             | `ULOG_HAS_TIME`             | Timestamps available             |
| `ULOG_CFG_TOPICS_NUM`       | `ULOG_HAS_TOPICS`           | Topic system compiled            |
| `ULOG_CFG_DYNAMIC_CONFIG`   | `ULOG_HAS_DYNAMIC_CONFIG`   | Runtime toggles allowed          |
| `ULOG_CFG_WARN_NOT_ENABLED` | `ULOG_HAS_WARN_NOT_ENABLED` | Emit “feature not enabled” stubs |
