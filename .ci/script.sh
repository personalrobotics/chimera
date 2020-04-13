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

if [ $BUILD_NAME = XENIAL_GCC_DEBUG ]; then
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
  if [ ! $(lsb_release -sc) = "focal" ]; then
    # Parallel build for examples is disabled by
    # https://github.com/personalrobotics/chimera/pull/274
    # because it seems to cause missing intermediate target. Here are examples of the
    # build failures:
    # - https://travis-ci.org/github/personalrobotics/chimera/jobs/671315806#L2226-L2301
    # - https://travis-ci.org/github/personalrobotics/chimera/jobs/671301082#L2523-L2573
    make tests binding_tests

    if [ $BUILD_NAME = XENIAL_GCC_DEBUG ]; then
      make chimera_coverage
    else
      ctest --output-on-failure
    fi
  fi
fi

$SUDO make install
