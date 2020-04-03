#!/usr/bin/env bash
set -ex

$SUDO apt-get -q update
$SUDO apt-get -y install \
  lsb-release

if [ $(lsb_release -sc) = "trusty" ]; then
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | $SUDO apt-key add -
  $SUDO add-apt-repository -y "deb http://llvm.org/apt/$(lsb_release -sc)/ llvm-toolchain-$(lsb_release -sc)-${LLVM_VERSION} main"
  $SUDO apt-get -q update
fi
$SUDO apt-get -y install "llvm-${LLVM_VERSION}-dev" "llvm-${LLVM_VERSION}-tools" "libclang-${LLVM_VERSION}-dev" libedit-dev libyaml-cpp-dev libboost-dev

# Install build tools
$SUDO apt-get -y install \
  build-essential \
  cmake \
  curl \
  git \
  lib32z1-dev \
  lsb-release \
  pkg-config \
  software-properties-common \
  sudo \
  wget

# Install test dependencies.
$SUDO apt-get -y install libeigen3-dev
$SUDO apt-get -y install libboost-python-dev libboost-thread-dev
$SUDO apt-get -y install lcov
# Install Python 2 up to Eoan
if [ $(lsb_release -sc) = "xenial" ] || [ $(lsb_release -sc) = "bionic" ] || [ $(lsb_release -sc) = "eoan" ]; then
  $SUDO apt-get -y install python-dev libpython-dev
fi
$SUDO apt-get -y install python3-dev libpython3-dev

# Install ClangFormat 6
if [ $(lsb_release -sc) = "bionic" ]; then
  $SUDO apt-get -y install clang-format-6.0
fi

# Install pybind11 (we need pybind11 (>=2.2.4))
if [ $(lsb_release -sc) = "xenial" ] || [ $(lsb_release -sc) = "bionic" ]; then
  git clone https://github.com/pybind/pybind11.git
  cd pybind11
  git checkout tags/v2.2.4
  mkdir build
  cd build
  cmake .. -DPYBIND11_TEST=OFF -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION
  make -j4
  $SUDO make install
  cd ../..
else
  $SUDO apt-get -y install pybind11-dev
fi
