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

add_subdirectory(examples)
