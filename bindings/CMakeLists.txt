# List of language bindings that are assumed to be in corresponding
# subdirectories of this folder.
set(binding_LIST "boost_python")

# Iterate over each available language binding and generate a C++ source file.
foreach(binding_NAME IN LISTS binding_LIST)
    set(binding_IMPL "${binding_NAME}_binding-impl.h")
    set(binding_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${binding_NAME}")

    # Generate a binding from file template and add to implementation list,
    # then append the implementation to variables that are used to create a
    # single binding library.
    add_custom_command(OUTPUT "${binding_IMPL}"
      COMMAND ${CMAKE_COMMAND}
        "-DBINDING_PATH=${binding_PATH}"
        "-DBINDING_OUTPUT=${binding_IMPL}"
        "-DBINDING_NAME=${binding_NAME}"
        "-DBINDING_TEMPLATE=${CMAKE_CURRENT_SOURCE_DIR}/binding-impl.h.tmpl"
        -P "${PROJECT_SOURCE_DIR}/cmake/generateChimeraBinding.cmake"
      DEPENDS
        "${binding_PATH}/class.cpp.tmpl"
        "${binding_PATH}/enum.cpp.tmpl"
        "${binding_PATH}/function.cpp.tmpl"
        "${binding_PATH}/variable.cpp.tmpl"
        "${binding_PATH}/module.cpp.tmpl"
      COMMENT "Importing binding definition for '${binding_NAME}'."
      VERBATIM
    )

    # Add generated implementation header to list of dependencies.
    list(APPEND binding_IMPLS "${binding_IMPL}")
    list(APPEND BINDING_INCLUDES_LIST
        "#include <${binding_IMPL}>")
    list(APPEND BINDING_REGISTRATIONS_LIST
        "DEFINITIONS[\"${binding_NAME}\"] = ${binding_NAME}_DEFINITION;")
endforeach()

# Create a binding library which contains all built-in bindings.
configure_file(binding.cpp.tmpl binding.cpp
    @ONLY NEWLINE_STYLE UNIX
)
add_library(chimera_bindings STATIC binding.cpp ${binding_IMPLS})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
