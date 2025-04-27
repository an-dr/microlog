include(CMakePackageConfigHelpers)

# We have a custom Targets file below. Original is generated like this: install(
# EXPORT ${PROJECT_NAME}Targets NAMESPACE ${PROJECT_NAME}:: DESTINATION
# ${CMAKE_INSTALL_PREFIX})

install(FILES ${CMAKE_CURRENT_LIST_DIR}/micrologTargets.cmake
        DESTINATION ${CMAKE_INSTALL_PREFIX})

# Make a config file using targets based on the template
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}Config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_PREFIX})

# Make a version file
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  COMPATIBILITY SameMajorVersion)

# install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
# DESTINATION ${CMAKE_INSTALL_PREFIX})

install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        ${CMAKE_CURRENT_LIST_DIR}/../LICENSE
  DESTINATION ${CMAKE_INSTALL_PREFIX})
