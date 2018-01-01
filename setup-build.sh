set -e

os=$(uname)

if [[ "$os" == "Darwin" ]]; then
    if ! type "brew" > /dev/null; then
        echo "You don't have Homebrew installed. Get it from https://brew.sh."
        exit 1
    fi
fi

set -x

build_dir="$(dirname "$0")/build"
mkdir -p "$build_dir"
cd "$build_dir"

if [[ "$os" == "Darwin" ]]; then
    # Tested on macOS 10.13.

    brew update

    brew install cmake ninja llvm
    sudo easy_install pip
    sudo pip2 install lit

    cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$(brew --prefix llvm)"
elif [[ "$os" == "MINGW"* ]]; then
    # Tested on Windows 10 with MinGW64.

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
    # Tested on Ubuntu 17.04 and Windows Subsystem for Linux (running Ubuntu 16.04).

    sudo apt-get update

    # The libclang packages in apt don't contain ClangConfig.cmake
    # which we need so download a tarball from llvm.org instead.
    if [ ! -d clang+llvm-5.0.0-linux-x86_64-ubuntu16.04 ]; then
        curl http://releases.llvm.org/5.0.0/clang+llvm-5.0.0-linux-x86_64-ubuntu16.04.tar.xz | tar xJ
    fi

    # Install LLVM dependencies manually because we didn't use apt.
    sudo apt install libtinfo-dev

    sudo apt install cmake
    sudo apt install ninja-build
    sudo apt install libedit-dev
    sudo apt install python-pip
    sudo pip install lit

    cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/clang+llvm-5.0.0-linux-x86_64-ubuntu16.04"
fi
