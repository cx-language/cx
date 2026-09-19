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
It compiles to efficient native code via LLVM, with no garbage collection or hidden runtime.

The project is still in early stages of development: some planned features have not yet been implemented,
existing ones are subject to change, and the documentation is scarce.

To learn more about the language, visit the website:

> [emillaine.github.io/cx](https://emillaine.github.io/cx/)

For more code samples, see [`examples`](examples).

You can also read the language documentation in Markdown format under [`docs/book`](docs/book).

To ask a question or open a discussion, create an issue or join the [C* Discord server](https://discord.gg/hsDbW9p).

## Building from source

Compiling C* requires a C++20 compiler, [CMake](https://cmake.org) 3.16 or newer,
and [LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 23.

### Unix / macOS

Download and extract LLVM/Clang 23 pre-built binaries for your platform from
https://github.com/llvm/llvm-project/releases, or using a package manager, e.g. `brew install llvm@23`.
Then run the following commands:

```sh
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="/path/to/llvm"
cmake --build .
```

### Windows with Visual Studio

Download and extract LLVM/Clang 23 pre-built binaries from https://github.com/llvm/llvm-project/releases.
Then run the following commands:

```sh
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\llvm" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64
cmake --build .
```

The `cx` and `cx-lsp` binaries end up in the `build` directory.

### Editor support

For diagnostics, hover, go to definition, completions, find references, and semantic highlighting
in any LSP-capable editor, see the [language server setup guide](docs/lsp.md).
The `cx-lsp` binary is built alongside `cx`.

## Contributing

Contributions are welcome.
See the GitHub issues and the [Trello board](https://trello.com/b/NIbkM4v2/c) for the project backlog.

### Running tests

Install [lit](https://llvm.org/docs/CommandGuide/lit.html) and its optional dependencies via
[pip](https://pip.pypa.io/en/stable/installing/):

```sh
python3 -m pip install lit psutil
```

Then invoke the following from the `build` directory:

```sh
cmake --build . --target check
```

This runs the full suite via CTest: the lit/compiler tests, the example programs (LLVM and C backends),
the documentation snippets, and the language server end-to-end tests.

The C++ code style is enforced by ClangFormat and ClangTidy:

- Run the `format` CMake target to format the code.
- Run the `lint` CMake target to check if the code is formatted.

### Website and playground

To work on the browser playground or the website, see [wasm/README.md](wasm/README.md)
(architecture, building, and testing of the WebAssembly frontend and the in-browser C toolchain).
Run `docs/build-website.sh --serve` to develop the website locally.

## License

C* is licensed under the MIT license, a permissive free software license.
See the file [LICENSE.txt](LICENSE.txt) for the full license text.
