include(chimeraBoostPythonTest)

chimera_add_test_boost_python(test_dart_example
  EXCLUDE_FROM_ALL
  SOURCES dart_example.h
  NAMESPACES dart
)
