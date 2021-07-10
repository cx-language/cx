# The C* Programming Language

C* (pronounced "C star") is a C-based hybrid low-level/high-level programming language focused on runtime
performance and developer productivity (in this order of priority). The language is simple and unopinionated,
supporting imperative, generic, data-oriented, functional, and object-oriented programming.

Learn more about the language at http://cx-lang.org.

The project is still in early stages of development: some planned features have not yet been implemented,
existing ones are subject to change, and the documentation is scarce.

## Contributing

Contributions are welcome.
See the GitHub issues and the [Trello board](https://trello.com/b/NIbkM4v2/c) for the project backlog.
To ask a question or open a discussion, create an issue or join the [C* Discord server](https://discord.gg/hsDbW9p).

## Building from source

Compiling C* requires a C++17 compiler, [CMake](https://cmake.org), and
[LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 11.

- ### Unix / macOS

    Download and extract LLVM/Clang 11 pre-built binaries for your platform from
    https://releases.llvm.org/download.html, or using a package manager, e.g. `brew install llvm@11`.
    Then run the following commands:

      mkdir build
      cd build
      cmake .. -DCMAKE_PREFIX_PATH="/path/to/llvm"

- ### Windows with Visual Studio

    Download and extract LLVM/Clang 11 pre-built binaries from somewhere, such as
    https://ziglang.org/deps/llvm%2bclang%2blld-11.0.0-x86_64-windows-msvc-release-mt.tar.xz
    (consider [sponsoring Zig](https://github.com/sponsors/ziglang)).
    Then run the following commands:

      mkdir build
      cd build
      cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\llvm" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64

To run the tests, install [lit](https://llvm.org/docs/CommandGuide/lit.html)
and its optional dependencies via [pip](https://pip.pypa.io/en/stable/installing/):

    sudo python3 -m pip install lit psutil

After this, the following commands can be invoked from the `build` directory:

- `cmake --build .` builds the project.
- `cmake --build . --target check` runs the test suite.

The C++ code style is enforced by ClangFormat and ClangTidy:

- Run the `format` CMake target to format the code.
- Run the `lint` CMake target to check if the code is formatted.

## License

C* is licensed under the MIT license, a permissive free software license.
See the file [LICENSE.txt](LICENSE.txt) for the full license text.
