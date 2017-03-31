# Build a small subcomponent of cling using a custom `CMakeLists.txt`.
# This `CMakeLists.txt` has no install step so we can directly include it.
add_subdirectory(cling)

# Build mstch as *only* a static library for linking into chimera.
# Uses ExternalProject to prevent install steps from propagating to chimera.
include(ExternalProject)
ExternalProject_Add(mstch_EXTERNAL
    SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/mstch"
    DOWNLOAD_COMMAND ""
    INSTALL_COMMAND ""
    CMAKE_GENERATOR "${CMAKE_GENERATOR}"
)

# Retrieve properties of the ExternalProject to find build artifacts.
ExternalProject_Get_Property(mstch_EXTERNAL SOURCE_DIR BINARY_DIR)
set(mstch_INCLUDE_DIR "${SOURCE_DIR}/include")
set(mstch_LIBRARY "${BINARY_DIR}/src/${CMAKE_STATIC_LIBRARY_PREFIX}mstch${CMAKE_STATIC_LIBRARY_SUFFIX}")

# Create an IMPORTED target so that CMake can propagate the build properties of
# this ExternalProject to dependent targets.
# (Note: This target must be GLOBAL as per: http://stackoverflow.com/a/26315239)
add_library(mstch STATIC IMPORTED GLOBAL)
add_dependencies(mstch mstch_EXTERNAL)
set_target_properties(mstch PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${mstch_INCLUDE_DIR}"
    IMPORTED_LOCATION "${mstch_LIBRARY}"
)
