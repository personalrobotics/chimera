#
# Contains helper functions for running Chimera in other CMake projects.
#

# TODO: DO NOT COMMIT THIS!
cmake_policy(SET CMP0026 OLD)

# Chimera requires the generation of a compilation database.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Function to create a build target for a Chimera binding.
# The result of this operation is a CMake library target.
#
# add_chimera_binding(TARGET target
#                     DESTINATION destination_dir
#                     [MODULE module]  # Defaults to `target`
#                     [CONFIGURATION config_file]
#                     [NAMESPACE namespace1 namespace2 namespace3])
#                     [SOURCES source1_file source2_file ...]
function(add_chimera_binding)
    find_package(chimera REQUIRED)
    include(ExternalProject)

    # Parse boolean, unary, and list arguments from input.
    # Unparsed arguments can be found in variable ARG_UNPARSED_ARGUMENTS.
    set(prefix binding)
    set(options DEBUG)
    set(oneValueArgs TARGET MODULE CONFIGURATION DESTINATION)
    set(multiValueArgs SOURCES NAMESPACES)
    cmake_parse_arguments("${prefix}" "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    # Print errors if arguments are missing.
    if(NOT binding_TARGET)
        message(FATAL_ERROR "Chimera requires a CMake TARGET name.")
    elseif(NOT binding_SOURCES)
        message(FATAL_ERROR "Chimera requires one or more SOURCES.")
    endif()

    # Default MODULE to be the same as TARGET.
    if(NOT binding_MODULE)
        set(binding_MODULE "${binding_TARGET}")
    endif()

    # Default DESTINATION to `<BINARY_DIR>/<TARGET>`
    if(NOT binding_DESTINATION)
        set(binding_DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/${binding_TARGET}")
    endif()

    # If the debug flag is specified, dump all the chimera settings.
    if(binding_DEBUG)
        message(STATUS "Chimera binding: ${binding_TARGET}")
        message(STATUS "  Configuration:")
        message(STATUS "  Sources:")
        foreach(src ${binding_SOURCES})
            message(STATUS "  - ${src}")
        endforeach(src)
        message(STATUS "  Namespaces:")
        foreach(src ${binding_SOURCES})
            message(STATUS "  - ${src}")
        endforeach(src)
    endif()

    # Create a placeholder for the list of generated sources.
    file(APPEND "${binding_DESTINATION}/sources.txt" "")

    # Create an external target that re-runs chimera when any of the sources have changed.
    # This will necessarily invalidate a placeholder dependency that causes CMake to
    # rerun the compilation of the library if sources are regenerated.
    add_custom_target("${binding_TARGET}_SOURCES" DEPENDS "${binding_DESTINATION}/sources.txt")
    add_custom_command(
        TARGET "${binding_TARGET}_SOURCES" PRE_BUILD
        # OUTPUT "${binding_DESTINATION}/sources.txt"
        COMMAND "${chimera_EXECUTABLE}"
        ARGS -m "${binding_MODULE}"
             -o "${binding_DESTINATION}"
             -p "${PROJECT_BINARY_DIR}"
             ${binding_SOURCES}
             > "${binding_DESTINATION}/sources.txt"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        DEPENDS ${binding_SOURCES}
        COMMENT "Generating bindings for ${binding_TARGET}."
        VERBATIM
    )
    configure_file("${binding_DESTINATION}/sources.txt" "${binding_DESTINATION}/sources.txt" COPYONLY)

    # Get the current list of generated sources and create a library target.
    file(STRINGS "${binding_DESTINATION}/sources.txt" binding_GENERATED NO_HEX_CONVERSION)
    add_library("${binding_TARGET}" MODULE
        ${binding_SOURCES}
        ${binding_GENERATED}
    )
    add_dependencies("${binding_TARGET}" "${binding_TARGET}_SOURCES")

    # Trigger the rebuild of the library target after new sources have been generated.
    ExternalProject_Add("${binding_TARGET}_postbuild"
        DOWNLOAD_COMMAND ""
        INSTALL_COMMAND ""
        BUILD_COMMAND make "${binding_TARGET}"
        DEPENDS "${binding_TARGET}_SOURCES"
        SOURCE_DIR "${PROJECT_SOURCE_DIR}"
        BINARY_DIR "${PROJECT_BINARY_DIR}"
    )

endfunction(add_chimera_binding)