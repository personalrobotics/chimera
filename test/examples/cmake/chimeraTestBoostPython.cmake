# Define function for Boost.Python binding tests
#
# chimera_add_binding_test_boost_python(target
#   SOURCES source1_file [source2_file ...]
#   [EXTRA_SOURCES source1_file ...]
#   [DESTINATION destination_dir] # Defaults to `target`
#   [MODULE module]  # Defaults to `target`
#   [CONFIGURATION config_file]
#   [NAMESPACES namespace1 namespace2 ...])
#   [DEBUG]
#   [EXCLUDE_FROM_ALL]
#   [COPY_MODULE]
#   [INCLUDE_DIRS dir1 [dir2 ...]]
#   [LINK_LIBRARIES lib1 [lib2 ...]]
#
function(chimera_add_binding_test_boost_python test_name)
  if(NOT CHIMERA_TEST_Boost_PYTHON_LIBRARIES)
    message(WARNING "Skipping '${test_name}' test because Boost.Python is not found")
    return()
  endif()

  include(ExternalProject)

  # Parse boolean, unary, and list arguments from input.
  # Unparsed arguments can be found in variable ARG_UNPARSED_ARGUMENTS.
  set(prefix chimera_test)
  set(options DEBUG EXCLUDE_FROM_ALL COPY_MODULE)
  set(oneValueArgs TARGET MODULE CONFIGURATION DESTINATION)
  set(multiValueArgs SOURCES NAMESPACES EXTRA_SOURCES INCLUDE_DIRS LINK_LIBRARIES)
  cmake_parse_arguments(
    "${prefix}" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN}
  )

  if(chimera_test_EXTRA_SOURCES)
    set(chimera_binding_EXTRA_SOURCES EXTRA_SOURCES ${chimera_test_EXTRA_SOURCES})
  endif()

  if(chimera_test_DESTINATION)
    set(chimera_binding_DESTINATION DESTINATION ${chimera_test_DESTINATION})
  endif()

  if(chimera_test_MODULE)
    set(chimera_binding_MODULE MODULE ${chimera_test_MODULE})
  endif()

  if(chimera_test_CONFIGURATION)
    set(chimera_binding_CONFIGURATION CONFIGURATION ${chimera_test_CONFIGURATION})
  endif()

  if(chimera_test_NAMESPACES)
    set(chimera_binding_NAMESPACES NAMESPACES ${chimera_test_NAMESPACES})
  endif()

  if(chimera_test_DEBUG)
    set(chimera_binding_DEBUG DEBUG)
  endif()

  if(chimera_test_EXCLUDE_FROM_ALL)
    set(chimera_binding_EXCLUDE_FROM_ALL EXCLUDE_FROM_ALL)
  endif()

  add_chimera_binding(
    TARGET ${test_name}
    SOURCES ${chimera_test_SOURCES}
    BINDING boost_python
    ${chimera_binding_EXTRA_SOURCES}
    ${chimera_binding_DESTINATION}
    ${chimera_binding_MODULE}
    ${chimera_binding_CONFIGURATION}
    ${chimera_binding_NAMESPACES}
    ${chimera_binding_DEBUG}
    ${chimera_binding_EXCLUDE_FROM_ALL}
  )

  set_target_properties(${test_name} PROPERTIES PREFIX "")

  target_compile_options(${test_name}
    PUBLIC "-std=c++11"
  )

  target_include_directories(${test_name}
    PUBLIC
      ${Boost_INCLUDE_DIRS}
      ${PYTHON_INCLUDE_DIRS}
      ${chimera_test_INCLUDE_DIRS}
  )

  target_link_libraries(${test_name}
    PUBLIC
      ${Boost_LIBRARIES}
      ${CHIMERA_TEST_Boost_PYTHON_LIBRARIES}
      ${PYTHON_LIBRARIES}
      ${chimera_test_LINK_LIBRARIES}
  )

  add_test(
    NAME ${test_name}
    COMMAND "${CMAKE_COMMAND}"
      --build ${CMAKE_BINARY_DIR}
      --target ${test_name}
  )

  if(chimera_test_COPY_MODULE)
    add_custom_command(
      TARGET ${test_name}
      POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${test_name}*.so ${CMAKE_CURRENT_SOURCE_DIR}
    )
  endif()

  clang_format_add_sources(${chimera_test_SOURCES})
  clang_format_add_sources(${chimera_test_EXTRA_SOURCES})

  set_property(GLOBAL APPEND PROPERTY CHIMERA_BOOST_PYTHON_BINDING_TESTS ${test_name})
endfunction(chimera_add_binding_test_boost_python)
