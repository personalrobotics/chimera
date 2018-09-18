set(test_name "assimp_example")

find_package(ASSIMP QUIET)
if (NOT ASSIMP_FOUND)
  return()
endif()

chimera_add_binding_tests(${test_name}
  SOURCES ${test_name}.h
  EXTRA_SOURCES ${test_name}.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.yaml
  DEBUG
  COPY_MODULE
  INCLUDE_DIRS ${ASSIMP_INCLUDE_DIRS}
  LINK_LIBRARIES ${ASSIMP_LIBRARIES}
)

chimera_add_python_test(${test_name}_python_tests ${test_name}.py)
