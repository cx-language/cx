#!/usr/bin/env bash

LLVM_VERSION=7.0.1

set -e

os=$(uname)

if [[ "$os" == "Darwin" ]]; then
    if ! type "brew" > /dev/null; then
        echo "You don't have Homebrew installed. Get it from https://brew.sh."
        exit 1
    fi
fi

set -x

root_dir="$(dirname "$0")"
build_dir="$(cd "$(dirname "$root_dir")"; pwd)/$(basename "$root_dir")/build" # Convert to absolute path
mkdir -p "$build_dir"
cd "$build_dir"

if [[ "$os" == "Darwin" ]]; then
    brew update

    for package in cmake llvm@7; do
        brew ls --versions $package > /dev/null || brew install $package
    done

    sudo easy_install pip
    sudo pip2 install lit

    cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$(brew --prefix llvm@7)"
elif [[ "$os" == "MINGW"* ]]; then
    llvm_dir="$1"
    clang_dir="$2"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$llvm_dir;$clang_dir" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64
else
    UBUNTU_VERSION=$(lsb_release -r -s)
    LLVM_TARBALL_NAME=clang+llvm-$LLVM_VERSION-x86_64-linux-gnu-ubuntu-$UBUNTU_VERSION

    sudo apt-get update

    # The libclang packages in apt don't contain ClangConfig.cmake
    # which we need so download a tarball from llvm.org instead.
    if [ ! -d "$LLVM_TARBALL_NAME" ]; then
        curl http://releases.llvm.org/$LLVM_VERSION/$LLVM_TARBALL_NAME.tar.xz | tar xJ
    fi

    # Install LLVM dependencies manually because we didn't use apt.
    sudo apt install libtinfo-dev
    sudo apt install zlib1g-dev

    sudo apt install cmake
    sudo apt install libedit-dev
    sudo apt install python-pip
    sudo pip install lit

    cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/$LLVM_TARBALL_NAME"
fi
