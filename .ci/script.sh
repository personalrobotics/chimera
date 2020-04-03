#!/usr/bin/env bash
set -ex

if [ "${OS_NAME}" = "linux" ]; then
  if [ "${CI}" = "TRAVIS" ]; then
    export LLVM_DIR="/usr/share/llvm-${LLVM_VERSION}/cmake/"
  else
    export LLVM_DIR="/usr/lib/llvm-${LLVM_VERSION}/lib/cmake/llvm/"
  fi
fi

mkdir build
cd build

if [ $BUILD_NAME = TRUSTY_GCC_DEBUG ]; then
  cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=ON" ..
else
  cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=OFF" ..
fi

if [ "$OS_NAME" = "linux" ] && [ $(lsb_release -sc) = "bionic" ]; then
  make check-format
fi

make -j4

# Building tests and binding_tests fails on macOS (see: https://github.com/personalrobotics/chimera/issues/218)
if [ "${OS_NAME}" = "linux" ]; then
  # Building binding_tests fails on Ubuntu Focal
  if [ ! $(lsb_release -sc) = "forcal" ]; then
    make -j4 tests binding_tests

    if [ $BUILD_NAME = TRUSTY_GCC_DEBUG ]; then
      make chimera_coverage
    else
      ctest --output-on-failure
    fi
  fi
fi

$SUDO make install
