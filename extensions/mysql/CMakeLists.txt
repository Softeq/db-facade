make_softeq_component(mysql OBJECT
  GROUP extensions)

################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES
pkg_check_modules(LIBMYSQLCLIENT REQUIRED mysqlclient)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/mysqlconnection.cc
  src/mysqlexception.cc
  src/mysqlquerybuilder.cc
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE
  ${LIBMYSQLCLIENT_INCLUDE_DIRS}
  )

target_link_directories(${PROJECT_NAME} 
  PRIVATE
  ${LIBMYSQLCLIENT_LIBRARY_DIRS}
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ${LIBMYSQLCLIENT_LIBRARIES}
  )

################################### SUBCOMPONENTS
if(BUILD_TESTING)
  add_subdirectory(tests)
endif()

################################### INSTALLATION
deploy_softeq_component(${PROJECT_NAME}
  PUBLIC_HEADERS
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/mysqlconnection.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/mysqlexception.hh
  ${CMAKE_SOURCE_DIR}/include/${COMPONENT_PATH}/mysqlquerybuilder.hh
  INSTALL_PARAMS
# static lib is excluded because of LGPL
  ARCHIVE DESTINATION EXCLUDE_FROM_ALL
  )
