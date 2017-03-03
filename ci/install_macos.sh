brew install yaml-cpp --with-static-lib
PKG_CONFIG_PATH=/usr/local/Cellar/yaml-cpp/0.5.2/lib/pkgconfig cmake -DLLVM_DIR=/usr/local/opt/llvm/share/llvm/cmake ..
