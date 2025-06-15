project('example', 'c', 'cpp')

add_global_arguments('-DULOG_NO_COLOR', language: ['cpp', 'c'])

exe = executable(
    meson.project_name(),
    'example.cpp',
    dependencies: dependency('microlog'),
)
