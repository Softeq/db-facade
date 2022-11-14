make_softeq_component(tests EXECUTABLE)
################################### PROJECT SPECIFIC GLOBALS
gtest_discover_tests(${PROJECT_NAME})

################################### COMPONENT SOURCES

target_include_directories(${PROJECT_NAME}
  PRIVATE
    ${CMAKE_BINARY_DIR}/include
  )

target_sources(${PROJECT_NAME}
  PRIVATE
  main.cc
  )

target_link_libraries(${PROJECT_NAME}
  PUBLIC
  GTest::GTest
  GTest::Main
  ${PARENT_COMPONENT_NAME}
  )

################################### SUBCOMPONENTS

################################### INSTALLATION
install(TARGETS ${PROJECT_NAME}
  EXCLUDE_FROM_ALL
  )
