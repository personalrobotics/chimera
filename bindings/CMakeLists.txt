# List of language bindings that are assumed to be in corresponding
# subdirectories of this folder.
set(binding_LIST "boost_python")

# Iterate over each available language binding and generate a C++ source file.
# Note: BINDING_NAME has non-standard capitalization because it is directly
# used in some of the template substitutions, which are generally ALL-CAPS.
foreach(BINDING_NAME IN LISTS binding_LIST)
    set(binding_IMPL "${BINDING_NAME}_binding-impl.h")

    # Set variables which will be used in binding template.
    file(READ "${BINDING_NAME}/class.cpp.tmpl" BINDING_CLASS_CPP)
    file(READ "${BINDING_NAME}/enum.cpp.tmpl" BINDING_ENUM_CPP)
    file(READ "${BINDING_NAME}/function.cpp.tmpl" BINDING_FUNCTION_CPP)
    file(READ "${BINDING_NAME}/variable.cpp.tmpl" BINDING_VARIABLE_CPP)
    file(READ "${BINDING_NAME}/module.cpp.tmpl" BINDING_MODULE_CPP)

    # Generate a binding from file template and add to implementation list,
    # then append the implementation to variables that are used to create a
    # single binding library.
    configure_file(binding-impl.h.tmpl "${binding_IMPL}"
        @ONLY NEWLINE_STYLE UNIX
    )
    list(APPEND binding_IMPLS "${binding_IMPL}")
    list(APPEND BINDING_INCLUDES_LIST
        "#include <${binding_IMPL}>")
    list(APPEND BINDING_REGISTRATIONS_LIST
        "DEFINITIONS[\"${BINDING_NAME}\"] = ${BINDING_NAME}_DEFINITION;")
endforeach()

# Create a binding library which contains all built-in bindings.
configure_file(binding.cpp.tmpl binding.cpp
    @ONLY NEWLINE_STYLE UNIX
)
add_library(chimera_bindings STATIC binding.cpp ${binding_IMPLS})
set(CMAKE_INCLUDE_CURRENT_DIR ON)
