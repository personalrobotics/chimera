# We don't need anything special to find LLVM on Linux
if(NOT APPLE)
  find_package(LLVM REQUIRED CONFIG)
  return()
endif()

# Find LLVM without LLVM_DIR
find_package(LLVM QUIET)
if(LLVM_FOUND)
  return()
endif()

# Set LLVM_DIR using Homebrew
find_program(BREW_EXECUTABLE brew)
if(NOT BREW_EXECUTABLE)
  message(FATAL_ERROR "Failed to find LLVM. Chimera now would like to find \
    LLVM using Homebrew, but it's not installed. Please install Homebrew and \
    install LLVM using Homebrew for building Chimera."
  )
endif()
execute_process(COMMAND 
  ${BREW_EXECUTABLE} --prefix llvm
  OUTPUT_VARIABLE LLVM_PREFIX
)
string(REGEX REPLACE "\n$" "" LLVM_PREFIX "${LLVM_PREFIX}")
set(LLVM_DIR "${LLVM_PREFIX}/lib/cmake/llvm")

# Find LLVM with LLVM_DIR
find_package(LLVM REQUIRED CONFIG)
