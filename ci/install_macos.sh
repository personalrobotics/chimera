if [ "${LLVM_VERSION}" = 'latest' ]; then
  export LLVM_PACKAGE='llvm'
  export LLVM_VERSION=$(brew list --versions | sed -n 's/llvm \([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)/\1.\2/p')
else
  export LLVM_PACKAGE="llvm@${LLVM_VERSION}"
fi

brew install boost "${LLVM_PACKAGE}"
brew install yaml-cpp --with-static-lib
