project('ulog', 'c',
        version : files('version'),
        license : 'MIT',
        default_options : ['warning_level=3'])
  
include = [ 'include' ]
src = [ 'src/ulog.c' ]


lib = static_library(meson.project_name(), 
                     src, 
                     include_directories : include,
                     install : true
                     )


ulog_dep = declare_dependency(link_with : lib,
                              include_directories : include,
                              sources : src)
meson.override_dependency(meson.project_name(), ulog_dep)

# install_headers(include, subdir : meson.project_name())
