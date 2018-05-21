# List of language bindings that are assumed to be in corresponding
# subdirectories of this folder.
set(binding_LIST "boost_python" "pybind11")
set(binding_IMPL_TEMPLATE "${CMAKE_CURRENT_SOURCE_DIR}/binding-impl.h.tmpl")

# Iterate over each available language binding and generate a C++ source file.
foreach(binding_NAME IN LISTS binding_LIST)
    set(binding_IMPL_OUTPUT "${binding_NAME}_binding-impl.h")
    set(binding_IMPL_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${binding_NAME}")

    # Generate a binding from file template and add to implementation list,
    # then append the implementation to variables that are used to create a
    # single binding library.
    add_custom_command(OUTPUT "${binding_IMPL_OUTPUT}"
      COMMAND ${CMAKE_COMMAND}
        "-DBINDING_NAME=${binding_NAME}"
        "-DBINDING_PATH=${binding_IMPL_PATH}"
        "-DBINDING_OUTPUT=${binding_IMPL_OUTPUT}"
        "-DBINDING_TEMPLATE=${binding_IMPL_TEMPLATE}"
        -P "${PROJECT_SOURCE_DIR}/cmake/generateChimeraBinding.cmake"
      DEPENDS
        "${binding_IMPL_TEMPLATE}"
        "${binding_IMPL_PATH}/class.cpp.tmpl"
        "${binding_IMPL_PATH}/enum.cpp.tmpl"
        "${binding_IMPL_PATH}/function.cpp.tmpl"
        "${binding_IMPL_PATH}/variable.cpp.tmpl"
        "${binding_IMPL_PATH}/module.cpp.tmpl"
      COMMENT "Importing binding definition for '${binding_NAME}'."
      VERBATIM
    )

    # Add generated implementation header to list of dependencies.
    list(APPEND binding_IMPLS "${binding_IMPL_OUTPUT}")
    list(APPEND BINDING_INCLUDES_LIST
        "#include <${binding_IMPL_OUTPUT}>")
    list(APPEND BINDING_REGISTRATIONS_LIST
        "{ \"${binding_NAME}\", ${binding_NAME}_DEFINITION },")
endforeach()

string (REPLACE ";" "\n" BINDING_INCLUDES_LIST "${BINDING_INCLUDES_LIST}")
string (REPLACE ";" " " BINDING_REGISTRATIONS_LIST "${BINDING_REGISTRATIONS_LIST}")

# Create a binding library which contains all built-in bindings.
configure_file(binding.cpp.tmpl binding.cpp
    @ONLY NEWLINE_STYLE UNIX
)
add_library(chimera_bindings STATIC binding.cpp ${binding_IMPLS})
target_compile_options(chimera_bindings PUBLIC "-std=c++11")
set(CMAKE_INCLUDE_CURRENT_DIR ON)
