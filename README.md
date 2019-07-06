# Delta Programming Language

[![Build Status](https://travis-ci.org/delta-lang/delta.svg?branch=master)](https://travis-ci.org/delta-lang/delta)

Delta is a general-purpose systems programming language intended as an
alternative to C++, C, and Rust. Its primary goal is to improve programmer
productivity from what C++ offers, to make writing fast code as enjoyable as
possible.

The project is currently in very early stages of development: many language
features are still missing, and existing ones are subject to change at any time.

For an introduction to the language, check out the [Delta book](https://delta-lang.gitbooks.io/delta-book/content/),
the [design document](docs/design.md), and the [language specification](docs/spec/spec.pdf).
Note that all of these documents are still incomplete.

## Building from source

Building Delta from source requires the following dependencies: a C++17
compiler, [CMake](https://cmake.org), [libedit](http://thrysoee.dk/editline/) (only on Linux),
and the [LLVM](http://llvm.org) and [Clang](http://clang.llvm.org) libraries
(version 7.1). To run the test suite, you also need
[lit](http://llvm.org/docs/CommandGuide/lit.html), which you can get with `pip
install lit`.

### Ubuntu/macOS

Run the provided `setup-build.sh` script to automatically download all the dependencies
and invoke the appropriate commands to generate a ready-to-use build system.

### Windows with Visual Studio 15

1. Download and extract LLVM sources from https://github.com/vovkos/llvm-package-windows/releases/download/llvm-7.1.0/llvm-7.1.0-windows-amd64-msvc15-msvcrt-dbg.7z.
2. Download and extract Clang sources from https://github.com/vovkos/llvm-package-windows/releases/download/clang-7.1.0/clang-7.1.0-windows-amd64-msvc15-msvcrt-dbg.7z.
3. Run `setup-build.sh`, passing the paths to the LLVM and Clang source directories as arguments.
4. For running the tests, install [lit](http://llvm.org/docs/CommandGuide/lit.html)
   by running `sudo pip install lit` (you need [pip](https://pip.pypa.io/en/stable/)
   installed).

Otherwise, install the dependencies manually and do the following in the root
directory of the repository:

    mkdir build
    cd build
    cmake -G "Unix Makefiles" ..

After this, the following commands can be invoked from the `build` directory:

- `make` builds the project (or `make -j` to run the build in parallel).
- `make check` runs the test suite and reports errors in case of failure. Note:
  if your checks fail because of not finding C standard library headers, you can
  tell Delta where to find them with the `C_INCLUDE_PATH` or `CPATH` environment
  variable: e.g. `export C_INCLUDE_PATH=/usr/include/x86_64-linux-gnu/`.
- `make coverage` generates a test coverage report under `coverage/`.

## Documentation

To learn how to compile Delta programs, see the [compiler manual](docs/compiler-manual.md).

To generate HTML for the API documentation locally, run `doxygen docs/doxyfile`
in the project root directory. The output is generated under `docs/html` and can be
viewed by opening `docs/html/index.html` in a browser.

## Contributing

Delta is an open-source language. Contributions are welcome and encouraged! See the
current [projects](https://github.com/delta-lang/delta/projects) for things that
need to be done. All contributors are expected to follow the Delta
[code of conduct](docs/CODE_OF_CONDUCT.md). If you have any questions or just want
to chat, join our Telegram group: [t.me/deltalang](https://t.me/deltalang).

The C++ code style is enforced by ClangFormat and ClangTidy:

- Run the `format` CMake target to format the code.
- Run the `lint` CMake target to check if the code is formatted.

The build system also installs a git pre-commit hook that runs `lint`.

## License

This implementation of Delta is licensed under the MIT license, a permissive
free software license. See the file [LICENSE.txt](LICENSE.txt) for the full
license text.
