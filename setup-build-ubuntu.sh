# Tested on Ubuntu 17.04.

set -x

cd "$(dirname "$0")"
mkdir -p build
cd build

# The libclang packages in apt don't contain ClangConfig.cmake
# which we need so download a tarball from llvm.org instead.
if [ ! -d clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.10 ]; then
    curl http://releases.llvm.org/4.0.0/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.10.tar.xz | tar xJ
fi

# Install LLVM dependencies manually because we didn't use apt.
sudo apt install libtinfo-dev

sudo apt install cmake
sudo apt install ninja-build
sudo apt install libboost-dev
sudo apt install python-pip

# For some reason lit cannot be invoked after installing it like this.
# TODO: Figure out what's going on.
pip install lit

cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$PWD/clang+llvm-4.0.0-x86_64-linux-gnu-ubuntu-16.10"
