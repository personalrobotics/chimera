chimera_add_binding_test_boost_python(test_double_pointer_boost_python
  SOURCES test_double_pointer.cpp
  NAMESPACES test_double_pointer
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/test_double_pointer_boost_python.yaml
  DEBUG
  COPY_MODULE
)

#chimera_add_binding_test_pybind11(test_double_pointer_pybind11
#  SOURCES test_double_pointer.cpp
#  NAMESPACES test_double_pointer
#  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/test_double_pointer_pybind11.yaml
#  DEBUG
#  COPY_MODULE
#)
