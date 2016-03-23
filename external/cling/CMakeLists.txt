# Compile a small subset of cling::utils.
set(cling_utils_INCLUDE_DIR
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  CACHE STRING
  "cling utils include directory"
)

include_directories(
  ${cling_utils_INCLUDE_DIR}
)

add_library(cling_utils STATIC
  src/cling_utils_AST.cpp
)

target_link_libraries(cling_utils
  ${CLANG_LIBS}
  ${llvm_libs}
)