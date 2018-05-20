# Define function for Boost.Python binding tests
#
# chimera_add_test_boost_python(target
#   [DESTINATION destination_dir] # Defaults to `target`
#   [MODULE module]  # Defaults to `target`
#   [CONFIGURATION config_file]
#   [NAMESPACES namespace1 namespace2 ...])
#   SOURCES source1_file [source2_file ...]
#   [EXTRA_SOURCES source1_file ...]
#   [DEBUG] [EXCLUDE_FROM_ALL]
function(chimera_add_test_boost_python test_name)
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
