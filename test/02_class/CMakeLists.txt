chimera_add_binding_tests(class
  SOURCES class.h
  EXTRA_SOURCES class.cpp
  NAMESPACES chimera_test
  CONFIGURATION ${CMAKE_CURRENT_SOURCE_DIR}/class.yaml
  COPY_MODULE
)

chimera_add_python_test(class_python_tests class.py)
