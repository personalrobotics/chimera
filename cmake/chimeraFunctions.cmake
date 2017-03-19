#
# Contains helper functions for running Chimera in other CMake projects.
#

# Chimera requires the generation of a compilation database.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Function to convert the HTML special entities accordingly (e.g., &lt; &gt;
# &amp; to <, >, &, respectively).
# This function is a workaround to rectify unintended conversions by one of the
# dependencies of Chimera.
#
# rectify_html_special_entities(file1 [file2 [...]])
function(rectify_html_special_entities)
    foreach(file ${ARGN})
        file(READ "${file}" file_string)
        string(REPLACE "&lt;" "<" file_string "${file_string}")
        string(REPLACE "&gt;" ">" file_string "${file_string}")
        string(REPLACE "&amp;" "&" file_string "${file_string}")
        file(WRITE "${file}" "${file_string}")
    endforeach()
endfunction()

# Function to create a build target for a Chimera binding.
# The result of this operation is a CMake library target.
#
# add_chimera_binding(TARGET target
#                     [DESTINATION destination_dir] # Defaults to `target`
#                     [MODULE module]  # Defaults to `target`
#                     [CONFIGURATION config_file]
#                     [NAMESPACES namespace1 namespace2 ...])
#                     SOURCES source1_file [source2_file ...]
#                     [EXTRA_SOURCES source1_file ...]
#                     [LINK_LIBRARIES item1 [item2 [...]]]
#                     [DEBUG] [EXCLUDE_FROM_ALL]
function(add_chimera_binding)
    include(ExternalProject)

    # Parse boolean, unary, and list arguments from input.
    # Unparsed arguments can be found in variable ARG_UNPARSED_ARGUMENTS.
    set(prefix binding)
    set(options DEBUG EXCLUDE_FROM_ALL)
    set(oneValueArgs TARGET MODULE CONFIGURATION DESTINATION)
    set(multiValueArgs SOURCES NAMESPACES EXTRA_SOURCES LINK_LIBRARIES)
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
        message(STATUS "  Extra Sources:")
        foreach(source ${binding_EXTRA_SOURCES})
            message(STATUS "  - ${source}")
        endforeach()
        message(STATUS "  Link Libraries:")
        foreach(library ${binding_LINK_LIBRARIES})
            message(STATUS "  - ${library}")
        endforeach()
    endif()

    # Ensure that the output destination directory is created.
    file(MAKE_DIRECTORY "${binding_DESTINATION}")
    file(WRITE "${binding_DESTINATION}/empty.cpp" "")

    # Construct the list of chimera arguments.
    set(binding_ARGS)
    list(APPEND binding_ARGS -m "${binding_MODULE}")
    list(APPEND binding_ARGS -o "${binding_DESTINATION}")
    list(APPEND binding_ARGS -p "${PROJECT_BINARY_DIR}")
    if(binding_CONFIGURATION)
        list(APPEND binding_ARGS -c "${binding_CONFIGURATION}")
    endif()
    if(binding_NAMESPACES)
        foreach(namespace ${binding_NAMESPACES})
            list(APPEND binding_ARGS -n "${namespace}")
        endforeach()
    endif()
    list(APPEND binding_ARGS ${binding_SOURCES})
    list(APPEND binding_ARGS > "${binding_DESTINATION}/sources.txt")

    # Create an external target that re-runs chimera when any of the sources have changed.
    # This will necessarily invalidate a placeholder dependency that causes CMake to
    # rerun the compilation of the library if sources are regenerated.
    add_custom_target("${binding_TARGET}_SOURCES" DEPENDS "${binding_DESTINATION}/sources.txt")
    add_custom_command(
        OUTPUT "${binding_DESTINATION}/sources.txt"
        COMMAND "${chimera_EXECUTABLE}"
        ARGS ${binding_ARGS}
        DEPENDS "${binding_CONFIGURATION}" ${binding_SOURCES}
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Generating bindings for ${binding_TARGET}."
        VERBATIM
    )

    # Get the current list of generated sources if already generated.
    if(EXISTS "${binding_DESTINATION}/sources.txt")
        file(STRINGS "${binding_DESTINATION}/sources.txt" binding_GENERATED_RELATIVE NO_HEX_CONVERSION)

        set(binding_GENERATED)
        foreach(relative_path ${binding_GENERATED_RELATIVE})
            list(APPEND binding_GENERATED "${binding_DESTINATION}/${relative_path}")
        endforeach()

        rectify_html_special_entities(${binding_GENERATED})
    endif()

    # Placeholder target to generate compilation database.
    add_library("${binding_TARGET}_placeholder" EXCLUDE_FROM_ALL
        ${binding_SOURCES}
    )
    target_link_libraries("${binding_TARGET}_placeholder"
        ${binding_LINK_LIBRARIES}
    )
    set_target_properties("${binding_TARGET}_placeholder" PROPERTIES
        LINKER_LANGUAGE CXX
    )

    # Create a library target to build the binding as a module.
    add_library("${binding_TARGET}" MODULE
        "${binding_EXCLUDE_FROM_ALL_FLAG}"
        "${binding_DESTINATION}/empty.cpp"
        ${binding_GENERATED}
        ${binding_EXTRA_SOURCES}
    )

    # Trigger the rebuild of the library target after new sources have been generated.
    #
    # For BUILD_COMMAND, '$(MAKE)' is used instead of 'make' to propagate the
    # make commands of the parent project to the child process.
    # (see: http://stackoverflow.com/a/33171336)
    ExternalProject_Add("${binding_TARGET}_REBUILD"
        DOWNLOAD_COMMAND ""
        INSTALL_COMMAND ""
        BUILD_COMMAND $(MAKE) "${binding_TARGET}_SOURCES"
        DEPENDS "${binding_TARGET}_SOURCES"
        SOURCE_DIR "${PROJECT_SOURCE_DIR}"
        BINARY_DIR "${PROJECT_BINARY_DIR}"
    )
    set_target_properties("${binding_TARGET}_REBUILD" PROPERTIES EXCLUDE_FROM_ALL TRUE)
    add_dependencies("${binding_TARGET}" "${binding_TARGET}_REBUILD")

endfunction(add_chimera_binding)
