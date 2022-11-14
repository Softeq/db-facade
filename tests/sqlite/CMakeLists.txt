make_softeq_component(tests EXECUTABLE GROUP sqlite)
################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  )

target_link_libraries(${PROJECT_NAME}
  PRIVATE
  ${CMAKE_PROJECT_NAME}-commontests
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
