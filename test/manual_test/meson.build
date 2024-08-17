project('manual_test', 'c', 'cpp')
  
include = ['.']
src = [ 
    'src/main.c',
    ]


microlog_dep = dependency('microlog', 
                          default_options : ['no_color=false',
                                             'hide_file_string=false',
                                             'short_level_strings=false',
                                             'use_emoji=false',
                                             'have_time=false',
                                             'extra_destinations=2',
                                             'no_stdout=false',
                                             'custom_prefix_size=16',
                                             ],
                          )

exe = executable(meson.project_name(), 
                     src, 
                     include_directories : include,
                     dependencies : [microlog_dep],
                     install : true,
                     )
