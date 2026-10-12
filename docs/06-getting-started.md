# Getting started

## Installing cx

The fastest install on Linux and macOS is the install script, which downloads
the prebuilt binary for your platform into `~/.cx` and smoke-tests it:

```sh
curl https://cx-language.github.io/cx/install.sh | bash
```

It adds itself to your `PATH` (bash, zsh, and fish), so new shells pick it up
automatically; in the current shell, run the `export` it prints first.
Prebuilt binaries cover Linux (x64), macOS (Apple Silicon), and Windows (x64).
The Windows archive and manual downloads for all platforms are on the
[releases page](https://github.com/cx-language/cx/releases); each archive also
contains the [`cx-lsp` language server](./editor-setup) and the standard library.
Native compilation needs a C toolchain as well: `cc` on `PATH` on Linux/macOS,
or Visual Studio Build Tools installed on Windows (`cx run` JIT-compiles and
needs nothing).

To build from source instead (required on Intel Macs and other platforms
without a prebuilt binary), see [Building from source](./building-from-source).

The rest of this page assumes `cx` is on your `PATH`.

## Hello world

The following program outputs "Hello world" and exits:

```cs
void main() {
    println("Hello world");
}
```

The program starts execution in the `main` function.
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

`args` holds the arguments as strings. The first argument is the
program name (or path).
The parameter may also be omitted entirely, and `main` may return `void` or `int` for the exit status.

## Syntax

A few purposeful choices make the syntax slightly easier to read and write:

- Semicolons are optional.
- Parentheses around conditions of control flow statements (if, for, while, switch) are optional.

### Comments

cx has two kinds of comments:

- Line comments, starting with `//` and extending until the end of the line.
- Block comments, enclosed within `/*` and `*/`, with the ability to be nested.
  Comment markers inside `"..."` strings don't affect nesting, so commenting
  out code with `/*` or `*/` in a string just works.

## Where to go next

- [Variables and constants](./variables-and-constants), [Control flow](./control-flow)
  and [Strings](./strings) to learn the basics.
- The [examples](https://github.com/cx-language/cx/tree/main/examples) directory
  for complete runnable programs.
