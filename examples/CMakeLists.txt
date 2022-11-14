cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(examples LANGUAGES CXX)

if (NOT BUILD_EXAMPLES)
  set(CMAKE_CXX_STANDARD 11)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  find_package(softeq-dbfacade REQUIRED)
endif ()

link_libraries(
  softeq::dbfacade
  )

# base_operations example
add_executable(base_operations
  base_operations.cc
  )

# complex_operations example
add_executable(complex_operations
  complex_operations.cc
  )

# serializers example
add_executable(typeconverters 
  typeconverters.cc
)

if (EXTENSION_MIGRATION OR BUILD_ALL)

  find_package(softeq-common REQUIRED)

  # db migration example
  add_executable(db_migration
    migration.cc
    )
  target_link_libraries(db_migration
    PUBLIC
    softeq::common
  )
endif ()
