include(CTest)

set(chimera_EXECUTABLE $<TARGET_FILE:chimera>)
include(chimeraFunctions)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
find_package(Boost REQUIRED COMPONENTS python)
find_package(PythonLibs REQUIRED)

add_chimera_binding(
  EXCLUDE_FROM_ALL
  TARGET test_binding
  SOURCES dart-example.h
  NAMESPACES dart
)

set_target_properties(test_binding PROPERTIES PREFIX "")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

target_compile_options(test_binding
  PUBLIC "-std=c++11"
)

target_include_directories(test_binding SYSTEM
  PRIVATE
    ${Boost_INCLUDE_DIRS}
    ${PYTHON_INCLUDE_DIRS}
)

target_link_libraries(test_binding
  PRIVATE
    ${Boost_PYTHON_LIBRARY}
    ${PYTHON_LIBRARIES}
)

add_test(
  NAME ctest_build_binding
  COMMAND "${CMAKE_COMMAND}"
    --build ${CMAKE_BINARY_DIR}
    --target test_binding
)
