include(CTest)

set(chimera_EXECUTABLE $<TARGET_FILE:chimera>)
include(chimeraFunctions)

# Set python version to be used to build bindings for.
if(NOT CHIMERA_TEST_PYTHON_VERSION)
  set(CHIMERA_TEST_PYTHON_VERSION 3.4 CACHE STRING
    "Choose the target Python version (e.g., 3.4, 2.7)" FORCE
  )
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Find PythonInter
find_package(PythonInterp ${CHIMERA_TEST_PYTHON_VERSION} REQUIRED)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -c
  "from distutils.sysconfig import get_python_lib;\
  print(get_python_lib(plat_specific=True, prefix=''))"
  OUTPUT_VARIABLE PYTHON_SITE_PACKAGES
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Find boost with python components. The name of python component varies
# depending on the boost version and python version.
if(${PYTHON_VERSION_MAJOR} EQUAL 3)
  find_package(Boost COMPONENTS
    python-py${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR} thread
  )
  set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES
    ${Boost_PYTHON-PY${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}_LIBRARIES}
  )
  if (NOT Boost_FOUND)
    find_package(Boost REQUIRED COMPONENTS python3 thread)
    set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES ${Boost_PYTHON3_LIBRARIES})
  endif()
else()
  find_package(Boost REQUIRED COMPONENTS python thread)
  set(CHIMERA_TEST_Boost_PYTHON_LIBRARIES ${Boost_PYTHON_LIBRARIES})
endif()

# Find PythonLibs version that is the same with the version of PythonInterp.
find_package(PythonLibs ${CHIMERA_TEST_PYTHON_VERSION} REQUIRED)

# Define function for binding tests
#
# chimera_add_binding_test(target
#   [DESTINATION destination_dir] # Defaults to `target`
#   [MODULE module]  # Defaults to `target`
#   [CONFIGURATION config_file]
#   [NAMESPACES namespace1 namespace2 ...])
#   SOURCES source1_file [source2_file ...]
#   [EXTRA_SOURCES source1_file ...]
#   [DEBUG] [EXCLUDE_FROM_ALL]
function(chimera_add_binding_test test_name)
  include(ExternalProject)

  # Parse boolean, unary, and list arguments from input.
  # Unparsed arguments can be found in variable ARG_UNPARSED_ARGUMENTS.
  set(prefix chimera_test)
  set(options DEBUG EXCLUDE_FROM_ALL)
  set(oneValueArgs TARGET MODULE CONFIGURATION DESTINATION)
  set(multiValueArgs SOURCES NAMESPACES EXTRA_SOURCES LINK_LIBRARIES)
  cmake_parse_arguments(
    "${prefix}" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )

  add_chimera_binding(
    EXCLUDE_FROM_ALL=${chimera_test_EXCLUDE_FROM_ALL}
    TARGET ${test_name}
    SOURCES ${chimera_test_SOURCES}
    NAMESPACES ${chimera_test_NAMESPACES}
  )

  set_target_properties(${test_name} PROPERTIES PREFIX "")

  target_compile_options(${test_name}
    PUBLIC "-std=c++11"
  )

  target_include_directories(${test_name} SYSTEM
    PRIVATE
      ${Boost_INCLUDE_DIRS}
      ${PYTHON_INCLUDE_DIRS}
  )

  target_link_libraries(${test_name}
    PRIVATE
      ${Boost_PYTHON_LIBRARY}
      ${PYTHON_LIBRARIES}
  )

  add_test(
    NAME ctest_${test_name}
    COMMAND "${CMAKE_COMMAND}"
      --build ${CMAKE_BINARY_DIR}
      --target ${test_name}
  )
endfunction()

chimera_add_binding_test(test_dart_example
  EXCLUDE_FROM_ALL
  SOURCES dart-example.h
  NAMESPACES dart
)
