chimera_add_binding_test_boost_python(test_class_boost_python
  SOURCES test_class.cpp
  NAMESPACES test_class
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/test_class_boost_python.yaml
  DEBUG
  COPY_MODULE
)

chimera_add_binding_test_pybind11(test_class_pybind11
  SOURCES test_class.cpp
  NAMESPACES test_class
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/test_class_pybind11.yaml
  DEBUG
  COPY_MODULE
)

chimera_add_python_test(class_python_tests test_class.py)
