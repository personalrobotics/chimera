if [ "${LLVM_VERSION}" = 'latest' ]; then
  brew install llvm
  export LLVM_VERSION=$(brew list --versions | sed -n 's/llvm \([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)/\1.\2/p')
else
  brew install "llvm@${LLVM_VERSION}"
fi

brew install boost
brew install yaml-cpp --with-static-lib
