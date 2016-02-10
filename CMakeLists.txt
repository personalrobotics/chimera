project(chimera)
cmake_minimum_required(VERSION 3.0.2)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

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
add_executable(chimera
  src/boost_python_mstch.cpp
  src/chimera.cpp
  src/configuration.cpp
  src/consumer.cpp
  src/frontend_action.cpp
  src/visitor.cpp
  src/util.cpp
  src/mstch.cpp
)

target_link_libraries(chimera
  ${YAMLCPP_LIBRARIES}
  ${CLANG_LIBS}
  ${llvm_libs}
)

target_link_libraries(chimera
  mstch
  cling_utils
)
