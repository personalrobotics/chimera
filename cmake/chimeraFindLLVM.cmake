if(NOT APPLE)

  # We don't need anything special to find LLVM on Linux
  find_package(LLVM REQUIRED CONFIG)

else()

  # Find LLVM without LLVM_DIR
  find_package(LLVM QUIET)

  if(NOT LLVM_FOUND)
    # Set LLVM_DIR using Homebrew
    find_program(BREW_EXECUTABLE brew)
    if(NOT BREW_EXECUTABLE)
      message(FATAL_ERROR "Failed to find LLVM. Chimera now would like to find \
        LLVM using Homebrew, but it's not installed. Please install Homebrew \
        and install LLVM using Homebrew for building Chimera."
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
  endif()

endif()

# Check if LLVM is compatible with Chimera
set(COMPATIBLE_LLVM_VERSIONS 3.6 3.9 6.0)
set(LLVM_VERSION_MAJOR_MINOR ${LLVM_VERSION_MAJOR}.${LLVM_VERSION_MINOR})
set(FOUND_COMPATIBLE_LLVM FALSE)
foreach(version ${COMPATIBLE_LLVM_VERSIONS})
  if (${LLVM_VERSION_MAJOR_MINOR} VERSION_EQUAL ${version})
    set(FOUND_COMPATIBLE_LLVM TRUE)
    break()
  endif()
endforeach()
if(NOT FOUND_COMPATIBLE_LLVM)
  message(FATAL_ERROR "Found LLVM ${LLVM_VERSION}, but it's not compatible \
    with Chimera. Please build Chimera with one of following LLVM \
    versions: ${COMPATIBLE_LLVM_VERSIONS}"
  )
endif()
