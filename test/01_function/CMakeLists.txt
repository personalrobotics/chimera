chimera_add_binding_tests(function
  SOURCES function.h
  EXTRA_SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function.yaml
  COPY_MODULE
)

chimera_add_python_test(function_python_tests function.py)
