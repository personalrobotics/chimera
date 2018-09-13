chimera_add_binding_tests(function
  SOURCES function.h
  EXTRA_SOURCES function.cpp
  NAMESPACES function
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/function.yaml
  DEBUG
  COPY_MODULE
)

add_test(
  NAME function_python_tests
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/function.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
