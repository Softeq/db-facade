make_softeq_component(migration INTERFACE
  GROUP extensions)
################################### PROJECT SPECIFIC GLOBALS
find_package(softeq-common REQUIRED)

################################### COMPONENT SOURCES
target_include_directories(${PROJECT_NAME}
  INTERFACE
  )

target_link_libraries(${PROJECT_NAME}
  INTERFACE
  softeq::common
  )

if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/migration.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
