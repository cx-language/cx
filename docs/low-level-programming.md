# Low-level programming

## `undefined`

By default cx warns you when you use a variable before initializing it.
The `undefined` keyword allows you to explicitly mark a variable as uninitialized, suppressing the warning.
This is useful for example when a variable is initialized through an out parameter.

```cs
void init(int& value) {
    *value = 42;
}

void main() {
    int result = undefined;
    init(result);
    println(result); // prints 42
}
```

## Memory arenas

`Arena` is a move-only, chunked bump allocator for short-lived groups of allocations. It grows by adding chunks, so pointers returned by earlier allocations stay valid until the arena is destroyed or explicitly deinitialized.

Allocation results are nullable. A null result means that the requested size overflowed or that the system allocator could not provide another chunk. Array counts use `uint64`, and the element-size multiplication is checked before requesting memory. Returned storage is aligned to 16 bytes. A zero-count array returns a valid pointer to zero-length storage.

```cs
void main() {
    var arena = Arena(chunkSize = 4096);

    var value = arena.allocate<int>();
    if value != null {
        value.init(42);
        println(*value); // prints 42
    }

    var values = arena.allocateArray<int>(3);
    if values != null {
        for i in 0..3 {
            values[i] = i;
        }
    }
}
```

`allocate<T>()` and `allocateArray<T>(count)` return uninitialized storage. Initialize each value before reading it. `allocate(value)` is a convenience overload that moves one initialized value into the arena. The arena destructor frees its backing chunks; it does not infer or run destructors for values placed in that storage, so explicitly deinitialize values that own resources before the arena is destroyed. Do not use an arena after calling `deinit()`. An arena is not thread-safe; concurrent writers need separate arenas or external synchronization.

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
to give common C functions more precise cx types.
Declare parameters and return types with the `c_` prefixed types
(`c_int`, `c_uint`, `c_long`, `c_double`, and the rest: see
[Builtin types](builtin-types)) so the signature matches the C headers
on every target:

```cs
extern c_int putchar(c_int ch);

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

## Calling cx from C

An `extern` function with a body is exported with C linkage under its plain
name, so C callers link it like any C function. It stays callable from cx
too:

```cs
extern c_int c_add(c_int a, c_int b) {
    return a + b;
}

void main() {
    println(c_add(40, 2)); // prints 42
}
```

Compile the cx side to an object file, then link it into the C program:

```sh
cx cxlib.cx -c -o cxlib.o
cc main.c cxlib.o -o c-caller
```

Signatures crossing the boundary must be C-compatible: plain numbers,
pointers, and C structs. Generic functions cannot be `extern`.
See the [`c-interop` example](https://github.com/emillaine/cx/tree/main/examples/c-interop)
for a complete project calling in both directions.
