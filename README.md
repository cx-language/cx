> [!NOTE]
> This project was started in January 2017 after I had been annoyed with C++ for a long time, and had not found any good
> alternatives for my use cases.
> The current state of the project is that it's functional, but far from truly production-ready. I almost got to the
> point of releasing the first pre-release version and announcing the project publicly.
> However, nowadays there are several languages with sufficiently similar design goals, such
> as [Odin](https://odin-lang.org/), [Jai](https://en.wikipedia.org/?title=JAI_(programming_language)&redirect=no),
> and [Zig](https://ziglang.org/). Therefore, this project is no longer being developed.

# The C* Programming Language

C* (pronounced "C star") is a C-based hybrid low-level/high-level programming language focused on runtime
performance and developer productivity (in this order of priority). The language is simple and unopinionated,
supporting imperative, generic, data-oriented, functional, and object-oriented programming.

Learn more about the language at https://emillaine.github.io/cx-language.github.io/.  
See code samples under [`examples`](https://github.com/emillaine/cx/tree/main/examples).

The project is still in early stages of development: some planned features have not yet been implemented,
existing ones are subject to change, and the documentation is scarce.

## Contributing

Contributions are welcome.
See the GitHub issues and the [Trello board](https://trello.com/b/NIbkM4v2/c) for the project backlog.
To ask a question or open a discussion, create an issue or join the [C* Discord server](https://discord.gg/hsDbW9p).

## Building from source

Compiling C* requires a C++17 compiler, [CMake](https://cmake.org), and
[LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 21.0.

- ### Unix / macOS

  Download and extract LLVM/Clang 21.0 pre-built binaries for your platform from
  https://github.com/llvm/llvm-project/releases, or using a package manager, e.g. `brew install llvm@21`.
  Then run the following commands:

      mkdir build
      cd build
      cmake .. -DCMAKE_PREFIX_PATH="/path/to/llvm"

- ### Windows with Visual Studio

  Download and extract LLVM/Clang 21.0 pre-built binaries from https://github.com/llvm/llvm-project/releases.
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
