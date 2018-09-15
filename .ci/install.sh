#!/usr/bin/env bash
set -ex

if [ "${TRAVIS_OS_NAME}" = "linux" ]; then . install_linux.sh; fi
if [ "${TRAVIS_OS_NAME}" = "osx"   ]; then . install_macos.sh; fi
