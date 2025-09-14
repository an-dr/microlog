project('example', 'c', 'cpp')

microlog_dep = dependency('microlog', required: true)

exe = executable(
    meson.project_name(),
    'example.cpp',
    dependencies: microlog_dep,
)
