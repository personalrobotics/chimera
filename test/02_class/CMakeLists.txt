chimera_add_binding_tests(class
  SOURCES class.h
  EXTRA_SOURCES class.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/class.yaml
  DEBUG
  COPY_MODULE
)

add_test(
  NAME class_python_tests
  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/class.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)
