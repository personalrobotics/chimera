# Compile a small subset of cling::utils.
add_library(cling_utils STATIC
  src/cling_utils_AST.cpp
)

target_compile_options(cling_utils
  PUBLIC "--std=c++11"
)

target_link_libraries(cling_utils
  ${CLANG_LIBS}
  ${llvm_libs}
)

target_include_directories(cling_utils
  PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
