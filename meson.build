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

# --- Amalgamation Start ---
# Assuming src/ulog.c and src/ulog_topics.c are the C files to be amalgamated.
# If other .c files are added to src/, they need to be added to this list.
microlog_all_c_files = files('src/ulog.c', 'src/ulog_topics.c')

amalgamated_c_file_target = custom_target(
    'MicrologAmalgamatedSource', # Name of the target
    input : microlog_all_c_files,
    output : 'microlog_amalgamated.c', # The single output file name
    # Command to concatenate input files into the output file.
    # @INPUT@ expands to a space-separated list of full paths to input files.
    command : ['sh', '-c', 'cat @INPUT@ > @OUTPUT@'],
    build_by_default: true # Ensure it's built
)
# Get the file object from the target
microlog_amalgamated_source_file = amalgamated_c_file_target[0]
# --- Amalgamation End ---

src = [microlog_amalgamated_source_file] # Use the amalgamated source for the library

# ========================
# Packaging
# ========================

install_subdir(public_include, install_dir: '.')
# Install the amalgamated source file, renaming it to ulog.c
install_data(
    [microlog_amalgamated_source_file], # Must be an array of sources/files
    install_dir : 'src',
    rename : ['ulog.c'] # Renames the first source (microlog_amalgamated.c) to 'ulog.c'
)
install_data('meson/meson.build', install_dir: '.')
install_data('version', install_dir: '.')
install_data('LICENSE', install_dir: '.')

# ========================
# Dependency
# ========================

microlog_dep = declare_dependency(include_directories: public_include, sources: src)
meson.override_dependency(meson.project_name(), microlog_dep)
