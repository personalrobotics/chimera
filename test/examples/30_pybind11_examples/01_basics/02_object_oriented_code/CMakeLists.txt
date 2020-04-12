set(test_name "object_oriented_code")

chimera_add_binding_test_boost_python(${test_name}_boost_python
  SOURCES ${test_name}.h
  EXTRA_SOURCES ${test_name}.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/chimera_boost_python.yaml
  COPY_MODULE
)

chimera_add_binding_test_pybind11(${test_name}_pybind11
  SOURCES ${test_name}.h
  EXTRA_SOURCES ${test_name}.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/chimera_pybind11.yaml
  COPY_MODULE
)

chimera_add_python_test(${test_name}_python_tests ${test_name}.py)
