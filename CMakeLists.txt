project(chimera)
cmake_minimum_required(VERSION 2.8.12)
list(APPEND CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake/")

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

# Set headers
set(chimera_headers
  ${PROJECT_SOURCE_DIR}/include/binding.h
  ${PROJECT_SOURCE_DIR}/include/configuration.h
  ${PROJECT_SOURCE_DIR}/include/consumer.h
  ${PROJECT_SOURCE_DIR}/include/frontend_action.h
  ${PROJECT_SOURCE_DIR}/include/mstch.h
  ${PROJECT_SOURCE_DIR}/include/util.h
  ${PROJECT_SOURCE_DIR}/include/visitor.h
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

# Build the main chimera executable.
add_executable("${PROJECT_NAME}"
  ${chimera_headers}  # not required, but helpful for some IDEs
  src/chimera.cpp
  src/configuration.cpp
  src/consumer.cpp
  src/frontend_action.cpp
  src/visitor.cpp
  src/util.cpp
  src/mstch.cpp
)

target_compile_options("${PROJECT_NAME}"
  PUBLIC "-std=c++11"
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

# Generate built-in template bindings for supported languages.
add_subdirectory(bindings)
target_link_libraries(chimera chimera_bindings)

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
option(BUILD_TESTING "Enable generation of Chimera build tests." ON)
if(BUILD_TESTING)
  enable_testing()
  add_subdirectory(test)
endif()
