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
    find_package(chimera REQUIRED)

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

    # Create a placeholder library that depends on the provided sources.  This is used to
    # generate the necessary compilation database.
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
    set(COMPILATION_DATABASE "${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json")
    add_library("${ARG_TARGET}_placeholder" SHARED EXCLUDE_FROM_ALL ${ARG_SOURCES})

    # Create an external target that re-runs chimera when any of the sources have changed.
    # This will necessarily invalidate a placeholder dependency that causes CMake to
    # rerun the compilation of the library if sources are regenerated.
    add_custom_command(
        PREBUILD
        OUTPUT "${ARG_TARGET}_sources.txt"
        COMMAND "${chimera_EXECUTABLE}"
            -m "${ARG_MODULE}"
            # -c "${ARG_CONFIGURATION}"
            -o "${ARG_DESTINATION}"
            -p "${COMPILATION_DATABASE}"
            ${ARG_SOURCES}
            > "$<TARGET_FILE>"
        SOURCES
            "${ARG_CONFIGURATION}"
            "${COMPILATION_DATABASE}"
            ${ARG_SOURCES}
        COMMENT "Generating chimera binding for '${ARG_TARGET}'."
        VERBATIM
    )

    # Load the file auto-generated by Chimera.
    file(STRINGS "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TARGET}_sources.txt"
      GENERATED_SOURCES
      NO_HEX_CONVERSION
    )

    # Combine the sources generated by chimera to create a new binding.
    add_library("${ARG_TARGET}"
        ${GENERATED_SOURCES}
    )
    add_dependencies("${ARG_TARGET}" "${CMAKE_CURRENT_BINARY_DIR}/${ARG_TARGET}_sources.txt")

endfunction(add_chimera_binding)