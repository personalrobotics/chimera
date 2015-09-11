project(kludge)
cmake_minimum_required(VERSION 2.8)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake/")

###########
## SETUP ##
###########

## Find necessary packages.
find_package(LLVM REQUIRED)
find_package(Clang REQUIRED)

include_directories(${LLVM_INCLUDE_DIRS})
link_directories(${LLVM_LIBRARY_DIRS})
add_definitions(${LLVM_CXXFLAGS})

include_directories(${CLANG_INCLUDE_DIRS})
link_directories(${CLANG_LIBRARY_DIRS})

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

add_executable(test src/test.cpp)
target_link_libraries(test
                      ${LLVM_MODULE_LIBS}
                      ${CLANG_LIBS})
