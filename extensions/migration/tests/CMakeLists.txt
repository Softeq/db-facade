make_softeq_component(tests EXECUTABLE)
################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  db_migration_tasks.cc
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  GTest::GTest
  GTest::Main
  softeq::common
  ${PARENT_COMPONENT_NAME}
  ${CMAKE_PROJECT_NAME}
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
