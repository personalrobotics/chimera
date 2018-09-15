#!/usr/bin/env bash
set -ex

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then . .ci/install_linux.sh; fi
if [ "${TRAVIS_OS_NAME}" = "osx"   ]; then . .ci/install_macos.sh; fi
