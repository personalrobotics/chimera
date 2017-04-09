# Run a CMake script which loads bindings and assembles them into a header.
#
# Requires the following CMake environment variables to be set:
#   BINDING_PATH - Path to .tmpl files for each CXX type.
#   BINDING_TEMPLATE - Path to .tmpl file defining the output header.
#   BINDING_OUTPUT - Path to output header that will be generated.

# Load variables which will be used in binding template.
file(READ "${BINDING_PATH}/class.cpp.tmpl" BINDING_CLASS_CPP)
file(READ "${BINDING_PATH}/enum.cpp.tmpl" BINDING_ENUM_CPP)
file(READ "${BINDING_PATH}/function.cpp.tmpl" BINDING_FUNCTION_CPP)
file(READ "${BINDING_PATH}/variable.cpp.tmpl" BINDING_VARIABLE_CPP)
file(READ "${BINDING_PATH}/module.cpp.tmpl" BINDING_MODULE_CPP)

# Uses a binding template to assemble the above files into a header.
configure_file("${BINDING_TEMPLATE}" "${BINDING_OUTPUT}"
    @ONLY NEWLINE_STYLE UNIX
)
