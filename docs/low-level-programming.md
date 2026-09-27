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

## The ambient context

Like Odin and Jai, cx has an ambient context: process-wide settings implicitly
used by the standard library. The context holds an allocator, initially the
default one, and `allocate` and friends allocate through it. The default
allocator uses malloc.

Install a custom allocator with `withAllocator` to route a stretch of code
through it, for example to count allocations or serve them from an arena:

```cs
int allocCount = 0;

void*? countingAlloc(void*? state, c_size_t size) {
    allocCount++;
    var def = defaultAllocator();
    return def.alloc(def.state, size);
}

void countingFree(void*? state, void*? ptr) {
    var def = defaultAllocator();
    def.free(def.state, ptr);
}

void makeList() {
    var list = List<int>();
    list.push(1);
}

void main() {
    var allocator = Allocator(state = null, alloc = countingAlloc, free = countingFree);
    withAllocator(allocator, makeList);
    println(allocCount); // prints 1
}
```

Each allocation must be freed with the same allocator that allocated it, so
values must not outlive the `withAllocator` call that allocated them.
Allocators must return 16-aligned pointers and tolerate freeing null.
A custom allocator must not call back into context-allocating standard library
functions, or allocation recurses forever; use `defaultAllocator()` for
passthrough as above. One exception: `Arena` chunks always use malloc, so an
arena installed as the context allocator never allocates from itself.

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

    var values = arena.allocate<int>(3);
    if values != null {
        for i in 0..3 {
            values[i] = i;
        }
    }
}
```

`allocate<T>()` and `allocate<T>(count)` return uninitialized storage. Initialize each value before reading it. `allocateValue(value)` is a convenience overload that moves one initialized value into the arena. The arena destructor frees its backing chunks; it does not infer or run destructors for values placed in that storage, so explicitly deinitialize values that own resources before the arena is destroyed. Do not use an arena after calling `deinit()`. An arena is not thread-safe; concurrent writers need separate arenas or external synchronization.

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

## Using C++ libraries

C++ headers can be imported the same way as C headers.
A header with a C++ extension (`.hpp`, `.hh`, `.hxx`, `.h++`, or `.H`)
is parsed as C++, and its global-scope functions and plain structs become
available, including overloads (from the `cxx-interop` example):

```cs {.noCompile}
import "veclib.hpp";

void main() {
    var xs = List<int>();
    xs.push(3);
    xs.push(1);
    println(vec_sum(xs)); // prints 4
}
```

Only declarations written in the imported header itself are imported.
Headers it includes are parsed for context, so their types work in
signatures, but their declarations are not imported; import each header
you use directly. Unsupported declarations (namespaces, templates,
globals, and non-POD types) are skipped with a warning naming how many
were skipped.

Signatures crossing the boundary must be C++-compatible: plain numbers,
pointers, references, function pointers, and plain structs. Structs
cross by value only when they are small (up to 16 bytes), normally
aligned, and hold no floats; anything else must cross behind a pointer
or reference. The same rules apply to extra arguments passed through
`...`. Structs are never returned by value; return through an
out-parameter instead. A cx function can be passed where a C++ function
pointer is expected. `extern "C"` functions in C++ headers keep
their plain names and work as usual. Classes with member functions and
exceptions are not supported.

`std::vector` parameters map to the standard library `CxxVector` type.
A `const std::vector<T>&` parameter imports as `const CxxVector<T>&`,
and `List` and slice values convert to it implicitly, so they pass
directly. `CxxVector` is a non-owning view: it borrows the cx storage
and must not outlive it, and it must not be passed where C++ may grow
the vector. Vectors never cross by value, and `vector<bool>` and custom
allocators are not supported.

Individual functions can also be declared with `extern "C++"`, without
importing a header. The compiler mangles the name with the Itanium C++
ABI, so it links against the C++ definition. Declare parameters and
return types with the `c_` prefixed types so the signature matches on
every target:

```cs {.noCompile}
extern "C++" c_int cpp_sum(const CxxVector<c_int>& v);
```

## Calling cx from C++

An `extern "C++"` function with a body is exported under its
Itanium-mangled name, so C++ callers declare and link it like any C++
function. It stays callable from cx too:

```cs
extern "C++" c_int cx_is_even(c_int n) {
    return n % 2 == 0 ? 1 : 0;
}

void main() {
    println(cx_is_even(42)); // prints 1
}
```

```cpp
int cx_is_even(int n);
```

```sh
cx cxlib.cx -c -o cxlib.o
c++ main.cpp cxlib.o -o cxx-caller
```

The same signature rules apply in both directions. Only free functions
can be `extern "C++"`; generic functions cannot be `extern`. C++
interop uses the Itanium ABI and is not supported on MSVC targets.
See the [`cxx-interop` example](https://github.com/emillaine/cx/tree/main/examples/cxx-interop)
for a complete project calling in both directions.
