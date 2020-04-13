#!/usr/bin/env bash
set -ex

# Install all the depenencies ubing Brewfile except llvm
brew update > /dev/null
brew bundle

# Compare two semantic version numbers A and B. Return code is 0 if A == B, 1
# if A > B, and 2 if A < B.
# Usage: compare_versions 3.9.0 4.0.0
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
# Usage: check_version 3.9.0 '<' 4.0.0
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

function get_homebrew_version () {
  brew info --json=v1 "$1" |\
    python -c 'import json, sys;print json.load(sys.stdin)[0]["versions"]["stable"]'
}

if [ -z "${LLVM_VERSION}" ]; then
  echo "error: LLVM_VERSION is not defined" 1>&2
  exit 1
fi

# Make sure homebrew has been initialized before trying to query it.
brew update

# Homebrew packages the latest version of LLVM as a package named "llvm". If
# LLVM_VERSION matches that version, then install that package. Otherwise,
# install a "llvm@MAJOR.MINOR" version package.
LLVM_PACKAGE_LATEST='llvm'
LLVM_VERSION_LATEST=$(get_homebrew_version "${LLVM_PACKAGE_LATEST}")
if check_version "${LLVM_VERSION}" '=' "${LLVM_VERSION_LATEST}"; then
  LLVM_PACKAGE="${LLVM_PACKAGE_LATEST}"
  LLVM_INSTALL_PREFIX='llvm'
else
  LLVM_PACKAGE="llvm@${LLVM_VERSION}"
  if check_version "${LLVM_VERSION}" '>=' "3.9"; then
    LLVM_INSTALL_PREFIX="llvm"
  else
    LLVM_INSTALL_PREFIX="llvm-${LLVM_VERSION}"
  fi

  if ! brew info "${LLVM_PACKAGE}" > /dev/null 2> /dev/null; then
    echo "error: There is no package Homebrew package named '${LLVM_PACKAGE}'"\
         "for LLVM version ${LLVM_VERSION}" 1>&2
    exit 1
  fi
fi

# Infer the correct value of CMAKE_PREFIX_PATH for this version of LLVM. This
# directory was changed between LLVM versions 3.8 and 3.9.
if check_version "${LLVM_VERSION}" '>=' '3.9'; then
  LLVM_DIR="$(brew --prefix ${LLVM_PACKAGE})/lib/cmake/${LLVM_INSTALL_PREFIX}"
else
  LLVM_DIR="$(brew --prefix ${LLVM_PACKAGE})/lib/${LLVM_INSTALL_PREFIX}/share/llvm/cmake"
fi
export LLVM_DIR

echo $LLVM_DIR

brew install "${LLVM_PACKAGE}"
