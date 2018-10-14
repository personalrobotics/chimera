#!/usr/bin/env bash
set -ex

$SUDO apt-get -q update

if [ $(lsb_release -sc) = "trusty" ]; then
  wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | $SUDO apt-key add -
  $SUDO add-apt-repository -y "deb http://llvm.org/apt/$(lsb_release -sc)/ llvm-toolchain-$(lsb_release -sc)-${LLVM_VERSION} main"
  $SUDO apt-get -q update
fi
$SUDO apt-get -y install "llvm-${LLVM_VERSION}-dev" "llvm-${LLVM_VERSION}-tools" "libclang-${LLVM_VERSION}-dev" libedit-dev libyaml-cpp-dev libboost-dev

# Install test dependencies.
$SUDO apt-get -y install libboost-python-dev libboost-thread-dev
$SUDO apt-get -y install lcov
$SUDO apt-get -y install python-dev python3-dev
$SUDO apt-get -y install libpython-dev libpython3-dev

# Install pybind11 from source (we need pybind11 (>=2.2.0))
git clone https://github.com/pybind/pybind11.git
cd pybind11
git checkout tags/v2.2.3
mkdir build
cd build
cmake .. -DPYBIND11_TEST=OFF -DPYBIND11_PYTHON_VERSION=$PYTHON_VERSION
make -j4
$SUDO make install
cd ../..
