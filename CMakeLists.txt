cmake_minimum_required(VERSION 3.13 FATAL_ERROR)
set(PRODUCT_NAMESPACE "softeq")
project(dbfacade VERSION 0.1.0 LANGUAGES CXX)

include(tools/cmake/SofteqComponentsModel.cmake)
#it MUST be shared lib due to LGPL
make_softeq_component(${PROJECT_NAME} SHARED)

add_library(${PRODUCT_NAMESPACE}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

################################### PROJECT SPECIFIC GLOBALS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
# -O0            (optimization level: -O0, -O1, -O2, -O3, -Os, -Og, -Ofast)
# -Wall          (enables all compiler's warning messages)
# -Wextra        (nables some extra warning flags that are not enabled by -Wall)
# -Wpedantic     (issue all the warnings demanded by strict ISO C and ISO C++)
# -Wfatal-errors (compiler to abort compilation on the first error)
# -Werror        (warning == error)
# -pthread       (multithrading enable)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_target_properties(${PROJECT_NAME}
  PROPERTIES
  LINKER_LANGUAGE CXX
  OUTPUT_NAME "${PRODUCT_NAMESPACE}${PROJECT_NAME}"
  VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}"
  SOVERSION ${PROJECT_VERSION_MAJOR}
  )

configure_file(include/${PROJECT_NAME}/version.hh.in
  include/${COMPONENT_PATH}/version.hh
  )

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_BINARY_DIR}/include/${COMPONENT_PATH}
  )

################################### COMPONENT SOURCES
find_package(SQLite3 REQUIRED)

target_sources(${PROJECT_NAME}
  PRIVATE
  src/version.cc
  src/sqlquery.cc
  src/sqliteconnection.cc
  src/sqlquerybuilder.cc
  src/createtable.cc
  src/sqlexception.cc
  src/sqliteexception.cc
  src/tablescheme.cc
  src/insert.cc
  src/select.cc
  src/update.cc
  src/remove.cc
  src/drop.cc
  src/alter.cc
  src/join.cc
  src/resultlimit.cc
  src/facade.cc
  src/transaction.cc
  src/sqlvalue.cc
  src/constraints.cc
  src/cellrepresentation.cc
  src/columntypes.cc
  src/columndatetime.cc
  )

set(PUBLIC_HEADERS
  include/dbfacade/alter.hh
  include/dbfacade/base_constraint.hh
  include/dbfacade/cell.hh
  include/dbfacade/cellrepresentation.hh
  include/dbfacade/columndatetime.hh
  include/dbfacade/columntypes.hh
  include/dbfacade/condition.hh
  include/dbfacade/connection.hh
  include/dbfacade/constraints.hh
  include/dbfacade/createtable.hh
  include/dbfacade/drop.hh
  include/dbfacade/facade.hh
  include/dbfacade/insert.hh
  include/dbfacade/join.hh
  include/dbfacade/orderby.hh
  include/dbfacade/remove.hh
  include/dbfacade/resultlimit.hh
  include/dbfacade/select.hh
  include/dbfacade/sqlexception.hh
  include/dbfacade/sqliteconnection.hh
  include/dbfacade/sqliteexception.hh
  include/dbfacade/sqlquerybuilder.hh
  include/dbfacade/sqlquery.hh
  include/dbfacade/sqlvalue.hh
  include/dbfacade/tablescheme.hh
  include/dbfacade/token.hh
  include/dbfacade/transaction.hh
  include/dbfacade/typeconverter.hh
  include/dbfacade/typehint.hh
  include/dbfacade/typeserializers.hh
  include/dbfacade/update.hh
  ${CMAKE_BINARY_DIR}/include/${COMPONENT_PATH}/version.hh
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  SQLite3::SQLite3
  )

########################################### SUBCOMPONENTS

option(ENABLE_MIGRATION "Enable DB migration extension" ${BUILD_ALL})
if (ENABLE_MIGRATION)
  add_subdirectory(extensions/migration)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    dbfacade-extensions-migration
#    $<TARGET_PROPERTY:dbfacade-extensions-migration,INTERFACE_LINK_LIBRARIES>
    )
endif ()

option(ENABLE_MYSQL "Enable MySQL extension" ${BUILD_ALL})
if (ENABLE_MYSQL)
  add_subdirectory(extensions/mysql)
  target_link_libraries(${PROJECT_NAME}
    PRIVATE
    dbfacade-extensions-mysql
    )
endif ()

if (BUILD_TESTING)
  add_subdirectory(tests/commontests)
  add_subdirectory(tests/sqlite)
  add_subdirectory(tests)
endif ()

option(BUILD_EXAMPLES "Enable building examples")
if (BUILD_EXAMPLES)
  message(STATUS "* Examples are added to build")
  add_subdirectory(examples)
endif ()

add_softeq_testing()

########################################### INSTALLATION

install_softeq_framework(${PROJECT_NAME})
