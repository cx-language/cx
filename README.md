# Delta Programming Language

[![Build Status](https://travis-ci.org/delta-lang/delta.svg?branch=master)](https://travis-ci.org/delta-lang/delta)
[![Coverage Status](https://coveralls.io/repos/github/delta-lang/delta/badge.svg?branch=master)](https://coveralls.io/github/delta-lang/delta?branch=master)

Delta is a general-purpose systems programming language intended as an
alternative to C++, C, and Rust. Its primary goal is to improve programmer
productivity from what C++ offers, to make writing fast code as enjoyable as
possible.

The project is currently in very early stages of development: many language
features are still missing, and existing ones are subject to change at any time.

For an introduction to the language, check out the [Delta book](https://delta-lang.gitbooks.io/delta-book/content/),
the [design document](docs/design.md), and the [language specification](docs/spec/spec.pdf).
Note that all of these documents are still incomplete.

You can try out the language online at the [Delta Sandbox](https://delta-lang.github.io/sandbox) (currently not working).
Also check out the [standard library API reference](https://delta-lang.github.io/stdlib).

## Building from source

Building Delta from source requires the following dependencies: a C++17
compiler, [CMake](https://cmake.org), [libedit](http://thrysoee.dk/editline/) (only on Linux),
and the [LLVM](http://llvm.org) and [Clang](http://clang.llvm.org) libraries
(version 7.0). To run the test suite, you also need
[lit](http://llvm.org/docs/CommandGuide/lit.html), which you can get with `pip
install lit`.

If you're on Ubuntu or macOS you can run the provided `setup-build.sh` script to
automatically download all the dependencies and invoke the appropriate commands
to generate a ready-to-use build system.

If you're on Windows, do the following:

1. Download and extract LLVM sources from http://releases.llvm.org/7.0.1/llvm-7.0.1.src.tar.xz.
2. Download and extract Clang sources from http://releases.llvm.org/7.0.1/cfe-7.0.1.src.tar.xz.
3. Rename the extracted Clang source directory to `clang`, and move it inside
   the `tools` subdirectory inside the LLVM source directory.
4. During this step, LLVM and Clang will be built from source, which requires
   about 24 GB of free disk space, and will take some time to complete. Run
   `setup-build.sh`, passing the path to your LLVM source directory as an
   argument.
5. For running the tests, install [lit](http://llvm.org/docs/CommandGuide/lit.html)
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

The C++ code style is enforced by `scripts/format.sh`, which requires
clang-format version 7.0.0. The script can also be run with the `fmt` CMake
target. The CMake build system installs a git pre-commit hook to check that the
code is formatted. You can also manually check if the code is formatted by
running `scripts/format.sh --check`.

## License

This implementation of Delta is licensed under the MIT license, a permissive
free software license. See the file [LICENSE.txt](LICENSE.txt) for the full
license text.
