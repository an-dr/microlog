project(
    'microlog',
    'c',
    version: files('version'),
    license: 'MIT',
    default_options: ['warning_level=3'],
)

# ========================
# Options (declared in meson_options.txt)
# ========================

syslog_ext = get_option('syslog_extension')

# ========================
# Library
# ========================

public_include = ['include']
src = ['src/ulog.c']

if syslog_ext
    src += ['extensions/ulog_syslog.c']
endif

# ========================
# Packaging
# ========================

install_subdir(public_include, install_dir: '.')
install_data(['src/ulog.c'], install_dir: 'src')

if syslog_ext
    install_data('extensions/ulog_syslog.c', install_dir: 'extensions')
endif
install_data('extensions/ulog_syslog.h', install_dir: 'extensions')
install_data('meson/meson.build', install_dir: '.')
install_data('version', install_dir: '.')
install_data('LICENSE', install_dir: '.')

# ========================
# Dependency
# ========================

microlog_dep = declare_dependency(include_directories: public_include, sources: src)
meson.override_dependency(meson.project_name(), microlog_dep)
