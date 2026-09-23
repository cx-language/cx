# Low-level programming

## `undefined`

By default cx warns you when you use a variable before initializing it.
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

C headers can be imported directly from cx code.
The compiler uses the Clang API to parse the header and make its declarations available.
To import a header, write an import declaration with the header file name:

```cs
import "stdio.h";

void main() {
    printf("hello, world\n");
}
```

Imported functions are called like ordinary cx functions.
Memory allocation functions work the same way:

```cs
import "stdlib.h";

void main() {
    var numbers = malloc(sizeof(int) * 3);
    free(numbers);
}
```

Individual functions can also be declared with `extern`, without importing a header.
The standard library uses `extern` declarations in `std/libc.cx`
to give common C functions more precise cx types:

```cs
extern int putchar(int ch);

void main() {
    putchar('f'); // prints "f"
}
```

C struct fields are accessed with `.`, just like cx struct fields.
A cx function can be passed where a C function pointer is expected:

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
See [Modules and imports](modules) for how imports are resolved.
