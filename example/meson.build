project('example', 'c', 'cpp')
  
include = ['.']
src = [ 
    'src/main.cpp',
    ]

# Cfg options. Uncomment to enable.
add_global_arguments('-DULOG_BUILD_TIME=1', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_BUILD_COLOR=1', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_BUILD_PREFIX_SIZE=16', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_BUILD_SOURCE_LOCATION=1', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_BUILD_LEVEL_STR_STYLE=ULOG_LEVEL_STYLE_SHORT', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_USE_EMOJI', language : ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_EXTRA_OUTPUTS=2', language : ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_TOPICS_NUM=-1', language : ['cpp', 'c'])
add_global_arguments('-DULOG_BUILD_DYNAMIC_CONFIG=1', language : ['cpp', 'c'])

# To fix paths in the log messages
add_project_arguments('-fmacro-prefix-map=../=',language: ['cpp', 'c'])

microlog_dep = dependency('microlog')

exe = executable(meson.project_name(), 
                     src, 
                     include_directories : include,
                     dependencies : [microlog_dep],
                     install : true,
                     )
