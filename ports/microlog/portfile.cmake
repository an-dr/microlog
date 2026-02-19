# This overlay port installs microlog from the local source tree.
# For the official vcpkg registry, see the release artifacts which
# contain a portfile with the correct REF and SHA512.

set(SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../..")

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
)

vcpkg_cmake_install()

# Relocate cmake package config files to the vcpkg-standard location.
# vcpkg_cmake_config_fixup also corrects PACKAGE_PREFIX_DIR so that
# src/ulog.c and include/ are still found at the package prefix root.
vcpkg_cmake_config_fixup(CONFIG_PATH ".")

# microlog is source-distributed (no compiled artifacts); remove debug copy
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug")

# Remove the LICENSE that cmake installed to prefix root (replaced by vcpkg_install_copyright)
file(REMOVE "${CURRENT_PACKAGES_DIR}/LICENSE")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
