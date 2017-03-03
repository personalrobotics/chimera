sudo add-apt-repository 'deb http://llvm.org/apt/trusty/ llvm-toolchain-trusty-3.6 main' -y
wget -O - http://llvm.org/apt/llvm-snapshot.gpg.key | sudo apt-key add -
sudo apt-get update -q
sudo apt-get install llvm-3.6-dev llvm-3.6-tools libclang-3.6-dev libedit-dev libyaml-cpp-dev -y
