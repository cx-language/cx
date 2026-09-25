# The cx Programming Language

cx (stylized as lowercase, not to be confused with languages called CX or Cx) is a C-family hybrid low-level/high-level programming language focused on runtime
performance and developer productivity (in this order of priority). The language is simple and unopinionated,
supporting imperative, generic, data-oriented, functional, and object-oriented programming.
It compiles to efficient native code via LLVM or the C backend, with no garbage collection or hidden runtime.


> [!NOTE]
> The project is nearing its 0.1 release: there are still some planned features that haven't been implemented yet,
and the currently implemented features are subject to change, and there are bugs.

To learn more about the language, visit the website:

> [cx-language.github.io/cx](https://cx-language.github.io/cx/)

For more code samples, see [`examples`](examples).

You can also read the language documentation in Markdown format under [`docs`](docs).

To ask a question or open a discussion, create an issue or join the [cx Discord server](https://discord.gg/hsDbW9p).

## Building from source

Compiling cx requires a C++20 compiler, [CMake](https://cmake.org) 3.16 or newer,
and [LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 23.

### Linux

On Ubuntu/Debian, install the prerequisites with:

```sh
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 23 all
sudo apt-get install -y cmake g++ python3-pip
```

On other distributions, install CMake and a C++ compiler with your package manager,
then download and extract the pre-built LLVM/Clang 23 binaries (replace `X64` with `ARM64` on ARM):

```sh
curl -L -O https://github.com/llvm/llvm-project/releases/download/llvmorg-23.1.1/LLVM-23.1.1-Linux-X64.tar.xz
tar -xf LLVM-23.1.1-Linux-X64.tar.xz
```

Then run the following commands, with `<llvm dir>` set to `/usr/lib/llvm-23`
for apt installs or the extracted directory for tarball installs:

```sh
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="<llvm dir>"
cmake --build .
```

### macOS

```sh
brew install llvm@23 cmake
```

Then run the following commands:

```sh
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix llvm@23)"
cmake --build .
```

### Windows

Download and extract LLVM/Clang 23 pre-built binaries from https://github.com/llvm/llvm-project/releases.
Then run the following commands:

```sh
mkdir build
cd build
cmake .. -DCMAKE_PREFIX_PATH="C:\path\to\llvm" -DCMAKE_GENERATOR_PLATFORM=x64 -Thost=x64
cmake --build .
```

The `cx` and `cx-lsp` binaries end up in the `build` directory.
For the fastest clean build of just the compiler, configure with
`-G Ninja -DCMAKE_BUILD_TYPE=Debug -DCMAKE_UNITY_BUILD=ON` and run
`cmake --build . --target cx`.

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

Some examples have extra prerequisites; see the README in each example directory for details.

Then invoke the following from the `build` directory:

```sh
cmake --build . --target check
```

This runs the full suite via CTest: the lit/compiler tests, the example programs (LLVM and C backends),
the documentation snippets, and the language server end-to-end tests.
Run a subset from the build directory with e.g. `ctest -R 'docs-snippets'`.
To run a single compiler test file, invoke `lit` with the flags shown by `ctest -N -V`.

The C++ code style is enforced by ClangFormat:

- Run the `format` CMake target to format the code.
- Run the `lint` CMake target to check if the code is formatted.

### Website and playground

To work on the browser playground or the website, see [wasm/README.md](wasm/README.md)
(architecture, building, and testing of the WebAssembly frontend and the in-browser C toolchain).
Run `website/build-website.sh --serve` to develop the website locally.

## License

cx is licensed under the MIT license, a permissive free software license.
See the file [LICENSE.txt](LICENSE.txt) for the full license text.
