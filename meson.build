project('microlog', 'c',
    # Meson allows version to be a File object but treat first line only.
    version: files('version'),
    license: 'MIT',
    default_options: ['warning_level=3'],
)

# ========================
# Library
# ========================

public_include = include_directories('include')
src = ['src/ulog.c']

# Treat sources as templates to replace version string
conf = configuration_data()
conf.set('ULOG_VERSION', meson.project_version())

ulog_h_cfg = configure_file(
  input: 'include/ulog.h',
  output: 'ulog.h',
  configuration: conf,
)

ulog_c_cfg = configure_file(
  input: 'src/ulog.c',
  output: 'ulog.c',
  configuration: conf,
)

# ========================
# Packaging
# ========================

install_headers(ulog_h_cfg, subdir: '')
install_data(ulog_c_cfg, install_dir: 'src')
install_data('meson/meson.build', install_dir: '.')
install_data('version', install_dir: '.')
install_data('LICENSE', install_dir: '.')

# ========================
# Dependency
# ========================

microlog_dep = declare_dependency(include_directories: public_include, sources: src)
meson.override_dependency(meson.project_name(), microlog_dep)
