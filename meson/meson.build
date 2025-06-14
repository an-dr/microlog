# *************************************************************************
#
# Copyright (c) 2025 Andrei Gramakov. All rights reserved.
#
# This file is licensed under the terms of the MIT license.  
# For a copy, see: https://opensource.org/licenses/MIT
#
# site:    https://agramakov.me
# e-mail:  mail@agramakov.me
#
# *************************************************************************

project(
    'microlog',
    'c',
    version: files('version'),
    license: 'MIT',
    license_files : ['LICENSE'],
    default_options: ['warning_level=3'],
)

microlog_lib = library(
    meson.project_name(),
    'src/ulog.c',
    include_directories: include_directories('include'),
    install: true,
    install_dir: join_paths(get_option('libdir'), meson.project_name()),
)

microlog_dep = declare_dependency(include_directories: ['include'],
                                  link_with: microlog_lib,
                                  version: meson.project_version())
meson.override_dependency(meson.project_name(), microlog_dep)
