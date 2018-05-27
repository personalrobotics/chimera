chimera_add_test_all(function
  SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function.yaml
  DEBUG
  COPY_MODULE
)
