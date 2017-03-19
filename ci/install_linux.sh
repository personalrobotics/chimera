export LLVM_DIR="/usr/share/llvm-${LLVM_VERSION}/cmake/"

set +ex
sudo add-apt-repository -y "deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-${LLVM_VERSION} main"
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get -q update
sudo apt-get -y install "llvm-${LLVM_VERSION}-dev" "llvm-${LLVM_VERSION}-tools" "libclang-${LLVM_VERSION}-dev" libedit-dev libyaml-cpp-dev libboost-dev
set -ex
