# Define function for pybind11 binding tests
#
# chimera_add_binding_test_pybind11(target
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
# Note: Specifying different binding other than pybind11 will results in
# undefined behavior. It'd be great if we can prevent this situation.
function(chimera_add_binding_test_pybind11 test_name)
  if(NOT pybind11_FOUND)
    message(WARNING "Skipping '${test_name}' test because pybind11 is not found")
    return()
  endif()

  include(ExternalProject)

  # Parse boolean, unary, and list arguments from input.
  # Unparsed arguments can be found in variable ARG_UNPARSED_ARGUMENTS.
  set(prefix chimera_test)
  set(options DEBUG EXCLUDE_FROM_ALL COPY_MODULE)
  set(oneValueArgs TARGET MODULE CONFIGURATION DESTINATION)
  set(multiValueArgs SOURCES NAMESPACES EXTRA_SOURCES LINK_LIBRARIES)
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

  if(chimera_test_INCLUDE_DIRS)
    set(chimera_binding_INCLUDE_DIRS INCLUDE_DIRS ${chimera_test_INCLUDE_DIRS})
  endif()

  if(chimera_test_LINK_LIBRARIES)
    set(chimera_binding_LINK_LIBRARIES LINK_LIBRARIES ${chimera_test_LINK_LIBRARIES})
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
    BINDING pybind11
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
      ${PYTHON_INCLUDE_DIRS}
      ${chimera_test_INCLUDE_DIRS}
  )

  target_link_libraries(${test_name}
    PUBLIC
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

  set_property(GLOBAL APPEND PROPERTY CHIMERA_PYBIND11_BINDING_TESTS ${test_name})
endfunction(chimera_add_binding_test_pybind11)
