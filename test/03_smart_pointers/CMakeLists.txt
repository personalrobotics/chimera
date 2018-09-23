set(test_name "smart_pointers")

chimera_add_binding_test_pybind11(${test_name}_pybind11
  SOURCES ${test_name}.h
  EXTRA_SOURCES ${test_name}.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}_pybind11.yaml
  COPY_MODULE
)

# TODO(JS): Disabled Boost.Python test until resolving issue of returning unique_ptr
# chimera_add_binding_test_boost_python(${test_name}_boost_python
#   SOURCES ${test_name}.h
#   EXTRA_SOURCES ${test_name}.cpp
#   NAMESPACES chimera_test
#   CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}_boost_python.yaml
#   COPY_MODULE
# )

add_test(
  NAME ${test_name}_python_tests
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${test_name}.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
