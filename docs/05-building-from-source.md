# Building from source

Prebuilt Linux, macOS, and Windows binaries are published on the [releases page](https://github.com/cx-language/cx/releases)
(the fastest install on Linux/macOS is `curl https://cx-language.github.io/cx/install.sh | bash`).
To compile cx yourself instead (required on platforms without a prebuilt binary, such as Intel Macs),
you need a C++20 compiler, [CMake](https://cmake.org) 3.16 or newer,
and [LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 23.

On Linux and macOS, the `cx` and `cx-lsp` binaries end up directly in the build directory.

## Linux

On Ubuntu/Debian, install the prerequisites with:

```sh
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 23 all
sudo apt-get install -y cmake g++ python3-pip
```

On other distributions, install CMake and a C++ compiler with your package manager,
then download and extract the pre-built LLVM/Clang 23 binaries (replace `X64` with `ARM64` on ARM):

```sh
curl -L -O https://github.com/llvm/llvm-project/releases/download/llvmorg-23.1.1/LLVM-23.1.1-Linux-X64.tar.xz
tar -xf LLVM-23.1.1-Linux-X64.tar.xz
```

Then run the following commands, with `<llvm dir>` set to `/usr/lib/llvm-23`
for apt installs or the extracted directory for tarball installs:

```sh
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="<llvm dir>"
cmake --build .
```

## macOS

```sh
brew install llvm@23 cmake
```

Then run the following commands:

```sh
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix llvm@23)"
cmake --build .
```

## Windows

Download and extract LLVM/Clang 23 pre-built binaries from <https://github.com/llvm/llvm-project/releases>.

Run the commands below from an x64 Native Tools Command Prompt for
Visual Studio (or any prompt with the VS environment loaded): it puts the
Visual Studio bundled CMake, Ninja, and the MSVC compiler on `PATH`.
A cmake.org install works too, but not the MSYS2 CMake, which generates
Unix-style paths that break the build.

Before configuring, check the environment:

- Keep MSYS2 off `PATH` entirely, not just its CMake: CMake may
  otherwise pick up MSYS2's LibXml2 and leak its GCC-specific headers
  into the MSVC build (100+ errors mentioning `__asm__` or
  `__UINTPTR_TYPE__`). If the errors already happened, strip every
  MSYS2 entry from `PATH` and configure in a fresh build directory.
- Install Python 3 (the python.org installer provides the `py`
  launcher), then install the test runner with
  `py -m pip install lit psutil` (`python -m pip ...` works too; stock
  Windows provides no `python3` alias). The test suite locates the
  interpreter itself, but the `lit` executable that pip installs must
  be on `PATH`.

```sh
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\llvm" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64
cmake --build . --config Release
```

LLVM's Windows binaries require zlib and zstd at configure time. CMake uses
system/vcpkg copies when findable (add their install prefix to
`CMAKE_PREFIX_PATH`), and otherwise builds them from source automatically,
so no manual dependency install is needed.

The `cx` and `cx-lsp` binaries end up in `build\Release`
(`build\Debug` when built without `--config Release`).
For the fastest clean build of just the compiler, configure with
`-G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_UNITY_BUILD=ON` and run
`cmake --build . --target cx`.

## Offline builds

Configuring downloads zlib and zstd on machines without system copies
(e.g. Windows). To build without network access, check out the pinned
commits (see `GIT_TAG` in CMakeLists.txt) and point CMake at them:

```sh
cmake .. -DFETCHCONTENT_SOURCE_DIR_ZLIB=/path/to/zlib -DFETCHCONTENT_SOURCE_DIR_ZSTD=/path/to/zstd
```

If your LLVM distribution does not ship FileCheck, configuring downloads
its source; pass `-DFILECHECK_SOURCE=/path/to/FileCheck.cpp` to build from
a local copy of the same LLVM version instead.

## Where to go next

- [Getting started](./getting-started) to write your first program.
- [Editor setup](./editor-setup) (`cx-lsp` is built alongside `cx`).
