project('microlog', 'c',
        version : files('version'),
        license : 'MIT',
        default_options : ['warning_level=3'])


# ========================
# Library
# ========================

public_include = ['include']
src = [ 'src/ulog.c' ]

lib = static_library(meson.project_name(), 
                     src, 
                     include_directories : public_include,
                     install : true,
                     install_dir: meson.project_name() + '/lib',
                     )

install_subdir(public_include, 
               install_dir: meson.project_name())


# ========================
# Dependency
# ========================

ulog_dep = declare_dependency(link_with : lib,
                              include_directories : public_include)
meson.override_dependency(meson.project_name(), ulog_dep)

