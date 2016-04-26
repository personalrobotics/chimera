project(chimera)
cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

set(RUNTIME_INSTALL_DIR "bin")
set(SHARE_INSTALL_DIR "share/${PROJECT_NAME}")

###########
## SETUP ##
###########

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
include_directories(${YAMLCPP_INCLUDE_DIRS})
add_definitions(${YAMLCPP_DEFINITIONS})

## Set up glorious compiler options.
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
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra -Wall -Wstrict-aliasing=2")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -fno-rtti -fno-exceptions")
endif()

###########
## BUILD ##
###########
include_directories("${CMAKE_CURRENT_LIST_DIR}/include")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++11 -Wno-unused-parameter")

## Use LLVM CMake macro to collate and order link libraries.
llvm_map_components_to_libnames(llvm_libs
  core
  irreader
  mcparser
  option
)

# Build the external tools: Mustache and Cling.
add_subdirectory(external/mstch)
include_directories(${mstch_INCLUDE_DIR})

add_subdirectory(external/cling)
include_directories(${cling_utils_INCLUDE_DIR})

# Build the main chimera executable.
add_executable("${PROJECT_NAME}"
  src/boost_python_mstch.cpp
  src/chimera.cpp
  src/configuration.cpp
  src/consumer.cpp
  src/frontend_action.cpp
  src/visitor.cpp
  src/util.cpp
  src/mstch.cpp
)

target_link_libraries("${PROJECT_NAME}"
  ${YAMLCPP_LIBRARIES}
  ${CLANG_LIBS}
  ${llvm_libs}
)

target_link_libraries("${PROJECT_NAME}"
  mstch
  cling_utils
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
#add_subdirectory(test)
