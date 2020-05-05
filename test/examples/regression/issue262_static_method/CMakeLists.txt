set(test_name "issue262_static_method")

chimera_add_binding_test_pybind11(${test_name}_pybind11
  SOURCES ${test_name}.h
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/pybind11.yaml
  COPY_MODULE
)

chimera_add_binding_test_boost_python(${test_name}_boost_python
  SOURCES ${test_name}.h
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/boost_python.yaml
  COPY_MODULE
)

chimera_add_python_test(${test_name}_python_tests ${test_name}.py)
