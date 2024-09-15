project('example', 'c', 'cpp')
  
include = ['.']
src = [ 
    'src/main.c',
    ]

# Cfg options. Uncomment to enable.
add_global_arguments('-DULOG_HAVE_TIME', language : 'c')
# add_global_arguments('-DULOG_NO_COLOR', language : 'c')
# add_global_arguments('-DULOG_CUSTOM_PREFIX_SIZE=16', language : 'c')
# add_global_arguments('-DULOG_HIDE_FILE_STRING', language : 'c')
# add_global_arguments('-DULOG_SHORT_LEVEL_STRINGS', language : 'c')
# add_global_arguments('-DULOG_USE_EMOJI', language : 'c')
add_global_arguments('-DULOG_EXTRA_DESTINATIONS=1', language : 'c')
add_global_arguments('-DULOG_TOPICS_NUM=4', language : 'c')

# To fix paths in the log messages
add_project_arguments('-fmacro-prefix-map=../=',language: 'c')

microlog_dep = dependency('microlog')

exe = executable(meson.project_name(), 
                     src, 
                     include_directories : include,
                     dependencies : [microlog_dep],
                     install : true,
                     )
