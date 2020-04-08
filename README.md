# Delta Programming Language

Delta is a high-performance programming language designed to improve programmer productivity.
The project is still in early stages of development: some planned features have not yet been implemented,
existing ones are subject to change, and the documentation is scarce.

Learn more about the language at https://delta-lang.github.io.

## Contributing

Contributions are welcome and encouraged.
See the GitHub issues and the [Trello board](https://trello.com/b/NIbkM4v2/delta) for the project backlog.
To ask a question or open a discussion, create an issue or join the [Delta Discord server](https://discord.gg/hsDbW9p).

## Building from source

Compiling Delta requires a C++17 compiler, [CMake](https://cmake.org), and
[LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 9.0. To run the test
suite you also need [lit](https://llvm.org/docs/CommandGuide/lit.html).

### Ubuntu / macOS / WSL

Download and extract LLVM/Clang 9.0 pre-built binaries for your platform from
https://releases.llvm.org/download.html. Then run the following commands:

    mkdir build
    cd build
    cmake .. -DCMAKE_PREFIX_PATH="/path/to/llvm"

### Windows with Visual Studio

Download and extract LLVM/Clang pre-built binaries from
https://github.com/vovkos/llvm-package-windows/releases/download/llvm-master/llvm-9.0.0-windows-amd64-msvc15-msvcrt.7z
and https://github.com/vovkos/llvm-package-windows/releases/download/clang-master/clang-9.0.0-windows-amd64-msvc15-msvcrt.7z.
Then run the following commands:

    mkdir build
    cd build
    cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\llvm;C:\path\to\clang" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64

For running the tests, install [lit](https://llvm.org/docs/CommandGuide/lit.html)
with `sudo pip install lit` (you need [pip](https://pip.pypa.io/en/stable/) installed).

After this, the following commands can be invoked from the `build` directory:

- `cmake --build .` builds the project.
- `cmake --build . --target check` runs the test suite.

The C++ code style is enforced by ClangFormat and ClangTidy:

- Run the `format` CMake target to format the code.
- Run the `lint` CMake target to check if the code is formatted.

## License

This implementation of Delta is licensed under the MIT license, a permissive
free software license. See the file [LICENSE.txt](LICENSE.txt) for the full
license text.
