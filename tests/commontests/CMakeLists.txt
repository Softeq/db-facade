make_softeq_component(commontests  SHARED)
################################### PROJECT SPECIFIC GLOBALS

################################### COMPONENT SOURCES

target_sources(${PROJECT_NAME}
  PRIVATE
  testfixture.cc
  alter.cc
  createtable.cc
  cascade.cc
  drop.cc
  insert.cc
  join.cc
  multithreading.cc
  remove.cc
  select.cc
  typeconverters.cc
  transaction.cc
  update.cc
  verifyscheme.cc
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
