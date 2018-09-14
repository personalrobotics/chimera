chimera_add_binding_tests(virtual_function
  SOURCES virtual_function.h
  EXTRA_SOURCES virtual_function.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/virtual_function.yaml
  DEBUG
  COPY_MODULE
)

add_test(
  NAME virtual_function_python_tests
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/virtual_function.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
