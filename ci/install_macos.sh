# Compare two semantic version numbers A and B. Return code is 0 if A == B, 1
# if A > B, and 2 if A < B.
# Source: http://stackoverflow.com/a/4025065/111426
function compare_versions () {
  if [[ $1 == $2 ]]
  then
    return 0
  fi
  local IFS=.
  local i ver1=($1) ver2=($2)
  # fill empty fields in ver1 with zeros
  for ((i=${#ver1[@]}; i<${#ver2[@]}; i++))
  do
      ver1[i]=0
  done
  for ((i=0; i<${#ver1[@]}; i++))
  do
    if [[ -z ${ver2[i]} ]]
    then
      # fill empty fields in ver2 with zeros
      ver2[i]=0
    fi
    if ((10#${ver1[i]} > 10#${ver2[i]}))
    then
      return 1
    fi
    if ((10#${ver1[i]} < 10#${ver2[i]}))
    then
      return 2
    fi
  done
  return 0
}

# Check a relationship (=, >, >=, <, <=) between two semantic version numbers.
function check_version () {
  compare_versions "$1" "$3"
  result="$?"

  case "$2" in
    '=')  [ "${result}" -eq 0 ];;
    '>')  [ "${result}" -eq 1 ];;
    '>=') [ "${result}" -ne 2 ];;
    '<')  [ "${result}" -eq 2 ];;
    '<=') [ "${result}" -ne 1 ];;
    '!=') [ "${result}" -ne 0 ];;
  esac
}


# Homebrew packages the latest version of LLVM as a package named "llvm" and
# old versions as "llvm@MAJOR.MINOR".
LLVM_VERSION=${LLVM_VERSION:-latest}
if [ "${LLVM_VERSION}" = 'latest' ]; then
  LLVM_PACKAGE='llvm'
else
  LLVM_PACKAGE="llvm@${LLVM_VERSION}"
fi

# Extract the version of the specified LLVM package. This is only strictly
# necessary in the case where LLVM_VERSION=latest.
LLVM_VERSION=$(brew list --versions \
  | sed -n "s/${LLVM_PACKAGE} \([0-9]*\)\.\([0-9]*\)\.\([0-9]*\)/\1.\2/p")

# Find the CMAKE_PREFIX_PATH for the installed version of LLVM. This directory
# was changed in the LLVM 3.9 release.
if check_version "${LLVM_VERSION}" '>=' '3.9'; then
  # TODO: This may break when a more recent version (>= 3.9) of LLVM is
  # distributed in a named package. Only LLVM 3.7 and 3.8 are currently
  # available, so it is not possible to test this.
  LLVM_DIR="$(brew --prefix ${LLVM_PACKAGE})/lib/cmake/llvm"
else
  LLVM_DIR="$(brew --prefix ${LLVM_PACKAGE})/lib/llvm-${LLVM_VERSION}/share/llvm/cmake"
fi

set +ex
brew install boost
brew install "${LLVM_PACKAGE}"
brew install yaml-cpp --with-static-lib
set -ex

export LLVM_DIR LLVM_PACKAGE LLVM_VERSION
