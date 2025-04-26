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

microlog_dep = declare_dependency(include_directories: ['include'], sources: ['src/ulog.c'])
meson.override_dependency(meson.project_name(), microlog_dep)
