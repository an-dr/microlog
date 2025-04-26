project(
    'microlog',
    'c',
    version: files('version'),
    license: 'MIT',
    default_options: ['warning_level=3'],
)

# ========================
# Library
# ========================

public_include = ['include']
src = ['src/ulog.c']

# ========================
# Packaging
# ========================

install_subdir(public_include, install_dir: '.')
install_data(src, install_dir: 'src')
install_data('meson/meson.build', install_dir: '.')
install_data('version', install_dir: '.')
install_data('LICENSE', install_dir: '.')

# ========================
# Dependency
# ========================

microlog_dep = declare_dependency(include_directories: public_include, sources: src)
meson.override_dependency(meson.project_name(), microlog_dep)
