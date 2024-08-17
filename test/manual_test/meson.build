project('manual_test', 'c', 'cpp')
  
include = ['.']
src = [ 
    'src/main.c',
    ]


microlog_dep = dependency('microlog', 
                          default_options : ['no_color=false',
                                             'hide_file_string=true',
                                             'short_level_strings=true',
                                             'use_emoji=true',
                                             'have_time=true',
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
