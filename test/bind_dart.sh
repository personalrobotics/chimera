#!/bin/bash
DART_INCLUDE="${HOME}/ws/devel/include"
ARGS="-DBOOST_TEST_DYN_LINK -Wall -msse2 -fPIC -std=c++11 -O3 -DNDEBUG -I${DART_INCLUDE} -isystem /usr/include/eigen3 -isystem /opt/ros/indigo/include -isystem /usr/include/coin -isystem /home/mkoval/storage/chimera-ws/devel/lib/clang/3.8.0/include/"

mkdir -p /tmp/dartpy
chimera -m dartpy -n dart::dynamics -n dart::math -o /tmp/dartpy "${DART_INCLUDE}/dart/dart.h" -- ${ARGS}
cp CMakeLists.txt /tmp/dartpy
