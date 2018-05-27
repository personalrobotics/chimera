chimera_add_binding_tests(function
  SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function.yaml
  DEBUG
  COPY_MODULE
)
