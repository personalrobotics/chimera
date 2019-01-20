project(chimera)
cmake_minimum_required(VERSION 2.8.12)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

set(RUNTIME_INSTALL_DIR "bin")
set(SHARE_INSTALL_DIR "share/${PROJECT_NAME}")

###########
## SETUP ##
###########

# Chimera version
set(CHIMERA_MAJOR_VERSION "0")
set(CHIMERA_MINOR_VERSION "1")
set(CHIMERA_PATCH_VERSION "0")
set(CHIMERA_VERSION
  ${CHIMERA_MAJOR_VERSION}.${CHIMERA_MINOR_VERSION}.${CHIMERA_PATCH_VERSION}
)

# Preset for code formatting
include(ClangFormat)
clang_format_setup()

## Find necessary packages.
find_package(LLVM REQUIRED CONFIG)
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

find_package(Clang REQUIRED)
include_directories(SYSTEM ${CLANG_INCLUDE_DIRS})
add_definitions(${CLANG_DEFINITIONS})

set(YAMLCPP_STATIC_LIBRARY ON CACHE BOOL
  "If true, try to find static yaml-cpp library first instead of dynamic.")
find_package(YamlCpp REQUIRED)
include_directories(SYSTEM ${YAMLCPP_INCLUDE_DIRS})
add_definitions(${YAMLCPP_DEFINITIONS})

find_package(Boost REQUIRED)
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

## Set up default compiler options.
if (NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE RelWithDebugInfo)
endif()

if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
  set(CMAKE_COMPILER_IS_CLANGCXX TRUE)
endif()

if ("${CMAKE_GENERATOR}" STREQUAL "Ninja")
  if (CMAKE_COMPILER_IS_CLANGCXX)
    add_definitions("-fcolor-diagnostics")
  endif()
endif()

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_CLANGCXX)
  add_compile_options("-Wextra" "-Wall" "-Wstrict-aliasing=2")
endif()
add_compile_options("-Wno-unused-parameter")

###################
## CodeCov SETUP ##
###################
option(CODECOV "Enable CodeCov support" OFF)
if(CODECOV)
  include(CodeCoverage)
  setup_target_for_coverage(${PROJECT_NAME}_coverage ctest coverage)
  set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage"
  )
endif()

###########
## BUILD ##
###########

# Globally inherit the top-level include directory because it is difficult to
# locate from CMakeLists.txt in source subdirectories.
include_directories("${PROJECT_SOURCE_DIR}/include")

# Set headers and sources
set(${PROJECT_NAME}_HEADERS
  include/chimera/binding.h
  include/chimera/chimera.h
  include/chimera/configuration.h
  include/chimera/consumer.h
  include/chimera/frontend_action.h
  include/chimera/mstch.h
  include/chimera/util.h
  include/chimera/visitor.h
)
set(${PROJECT_NAME}_SOURCES
  src/chimera.cpp
  src/configuration.cpp
  src/consumer.cpp
  src/frontend_action.cpp
  src/visitor.cpp
  src/util.cpp
  src/mstch.cpp
)

## Use LLVM CMake macro to collate and order link libraries.
llvm_map_components_to_libnames(llvm_libs
  core
  irreader
  ipo
  mcparser
  option
)

# Build the external tools: Mustache and Cling.
add_subdirectory(external)

# Build the chimera library.
add_library(libchimera
  ${${PROJECT_NAME}_HEADERS}  # not required, but helpful for some IDEs
  ${${PROJECT_NAME}_SOURCES}
)
target_compile_options(libchimera PUBLIC "-std=c++11")
target_link_libraries(libchimera
  PRIVATE
    ${YAMLCPP_LIBRARIES}
    ${CLANG_LIBS}
    ${llvm_libs}
)
target_link_libraries(libchimera PRIVATE mstch cling_utils)
target_link_libraries(libchimera PRIVATE chimera_bindings)
target_compile_definitions(libchimera
  PUBLIC
    CHIMERA_MAJOR_VERSION=${CHIMERA_MAJOR_VERSION}
    CHIMERA_MINOR_VERSION=${CHIMERA_MINOR_VERSION}
    CHIMERA_PATCH_VERSION=${CHIMERA_PATCH_VERSION}
    CHIMERA_VERSION=${CHIMERA_VERSION}
)
clang_format_add_sources(${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})

# Build the main chimera executable.
add_executable(chimera src/chimera_main.cpp)
target_link_libraries(chimera PUBLIC libchimera)
clang_format_add_sources(src/chimera_main.cpp)

# Generate built-in template bindings for supported languages.
add_subdirectory(bindings)

# Add main sources for code formatting
clang_format_add_sources(
  ${${PROJECT_NAME}_HEADERS}
  ${${PROJECT_NAME}_SOURCES}
)

#############
## INSTALL ##
#############
install(TARGETS "${PROJECT_NAME}"
  EXPORT "${PROJECT_NAME}Targets"
  RUNTIME DESTINATION "${RUNTIME_INSTALL_DIR}"
)
install(EXPORT "${PROJECT_NAME}Targets"
  FILE "${PROJECT_NAME}Targets.cmake"
  DESTINATION "${SHARE_INSTALL_DIR}/cmake"
)
install(FILES "package.xml"
  DESTINATION "${SHARE_INSTALL_DIR}"
)

# Generate and install a Config.cmake file.
# TODO: We should also generate a _VERSION file.
include(CMakePackageConfigHelpers)
configure_package_config_file("cmake/${PROJECT_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${SHARE_INSTALL_DIR}/cmake"
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
              "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Functions.cmake"
  DESTINATION "${SHARE_INSTALL_DIR}/cmake"
)

# Enable unit testing by including the `test` subdirectory.
enable_testing()
add_subdirectory(test EXCLUDE_FROM_ALL)

#####################
## CODE FORMATTING ##
#####################

# Define 'format' and 'check-format' targets
clang_format_add_targets()
