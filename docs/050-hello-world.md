# Getting started

## Installing cx

Compiling cx requires a C++20 compiler, [CMake](https://cmake.org) 3.16 or newer,
and [LLVM](https://llvm.org)/[Clang](https://clang.llvm.org) 23.

On Ubuntu/Debian, install the prerequisites with:

```sh
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 23 all
sudo apt-get install -y cmake g++ python3-pip
```

On other distributions, install CMake and a C++ compiler with your package manager,
then download and extract the pre-built LLVM/Clang 23 binaries.

On macOS, install the prerequisites with [Homebrew](https://brew.sh):

```sh
brew install llvm@23 cmake
```

On Windows, download and extract the LLVM/Clang 23 pre-built binaries from
https://github.com/llvm/llvm-project/releases.

Then build the compiler. On Linux, set `<llvm dir>` to `/usr/lib/llvm-23`
for apt installs or to the extracted directory for tarball installs:

```sh
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="<llvm dir>"
cmake --build .
```

On macOS, point CMake at the Homebrew LLVM instead:

```sh
mkdir build && cd build
cmake .. -DCMAKE_PREFIX_PATH="$(brew --prefix llvm@23)"
cmake --build .
```

The `cx` binary ends up directly in the build directory. The rest of this page
assumes it is on your `PATH`.

## Hello world

The following program outputs "Hello world" and exits:

```cs
void main() {
    println("Hello world");
}
```

Like C-based languages, the program starts execution in the `main` function.
In this program we don't need to return anything from main, so the return type is declared as `void`.
If we wanted to specify an exit status, we could change the return type to `int` and add a return statement.

To compile and run the code from the command line, we can put the code in a file called for example `hello.cx`, and then do:

```sh
cx hello.cx
./hello # or hello.exe on Windows
```

Or we can let the cx compiler do both steps using the `run` command:

```sh
cx run hello.cx
```

If we use the `run` command, the executable file `hello` will not be created.
To pass command-line arguments to the program, put them after a `--` separator:

```sh
cx run hello.cx -- arg1 arg2
```

## Command-line arguments

To read command-line arguments, declare `main` with an `args` parameter:

```cs
int main(string[] args) {
    println(args.size());
    for arg in args {
        println(arg);
    }
    return 0;
}
```

`args` holds the arguments as strings. Like in C, the first argument is the
program name (or path), so `args` is never empty when arguments are passed normally.
The parameter may also be omitted entirely, and `main` may return `void` or `int` for the exit status.

## Syntax

The syntax of cx is mostly similar to the C family of languages,
but there are some minor differences intended to make the language slightly easier to read and write:

- Semicolons are optional.
- Parentheses around conditions of control flow statements (if, for, while, switch) are optional.

### Comments

cx has two kinds of comments:

- Line comments, starting with `//` and extending until the end of the line.
- Block comments, enclosed within `/*` and `*/`, with the ability to be nested.

## Where to go next

- [Language overview](./language-overview) for what the language offers and why.
- [Variables and constants](./variables-and-constants), [Control flow](./control-flow)
  and [Strings](./strings) to learn the basics.
- The [examples](https://github.com/emillaine/cx/tree/main/examples) directory
  for complete runnable programs.
