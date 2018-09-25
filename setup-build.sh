LLVM_VERSION=7.0.0

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

    for package in cmake llvm@6; do
        brew ls --versions $package > /dev/null || brew install $package
    done

    sudo easy_install pip
    sudo pip2 install lit

    cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$(brew --prefix llvm)"
elif [[ "$os" == "MINGW"* ]]; then
    # Build LLVM and Clang libraries from source, because the prebuilt Windows
    # binaries provided in http://releases.llvm.org/download.html don't include
    # everything we need, e.g. header files.

    llvm_dir="$1"
    llvm_build_dir="$llvm_dir/build"
    llvm_install_dir="$llvm_dir/install"

    mkdir -p "$llvm_build_dir"
    cd "$llvm_build_dir"
    cmake "$llvm_dir" -DCMAKE_INSTALL_PREFIX="$llvm_install_dir" -DCMAKE_BUILD_TYPE=Debug \
        -DLLVM_TARGETS_TO_BUILD="${LLVM_TARGETS_TO_BUILD:-X86}" -DLLVM_OPTIMIZED_TABLEGEN=ON
    cmake --build .
    cmake --build . --target install

    cd "$build_dir"
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$llvm_install_dir"
else
    LLVM_TARBALL_NAME=clang+llvm-$LLVM_VERSION-x86_64-linux-gnu-ubuntu-16.04

    sudo apt-get update

    # The libclang packages in apt don't contain ClangConfig.cmake
    # which we need so download a tarball from llvm.org instead.
    if [ ! -d "$LLVM_TARBALL_NAME" ]; then
        curl http://releases.llvm.org/$LLVM_VERSION/$LLVM_TARBALL_NAME.tar.xz | tar xJ
    fi

    # Install LLVM dependencies manually because we didn't use apt.
    sudo apt install libtinfo-dev

    sudo apt install cmake
    sudo apt install libedit-dev
    sudo apt install python-pip
    sudo pip install lit

    cmake -G "Unix Makefiles" .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/$LLVM_TARBALL_NAME"
fi
