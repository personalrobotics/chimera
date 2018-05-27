chimera_add_test_boost_python(function_boost_python
  SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function_boost_python.yaml
  DEBUG
  COPY_MODULE
)

chimera_add_test_pybind11(function_pybind11
  SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function_pybind11.yaml
  DEBUG
  COPY_MODULE
)
