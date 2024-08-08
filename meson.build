project('microlog', 'c',
        version : files('version'),
        license : 'MIT',
        default_options : ['warning_level=3'])

c_args = []

# ========================
# Options
# ========================


if get_option('no_color')
  message('Disabling ANSI color escape codes')
  add_project_arguments('-DULOG_NO_COLOR', language : 'c')
endif

if get_option('hide_file_string')
    message('Hiding file name and line number')
    add_project_arguments('-DULOG_HIDE_FILE_STRING', language : 'c')
endif

if get_option('short_level_strings')
    message('Using short level strings')
    add_project_arguments('-DULOG_SHORT_LEVEL_STRINGS', language : 'c')
endif

if get_option('use_emoji')
    message('Using emoji in log messages')
    add_project_arguments('-DULOG_USE_EMOJI', language : 'c')
endif

if get_option('have_time')
    message('Printing the time in the log messages')
    add_project_arguments('-DULOG_HAVE_TIME', language : 'c')
endif

extra_destinations = get_option('extra_destinations')
if extra_destinations > 0
    message('Adding ' + extra_destinations.to_string() + ' extra logging destinations')
    add_project_arguments('-DULOG_EXTRA_DESTINATIONS=' + extra_destinations.to_string(), language : 'c')
endif

if get_option('no_stdout')
    message('Not printing log messages to stdout')
    add_project_arguments('-DULOG_NO_STDOUT', language : 'c')
endif

custom_prefix_size = get_option('custom_prefix_size')
if custom_prefix_size > 0
    message('Custom prefix buffer size: ' + custom_prefix_size.to_string())
    add_project_arguments('-DULOG_CUSTOM_PREFIX_SIZE=' + custom_prefix_size.to_string(), language : 'c')
endif


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

