# ----------------------------------------------------------------------------
# Building
# ----------------------------------------------------------------------------

add_library(${PROJECT_NAME} INTERFACE)
target_sources(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../src/ulog.c>
            $<INSTALL_INTERFACE:src/ulog.c> # installed version will assume
                                            # relative to install prefix
)
target_include_directories(
  ${PROJECT_NAME}
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/../include> # while
                                                                 # building
            $<INSTALL_INTERFACE:include>) # when installed

# ----------------------------------------------------------------------------
# Installing
# ----------------------------------------------------------------------------

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  ARCHIVE DESTINATION lib)

# Headers (they are sources, not artifacts so copy them explicitly)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_PREFIX}/include)
install(FILES src/ulog.c DESTINATION ${CMAKE_INSTALL_PREFIX}/src)
