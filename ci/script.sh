#!/usr/bin/env bash
set -x
# Disable e option for now

mkdir build
cd build
cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=${CODECOV}" ..
make -j4
if [ $CODECOV = ON ]; then make chimera_coverage; else make test; fi
if [ $CODECOV = ON ]; then bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports."; fi
