#
# Contains helper functions for running Chimera in other CMake projects.
#

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
#                     SOURCES source1_file [source2_file ...]
#                     [DEBUG] [EXCLUDE_FROM_ALL]
function(add_chimera_binding)
    find_package(chimera REQUIRED)
    include(ExternalProject)

    # Parse boolean, unary, and list arguments from input.
    # Unparsed arguments can be found in variable ARG_UNPARSED_ARGUMENTS.
    set(prefix binding)
    set(options DEBUG EXCLUDE_FROM_ALL)
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

    # Set EXCLUDE_FROM_ALL flag if provided.
    if(binding_EXCLUDE_FROM_ALL)
        set(binding_EXCLUDE_FROM_ALL_FLAG "EXCLUDE_FROM_ALL")
    else()
        set(binding_EXCLUDE_FROM_ALL_FLAG "")
    endif()

    # If the debug flag is specified, dump all the chimera settings.
    if(binding_DEBUG)
        message(STATUS "Chimera binding: ${binding_TARGET}")
        message(STATUS "  Exclude from ALL: ${binding_EXCLUDE_FROM_ALL}")
        message(STATUS "  Module: ${binding_MODULE}")
        message(STATUS "  Configuration: ${binding_CONFIGURATION}")
        message(STATUS "  Namespaces:")
        foreach(namespace ${binding_NAMESPACES})
            message(STATUS "  - ${namespace}")
        endforeach()
        message(STATUS "  Sources:")
        foreach(source ${binding_SOURCES})
            message(STATUS "  - ${source}")
        endforeach()
    endif()

    # Ensure that the output destination directory is created.
    file(MAKE_DIRECTORY "${binding_DESTINATION}")

    # Create an external target that re-runs chimera when any of the sources have changed.
    # This will necessarily invalidate a placeholder dependency that causes CMake to
    # rerun the compilation of the library if sources are regenerated.
    add_custom_target("${binding_TARGET}_SOURCES" DEPENDS "${binding_DESTINATION}/sources.txt")
    add_custom_command(
        OUTPUT "${binding_DESTINATION}/sources.txt"
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

    # Get the current list of generated sources if already generated.
    if(EXISTS "${binding_DESTINATION}/sources.txt")
        file(STRINGS "${binding_DESTINATION}/sources.txt" binding_GENERATED NO_HEX_CONVERSION)
    endif()

    # Create a library target to build the binding as a module.
    add_library("${binding_TARGET}" MODULE
        ${binding_EXCLUDE_FROM_ALL_FLAG}
        ${binding_SOURCES}
        ${binding_GENERATED}
    )

    # Trigger the rebuild of the library target after new sources have been generated.
    ExternalProject_Add("${binding_TARGET}_REBUILD"
        DOWNLOAD_COMMAND ""
        INSTALL_COMMAND ""
        BUILD_COMMAND make "${binding_TARGET}_SOURCES"
        DEPENDS "${binding_TARGET}_SOURCES"
        SOURCE_DIR "${PROJECT_SOURCE_DIR}"
        BINARY_DIR "${PROJECT_BINARY_DIR}"
    )
    set_target_properties("${binding_TARGET}_REBUILD" PROPERTIES EXCLUDE_FROM_ALL TRUE)
    add_dependencies("${binding_TARGET}" "${binding_TARGET}_REBUILD")

endfunction(add_chimera_binding)