# Delta Programming Language

[![Build Status](https://travis-ci.org/delta-lang/delta.svg?branch=master)](https://travis-ci.org/delta-lang/delta)
[![Coverage Status](https://coveralls.io/repos/github/delta-lang/delta/badge.svg?branch=master)](https://coveralls.io/github/delta-lang/delta?branch=master)

Delta is a general-purpose programming language intended as an alternative to
C++, C, and Rust. The project is currently in very early stages of development.
See the (incomplete) [specification document](doc/spec.md) for a detailed
description of the language, or the more learning-oriented (and equally
incomplete) [Delta Book](https://delta-lang.gitbooks.io/delta-book/content/).

You can try out the language online at the [Delta Sandbox](https://delta-lang.github.io/delta-sandbox).

## Building from source

Building Delta from source requires the following dependencies: a C++11
compiler, [CMake](https://cmake.org), [libedit](http://thrysoee.dk/editline/),
and the [LLVM](http://llvm.org) and [Clang](http://clang.llvm.org) libraries
(version 4.0.0). To run the test suite, you also need
[lit](http://llvm.org/docs/CommandGuide/lit.html), which you can get with `pip
install lit`.

I recommend using [Ninja](https://ninja-build.org/) instead of Make, because it
has cleaner output, and runs builds in parallel by default. It can be installed
with `sudo apt install ninja-build` (Ubuntu) or `brew install ninja` (macOS).

If you're on Ubuntu you can run the provided `setup-build-ubuntu.sh` script to
automatically download all the dependencies and invoke the appropriate commands
to generate a ready-to-use build system.

Otherwise, install the dependencies manually and do the following in the root
directory of the repository:

    mkdir build
    cd build
    cmake -G Ninja ..

After this, the following commands can be invoked from the `build` directory:

- `ninja` builds the project.
- `ninja check` runs the test suite and reports errors in case of failure.
- `ninja coverage` generates a test coverage report under `coverage/`.

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
