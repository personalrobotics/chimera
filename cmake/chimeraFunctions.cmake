#
# Contains helper functions for running Chimera in other CMake projects.
#

#
# Function to create a build target for a Chimera binding.
# The result of this operation is a CMake library target.
#
# add_chimera_binding(TARGET target
#                     DESTINATION destination_dir
#                     [MODULE module]  # Defaults to `target`
#                     [CONFIGURATION config_file]
#                     [SOURCES source1_file source2_file ...]
#                     [NAMESPACE namespace1 namespace2 namespace3])
function(add_chimera_binding)
    # Parse boolean, unary, and list arguments from input.
    # Unparsed arguments can be found in variable ARG_UNPARSED_ARGUMENTS.
    set(prefix ARG)
    set(options DEBUG)
    set(oneValueArgs TARGET MODULE CONFIGURATION DESTINATION)
    set(multiValueArgs SOURCES NAMESPACES)
    cmake_parse_arguments("${prefix}" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Print errors if arguments are missing.
    if(NOT ARG_TARGET)
        message(FATAL_ERROR "Chimera requires a CMake TARGET name.")
    elseif(NOT ARG_SOURCES)
        message(FATAL_ERROR "Chimera requires one or more SOURCES.")
    endif()

    # Default MODULE to be the same as TARGET.
    if(NOT ARG_MODULE)
        set(ARG_MODULE "${ARG_TARGET}")
    endif()

    # Default DESTINATION to `./BINARY_DIR/TARGET`
    if(NOT ARG_DESTINATION)
        set(ARG_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TARGET}")
    endif()

    message("Provided sources are:")
    foreach(src ${ARG_SOURCES})
        message("- ${src}")
    endforeach(src)
endfunction(add_chimera_binding)