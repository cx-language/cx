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

To compile and run the code from the command line, we can put the code in a file called for example `hello.delta`, and then do:

```sh
delta hello.delta
./hello # or hello.exe on Windows
```

Or we can let the Delta compiler do both steps using the `run` command:

```sh
delta run hello.delta
```

If we use the `run` command, the executable file `hello` will not be created.

## Syntax

The syntax of Delta is mostly similar to the C family of languages, 
but there are some stylistic differences worth noting:

- Semicolons are optional.
- Parentheses around conditions of control flow statements (if, for, while, switch) are optional.

However, to attract attention to more important differences, this documentation
uses semicolons and parentheses even if not required. 

### Comments

Delta has two kinds of comments:

- Line comments, starting with `//` and extending until the end of the line.
- Block comments, enclosed within `/*` and `*/`, with the ability to be nested.
