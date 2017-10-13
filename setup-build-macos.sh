# Tested on macOS 10.13.

set -e

if ! type "brew" > /dev/null; then
    echo "You don't have Homebrew installed. Get it from https://brew.sh."
    exit 1
fi

set -x

cd "$(dirname "$0")"
mkdir -p build
cd build

brew update

brew install cmake ninja llvm
sudo pip2 install lit

cmake -G Ninja .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PREFIX_PATH="$(brew --prefix llvm)"
