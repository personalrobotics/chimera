# Include helpers to define tests for each language.
include(chimeraTestBoostPython)
include(chimeraTestPybind11)

# Define a single helper function that runs all tests using the same configuration.
# (This can be used in simpler tests where all of the bindings can use the same
# configuration YAML.)
#
# chimera_add_binding_tests(target
#   SOURCES source1_file [source2_file ...]
#   [EXTRA_SOURCES source1_file ...]
#   [DESTINATION destination_dir] # Defaults to `target`
#   [MODULE module]  # Defaults to `target`
#   [CONFIGURATION config_file]
#   [NAMESPACES namespace1 namespace2 ...])
#   [DEBUG]
#   [EXCLUDE_FROM_ALL]
#   [COPY_MODULE]
#
function(chimera_add_binding_tests test_name)
  chimera_add_binding_test_boost_python(${test_name}_boost_python ${ARGN})
  chimera_add_binding_test_pybind11(${test_name}_pybind11 ${ARGN})
endfunction(chimera_add_binding_tests)

# Add a Python test that will run against the generated code. The Python test
# will be able to run through CTest
#
# chimera_add_binding_tests(test_name test_script)
function(chimera_add_python_test test_name python_filename)
  add_test(
    NAME ${test_name}
    COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/${python_filename}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  )
endfunction(chimera_add_python_test)
