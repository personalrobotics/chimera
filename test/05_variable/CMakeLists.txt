set(test_name "variable")

chimera_add_binding_tests(${test_name}
  SOURCES ${test_name}.h
  EXTRA_SOURCES ${test_name}.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.yaml
  COPY_MODULE
)

chimera_add_python_test(${test_name}_python_tests ${test_name}.py)
