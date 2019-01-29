#!/usr/bin/env bash
set -ex

if [ "${OS_NAME}" = "linux" ]; then
  export LLVM_DIR="/usr/lib/llvm-${LLVM_VERSION}/lib/cmake/llvm/"
fi

mkdir build
cd build

# [DEBUG]
dpkg -L llvm-${LLVM_VERSION}-dev
dpkg -L llvm-${LLVM_VERSION}-tools
test -e ${LLVM_DIR} && echo file exists || echo file not found

if [ $BUILD_NAME = TRUSTY_GCC_DEBUG ]; then
  cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=ON" ..
else
  cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=OFF" ..
fi

if [ "$OS_NAME" = "linux" ] && [ $(lsb_release -sc) = "bionic" ]; then
  make check-format
fi

make -j4 tests binding_tests

if [ $BUILD_NAME = TRUSTY_GCC_DEBUG ]; then
  make chimera_coverage
else
  ctest --output-on-failure
fi

$SUDO make install
