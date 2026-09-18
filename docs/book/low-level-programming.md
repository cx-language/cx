# Low-level programming

## `undefined`

By default C* warns you when you use a variable before initializing it.
The `undefined` keyword allows you to explicitly mark a variable as uninitialized, suppressing the warning.
This is useful for example when you need to pass a variable to a C function to be initialized as an out parameter.

```cs
void main() {
    char[3] result = undefined;
    sprintf(result, "%d", 42);
    // result is ['4', '2', '\0']
}
```

## Using C libraries

C headers can be imported directly from C* code.
The compiler uses the Clang API to parse the header and make its declarations available.
To import a header, write an import declaration with the header file name:

```cs
import "stdio.h";

void main() {
    printf("foo bar");
}
```

Imported functions are called like ordinary C* functions.
Memory allocation functions work the same way:

```cs
import "stdlib.h";

void main() {
    var buffer = malloc(1);
    buffer = realloc(buffer, 4);
    free(buffer);
}
```

Individual functions can also be declared with `extern`, without importing a header.
The standard library uses `extern` declarations in `std/libc.cx`
to give common C functions more precise C* types:

```cs
extern int putchar(int ch);

void main() {
    putchar(102);
}
```

C struct fields are accessed with `.`, just like C* struct fields.
A C* function can be passed where a C function pointer is expected:

```cs
import "stdlib.h";

void onExit() {
    println("exiting");
}

void main() {
    atexit(onExit);
    println("running");
}
```

Additional include directories, preprocessor definitions, and libraries
are passed on the command line with `-I`, `-D`, `-L`, and `-l`.
