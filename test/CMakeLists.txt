include(CTest)

set(chimera_EXECUTABLE $<TARGET_FILE:chimera>)
include(chimeraFunctions)

find_package(Boost REQUIRED COMPONENTS python)
find_package(PythonLibs REQUIRED)

include_directories(SYSTEM
  ${Boost_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
)

add_chimera_binding(
  EXCLUDE_FROM_ALL
  TARGET test_binding
  SOURCES dart-example.cpp
  NAMESPACES dart
)
target_link_libraries(test_binding
  ${Boost_PYTHON_LIBRARIES}
  ${PYTHON_LIBRARIES}
)
add_test(
    NAME ctest_build_binding
    COMMAND "${CMAKE_COMMAND}"
        --build ${CMAKE_BINARY_DIR}
        --target test_binding
)
