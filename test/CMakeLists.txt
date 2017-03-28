include(CTest)

set(chimera_EXECUTABLE $<TARGET_FILE:chimera>)
include(chimeraFunctions)

add_chimera_binding(
  EXCLUDE_FROM_ALL
  TARGET test_binding
  SOURCES dart-example.cpp
  NAMESPACES dart
)
add_test(
    NAME ctest_build_binding
    COMMAND "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target test_binding
)
