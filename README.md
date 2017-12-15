# Delta Programming Language

[![Build Status](https://travis-ci.org/delta-lang/delta.svg?branch=master)](https://travis-ci.org/delta-lang/delta)
[![Coverage Status](https://coveralls.io/repos/github/delta-lang/delta/badge.svg?branch=master)](https://coveralls.io/github/delta-lang/delta?branch=master)

Delta is a general-purpose programming language intended as an alternative to
C++, C, and Rust. The project is currently in very early stages of development.
See the (incomplete) [design](docs/design.md) and [specification](docs/spec.md)
documents for a description of the language and its purpose, or the more
learning-oriented (and equally incomplete) [Delta Book](https://delta-lang.gitbooks.io/delta-book/content/).

You can try out the language online at the [Delta Sandbox](https://delta-lang.github.io/delta-sandbox). Also check out the [standard library API reference](https://delta-lang.surge.sh).

## Building from source

Clone the repository including its submodules:

    git clone --recursive https://github.com/delta-lang/delta.git

Building Delta from source requires the following dependencies: a C++11
compiler, [CMake](https://cmake.org), [libedit](http://thrysoee.dk/editline/),
and the [LLVM](http://llvm.org) and [Clang](http://clang.llvm.org) libraries
(version 5.0.0). To run the test suite, you also need
[lit](http://llvm.org/docs/CommandGuide/lit.html), which you can get with `pip
install lit`.

I recommend using [Ninja](https://ninja-build.org/) instead of Make, because it
has cleaner output, and runs builds in parallel by default. It can be installed
with `sudo apt install ninja-build` (Ubuntu) or `brew install ninja` (macOS).

If you're on Ubuntu or macOS you can run the provided `setup-build.sh` script to
automatically download all the dependencies and invoke the appropriate commands
to generate a ready-to-use build system.

Otherwise, install the dependencies manually and do the following in the root
directory of the repository:

    mkdir build
    cd build
    cmake -G Ninja ..

After this, the following commands can be invoked from the `build` directory:

- `ninja` builds the project.
- `ninja check` runs the test suite and reports errors in case of failure. Note:
  if your checks fail because of not finding C standard library headers, you can
  tell Delta where to find them with the `C_INCLUDE_PATH` or `CPATH` environment
  variable: e.g. `export C_INCLUDE_PATH=/usr/include/x86_64-linux-gnu/`.
- `ninja coverage` generates a test coverage report under `coverage/`.

## Documentation

To learn how to compile Delta programs, see the [compiler manual](docs/compiler-manual.md).

Auto-generated API documentation for the compiler is available at https://delta-lang.github.io/delta/doc.

To generate HTML for the API documentation locally, run `doxygen docs/doxyfile`
in the project root directory. The output is generated under `docs/html` and can be
viewed by opening `docs/html/index.html` in a browser.

## Contributing

Delta is an open-source language. Contributions are welcome and encouraged! See the
current [projects](https://github.com/delta-lang/delta/projects) for things that
need to be done. All contributors are expected to follow the Delta
[code of conduct](docs/CODE_OF_CONDUCT.md). If you have any questions or just want
to chat, join our Telegram group: [t.me/deltalang](https://t.me/deltalang).

## License

This implementation of Delta is licensed under the MIT license, a permissive
free software license. See the file [LICENSE.txt](LICENSE.txt) for the full
license text.
