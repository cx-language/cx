# Delta Programming Language

[![Build Status](https://travis-ci.org/delta-lang/delta.svg?branch=master)](https://travis-ci.org/delta-lang/delta)

Delta is a general-purpose systems programming language intended as an
alternative to languages such as C, C++, and Rust. Its primary goals are
programmer productivity and performance, which are accomplished by the
following:

- Simple language with a carefully selected set of high-level and low-level features
- Clean, familiar C-like syntax
- Fast compile times
- Ability to import C headers and use existing C libraries seamlessly
- Strong type system with type inference, tagged unions, and compile-time null-safety
- Efficient compiled code leveraging existing [LLVM](https://llvm.org) optimizations

For an introduction to the language, check out the [Delta book](https://delta-lang.gitbooks.io/delta-book/content/),
the [design document](docs/design.md), and the [language specification](docs/spec/spec.pdf).

Note that the project is still in very early stages of development, so the above
documents may be incomplete and partly out of date. Also some planned language
features are still unimplemented, and existing ones are subject to change at any time.

## Building from source

Compiling Delta requires a C++17 compiler, [CMake](https://cmake.org), and
[LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 7. To run the test
suite you also need [lit](https://llvm.org/docs/CommandGuide/lit.html).

### Ubuntu / macOS / WSL

Download and extract LLVM/Clang 7 pre-built binaries for your platform from
https://releases.llvm.org/download.html. Then run the following commands:

    mkdir build
    cd build
    cmake .. -DCMAKE_PREFIX_PATH="/path/to/llvm"

### Windows with Visual Studio

Download and extract LLVM/Clang pre-built binaries from
https://github.com/vovkos/llvm-package-windows/releases/download/llvm-7.1.0/llvm-7.1.0-windows-amd64-msvc15-msvcrt.7z
and https://github.com/vovkos/llvm-package-windows/releases/download/clang-7.1.0/clang-7.1.0-windows-amd64-msvc15-msvcrt.7z.
Then run the following commands:

    mkdir build
    cd build
    cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\llvm;C:\path\to\clang" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64

For running the tests, install [lit](https://llvm.org/docs/CommandGuide/lit.html)
with `sudo pip install lit` (you need [pip](https://pip.pypa.io/en/stable/) installed).

After this, the following commands can be invoked from the `build` directory:

- `cmake --build .` builds the project.
- `cmake --build . --target check` runs the test suite.
- `cmake --build . --target coverage` generates a test coverage report under `coverage/`.

## Documentation

Run `delta -help` for a list of supported command-line options.

To generate compiler API documentation in HTML format, run `doxygen docs/doxyfile`
in the project root directory. The output is generated under `docs/html` and can be
viewed by opening `docs/html/index.html` in a browser.

## Contributing

Delta is an open-source language, so contributions are welcome and encouraged!
See the [issues](https://github.com/delta-lang/delta/issues) and
[projects](https://github.com/delta-lang/delta/projects) for things to do.
All contributors are expected to follow the [code of conduct](docs/CODE_OF_CONDUCT.md).
If you have any questions or anything else you want to discuss, feel free to
post an issue or join the [Delta Telegram group](https://t.me/deltalang).
You can also email me at [laine.emil@gmail.com](mailto:laine.emil@gmail.com).

The C++ code style is enforced by ClangFormat and ClangTidy:

- Run the `format` CMake target to format the code.
- Run the `lint` CMake target to check if the code is formatted.

The build system also installs a git pre-commit hook that runs `lint`.

## License

This implementation of Delta is licensed under the MIT license, a permissive
free software license. See the file [LICENSE.txt](LICENSE.txt) for the full
license text.
