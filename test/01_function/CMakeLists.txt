chimera_add_binding_test_boost_python(function_boost_python
  SOURCES function.h
  EXTRA_SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function_boost_python.yaml
  COPY_MODULE
)

chimera_add_binding_test_pybind11(function_pybind11
  SOURCES function.h
  EXTRA_SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function_pybind11.yaml
  COPY_MODULE
)

chimera_add_python_test(function_python_tests function.py)
