# Delta Programming Language

[![Build Status](https://travis-ci.org/delta-lang/delta.svg?branch=master)](https://travis-ci.org/delta-lang/delta)
[![Coverage Status](https://coveralls.io/repos/github/delta-lang/delta/badge.svg?branch=master)](https://coveralls.io/github/delta-lang/delta?branch=master)

Delta is a general-purpose programming language intended as an alternative to
C++, C, and Rust. The project is currently in very early stages of development.
See the (incomplete) [specification document](doc/spec.md) for a detailed
description of the language.

You can try out the language online at the [Delta Sandbox](https://delta-lang.github.io/delta-sandbox).

## Building from source

Building Delta from source requires the following dependencies:
a C++11 compiler, [CMake](https://cmake.org), [Boost](http://www.boost.org),
and the [LLVM](http://llvm.org) and [Clang](http://clang.llvm.org) libraries.
To run the test suite, you also need [lit](http://llvm.org/docs/CommandGuide/lit.html),
which you can get with `pip install lit`.

To get started, run the following commands:

    git clone https://github.com/delta-lang/delta.git
    cd delta
    mkdir build
    cd build
    cmake -G "Unix Makefiles" ..

After this, the following commands can be used:

- `make` builds the project.
- `make check` runs the test suite and reports errors in case of failure.
- `make coverage` generates a test coverage report under `coverage/`.

## Documentation

Auto-generated API documentation for the compiler is available at https://delta-lang.github.io/delta/doc.

To generate HTML for the API documentation locally, run `doxygen doc/doxyfile`
in the project root directory. The output is generated under `doc/html` and can be
viewed by opening `doc/html/index.html` in a browser.

## Usage

The basic usage of the Delta compiler is as follows:

- `delta <files>` compiles and links the given Delta source files into an executable.
- `delta -c <files>` compiles the given Delta source files into an object (.o) file.

Run `delta -help` for more options.

## Contributing

Delta is an open-source language. Contributions are welcome and encouraged! See the
current [projects](https://github.com/delta-lang/delta/projects) for things that
need to be done.

## License

This implementation of Delta is licensed under the MIT license, a permissive
free software license. See the file [LICENSE.txt](LICENSE.txt) for the full
license text.
