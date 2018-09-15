#!/usr/bin/env bash
set -x
# Disable e option for now

mkdir build
cd build

if [ $BUILD_NAME = TRUSTY_GCC_DEBUG ]; then
  cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=ON" ..
else
  cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=OFF" ..
fi

make -j4

if [ $BUILD_NAME = TRUSTY_GCC_DEBUG ]; then
  make chimera_coverage
  bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports."
else
  make test
fi
