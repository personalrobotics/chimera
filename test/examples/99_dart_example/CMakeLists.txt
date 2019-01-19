chimera_add_binding_test_boost_python(dart_example_boost_python
  SOURCES dart_example.h
  NAMESPACES dart
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/dart_example.yaml
  COPY_MODULE
)

#chimera_add_binding_test_pybind11(dart_example_pybind11
#  EXCLUDE_FROM_ALL
#  SOURCES dart_example.h
#  NAMESPACES dart
#  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/dart_example.yaml
#  COPY_MODULE
#)
