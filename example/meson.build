project('example', 'c', 'cpp')
  
include = ['.']
src = [ 
    'src/main.cpp',
    ]

# Cfg options. Uncomment to enable.
add_global_arguments('-DULOG_HAVE_TIME', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_NO_COLOR', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_CUSTOM_PREFIX_SIZE=16', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_HIDE_FILE_STRING', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_DEFAULT_LOG_LEVEL=LOG_WARNING', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_SHORT_LEVEL_STRINGS', language : ['cpp', 'c'])
# add_global_arguments('-DULOG_USE_EMOJI', language : ['cpp', 'c'])
add_global_arguments('-DULOG_EXTRA_OUTPUTS=2', language : ['cpp', 'c'])
add_global_arguments('-DULOG_TOPICS_NUM=-1', language : ['cpp', 'c'])

# To fix paths in the log messages
add_project_arguments('-fmacro-prefix-map=../=',language: ['cpp', 'c'])

microlog_dep = dependency('microlog')

exe = executable(meson.project_name(), 
                     src, 
                     include_directories : include,
                     dependencies : [microlog_dep],
                     install : true,
                     )
