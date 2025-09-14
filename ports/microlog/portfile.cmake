# Copyright (c) 2025 microlog authors
# Distributed under the MIT License.

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO an-dr/microlog
    REF v7.0.0-alpha.3
    # TODO: Replace placeholder hash with real SHA512 of the release archive
    SHA512 0
    HEAD_REF main
)

# Configure & install via the project's own CMake. Disable tests.
vcpkg_cmake_configure(
    SOURCE_PATH ${SOURCE_PATH}
    OPTIONS
        -DULOG_BUILD_TESTS=OFF
)

vcpkg_cmake_install()

# Fix up the config (microlog installs config files at root of prefix)
vcpkg_cmake_config_fixup(
    CONFIG_PATH .
    PACKAGE_NAME microlog
)

# Remove any debug share copies duplicated by fixup (standard cleanup)
vcpkg_fixup_pkgconfig()

# Provide a usage file
file(WRITE ${CURRENT_PACKAGES_DIR}/share/microlog/usage
"microlog provides target microlog::microlog.\n"
"Example:\n"
"  find_package(microlog CONFIG REQUIRED)\n"
"  target_link_libraries(app PRIVATE microlog::microlog)\n"
"Optional compile definitions (e.g.): ULOG_BUILD_COLOR=1\n"
)

# License
file(INSTALL
    DESTINATION ${CURRENT_PACKAGES_DIR}/share/microlog
    TYPE FILE
    FILES ${SOURCE_PATH}/LICENSE
)

vcpkg_install_copyright(FILE_LIST ${SOURCE_PATH}/LICENSE)
