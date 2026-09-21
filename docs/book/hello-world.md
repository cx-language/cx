# Hello world!

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

To read command-line arguments, declare `main` with `argc`/`argv` parameters:

```cs
void main(int argc, string[] argv) {
    println(argc);
    for arg in argv {
        println(arg);
    }
}
```

`argc` is the number of arguments and `argv` holds them as strings.
Like in C, the first argument is the program name (or path), so `argc` is at least 1 when arguments are passed normally.
The parameters may also be omitted entirely, and `main` may return `void` or `int` for the exit status.

## Syntax

The syntax of cx is mostly similar to the C family of languages, 
but there are some minor differences intended to make the language slightly easier to read and write:

- Semicolons are optional.
- Parentheses around conditions of control flow statements (if, for, while, switch) are optional.

### Comments

cx has two kinds of comments:

- Line comments, starting with `//` and extending until the end of the line.
- Block comments, enclosed within `/*` and `*/`, with the ability to be nested.
