#===============================================================================
# Emulator
#===============================================================================
add_library(emulator emulator.h emulator.cpp)
target_link_libraries(emulator PUBLIC libchimera)
clang_format_add_sources(emulator.h emulator.cpp)
target_compile_definitions(emulator
  PUBLIC
    EXAMPLES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/examples/"
    BUILD_PATH="${CMAKE_BINARY_DIR}"
)

#===============================================================================
# GoogleTest setup
add_library(gtest STATIC gtest/src/gtest-all.cc)
add_library(gtest_main STATIC gtest/src/gtest_main.cc)
target_include_directories(gtest
  PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/gtest"
    "${CMAKE_CURRENT_SOURCE_DIR}/gtest/include"
)
target_link_libraries(gtest_main gtest)
if(NOT WIN32)
  target_link_libraries(gtest pthread)
endif()
set_target_properties(
  gtest PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

#===============================================================================
# This function uses following global properties:
# - CHIMERA_UNITTESTS
# - CHIMERA_CPP_TESTS
#
# Usage:
#   chimera_add_test(test_UnitTestA) # assumed source is test_UnitTestA.cpp
#   chimera_add_test(test_UnitTestB test_SourceB1.cpp)
#   chimera_add_test(test_UnitTestA test_SourceC1.cpp test_SourceC2.cpp)
#===============================================================================
function(chimera_add_test target_name) # ARGN for source files

  set_property(GLOBAL APPEND PROPERTY CHIMERA_CPP_TESTS ${target_name})

  if(${ARGC} GREATER 2)
    set(sources ${ARGN})
  else()
    set(sources "${target_name}.cpp")
  endif()

  add_executable(${target_name} ${sources})
  add_test(${target_name} ${target_name})

  if(MSVC)
    target_link_libraries(${target_name}
        libchimera
        emulator
        optimized gtest debug gtestd
        optimized gtest_main debug gtest_maind
    )
  else()
    target_link_libraries(${target_name} libchimera emulator gtest gtest_main)
  endif()

  clang_format_add_sources(${sources})

endfunction()

#===============================================================================
# Add tests
#===============================================================================
chimera_add_test(test_empty)
chimera_add_test(test_emulator)

# Add custom target to build all the tests as a single target
get_property(chimera_cpp_tests GLOBAL PROPERTY CHIMERA_CPP_TESTS)
add_custom_target(tests DEPENDS ${chimera_cpp_tests})

#===============================================================================
# Add binding tests
#===============================================================================
add_subdirectory(examples)
