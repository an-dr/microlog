project('example', 'c', 'cpp')

# To fix paths in the log messages
add_project_arguments('-fmacro-prefix-map=../=', language: ['cpp', 'c'])

# Build options. Uncomment to enable.
# ===================================

## Note: ULOG_BUILD_DYNAMIC_CONFIG overrides all other options
# add_global_arguments('-DULOG_BUILD_DYNAMIC_CONFIG=1', language: ['cpp', 'c'])

# add_global_arguments('-DULOG_BUILD_SOURCE_LOCATION=1', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_BUILD_LEVEL_STYLE=ULOG_LEVEL_STYLE_SHORT', language : ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_COLOR=1', language : ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_TIME=1', language: ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_PREFIX_SIZE=16', language : ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_EXTRA_OUTPUTS=2', language: ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_TOPICS_NUM=3', language: ['cpp', 'c'])


exe = executable(
    meson.project_name(),
    ['main.cpp'],
    dependencies: [dependency('microlog')],
)
