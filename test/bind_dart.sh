#!/bin/bash
DART_INCLUDE="${HOME}/storage/dart-ws/devel/include"
ARGS="-DBOOST_TEST_DYN_LINK -Wall -msse2 -fPIC -std=c++11 -O3 -DNDEBUG -I${DART_INCLUDE} -isystem /usr/include/eigen3 -isystem /opt/ros/indigo/include -isystem /usr/include/coin"

mkdir -p /tmp/dartpy
../build/chimera -c ./test.yaml -o /tmp/dartpy "${DART_INCLUDE}/dart/dart.h" -- ${ARGS}
