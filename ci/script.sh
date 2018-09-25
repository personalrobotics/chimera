#!/usr/bin/env bash
set -x
# Disable e option for now; this always fails for some unknown reasons

mkdir build
cd build
cmake "-DCMAKE_BUILD_TYPE=${BUILD_TYPE}" "-DLLVM_DIR=${LLVM_DIR}" "-DCODECOV=${CODECOV}" ..

# Disable building using multi-core until #212 is resolved
# make -j4 
make

if [ $CODECOV = ON ]; then make chimera_coverage; else make test; fi
if [ $CODECOV = ON ]; then bash <(curl -s https://codecov.io/bash) || echo "Codecov did not collect coverage reports."; fi
