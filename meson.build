project('microlog', 'c',
    version: files('version'),
    license: 'MIT',
    default_options: ['warning_level=3'],
)

# ========================
# Library
# ========================

public_include = ['include']
src = ['src/ulog.c']

# Generate configured copies for install only. Build uses originals to avoid churn.
conf = configuration_data()
conf.set('ULOG_VERSION', meson.project_version())

configured_include = []
foreach h : ['include/ulog.h']
    out_name = h.split('/')[-1]
    subdir = ''
    configured = configure_file(
        input: h,
        output: out_name,
        configuration: conf,
    )
    if subdir != ''
        install_data(configured, install_dir: subdir)
    else
        install_headers(configured, subdir: '')
    endif
endforeach

configured_src = configure_file(
    input: 'src/ulog.c',
    output: 'ulog.c',
    configuration: conf,
)

# ========================
# Packaging
# ========================

# Install original include tree (so other headers/extensions besides substituted are present)
install_subdir('include', install_dir: 'include')
# Install configured source to mirror CMake packaging structure
install_data(configured_src, install_dir: 'src')
# (Configured syslog header already installed above.)
install_data('meson/meson.build', install_dir: '.')
install_data('version', install_dir: '.')
install_data('LICENSE', install_dir: '.')

# ========================
# Dependency
# ========================

microlog_dep = declare_dependency(include_directories: public_include, sources: src)
meson.override_dependency(meson.project_name(), microlog_dep)
