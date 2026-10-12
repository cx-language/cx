# C and C++ interop

C and C++ headers import directly with no bindings to write, and cx
functions can be exported for C and C++ callers to link against.

## Using C libraries

For libraries the compiler vendors modules for, import the module instead
of the C header: it compiles faster and carries precise nullability.
See [Vendored C libraries](modules#vendored-c-libraries).

Other C headers can be imported directly from cx code.
To import a header, write an import declaration with the header file name:

```cs
import "stdio.h";

void main() {
    printf("hello, world\n");
}
```

Imported functions are called like ordinary cx functions.

<details>
<summary>Imported type name collisions</summary>

An imported struct, union, or enum shares its name with any cx type of the
same spelling, so defining both is a redefinition error; use the imported
type from all files instead of defining a second one. An empty struct or union
is exempt as an opaque placeholder for the imported type. A generic cx type
is a different spelling, so it coexists with a same-named import: argument
count picks the template when arguments are present.

</details>

Memory allocation functions work the same way:

```cs
import "stdlib.h";

void main() {
    var numbers = malloc(sizeof(int) * 3);
    free(numbers);
}
```

Numbers, characters, and strings are imported as constants,
as are simple expressions over numbers and characters:

```cs
import "stdio.h";

void main() {
    println(EOF); // prints -1
}
```

Compound literals of an imported struct are imported as constants when every
element is constant; the elements may be numbers, characters, earlier scalar
macros, or nested compound literals:

```c
typedef struct Color { unsigned char r, g, b, a; } Color;
#define RAYWHITE (Color){ 245, 245, 245, 255 }
```

```cs {.noCompile}
import "raylib.h";

void main() {
    println(RAYWHITE.g); // prints 245
}
```

<details>
<summary>Macro import limits</summary>

Partial initializers, designators,
unions, and structs with anonymous members are not imported.

</details>

Function-like macros and casts are not imported.

Individual functions can also be declared with `extern`, without importing a header.
The standard library uses `extern` declarations in `std/system/libc.cx`
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

## Const in C APIs

cx has no const-qualified pointees: `const` never appears in types. C and
C++ `const` is dropped when declarations are imported, so a C `const char*`
becomes plain `char*`. Passing a mutable pointer where C expects a const
one is safe (C only reads through it); writing through a pointer that C
considers const is undefined behavior, and the compiler cannot check it.
Named constants still use the `const` declarator (`const x = ...`).

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

On Windows, link the object file with `link.exe`, adding the C runtime
libraries the driver links automatically, plus your own libraries as
`.lib` files:

```sh {.noRun}
cx main.cx -c -o main.obj
link main.obj legacy_stdio_definitions.lib ucrt.lib msvcrt.lib /SUBSYSTEM:CONSOLE /OUT:demo.exe
```

Signatures crossing the boundary must be C-compatible: plain numbers,
pointers, and C structs. Generic functions cannot be `extern`.

<details>
<summary>extern C value-crossing limits</summary>

Structs cross by value following the platform C ABI, including small
float-containing ones, which cross in registers on x86-64 and AArch64 (on
other targets those are rejected). Aggregates holding 80-bit floats are
rejected; pass them behind a pointer or reference instead, and return
them through an out-parameter.

</details>

An `extern` function must be called directly; referring to one as a value
is rejected because a call through a function pointer cannot use the
callee's C calling convention.
See the [`c-interop` example](https://github.com/cx-language/cx/tree/main/examples/c-interop)
for a complete project calling in both directions.

## Using C++ libraries

C++ headers can be imported the same way as C headers.
A header with a C++ extension (`.hpp`, `.hh`, `.hxx`, `.h++`, or `.H`)
is parsed as C++, and its global-scope functions and plain structs become
available, including overloads (from the `cxx-interop` example):

```cs {.noCompile}
import "veclib.hpp";

void main() {
    Accum a;
    a.count = 3;
    a.total = 14;
    println(accum_add(a, 1)); // prints 15
}
```

<details>
<summary>C++ header import boundaries</summary>

Only declarations written in the imported header itself are imported.
Headers it includes are parsed for context, so their types work in
signatures, but their declarations are not imported; import each header
you use directly. Unsupported declarations (namespaces, templates,
globals, and non-POD types) are skipped with a warning naming how many
were skipped.

</details>

Signatures crossing the boundary must be C++-compatible: plain numbers,
pointers, references, function pointers, and plain structs.

<details>
<summary>C++ value-crossing limits</summary>

Structs
cross by value only when they are small (up to 16 bytes), normally
aligned, and hold no floats; anything else must cross behind a pointer
or reference. The same rules apply to extra arguments passed through
`...`. Structs are never returned by value; return through an
out-parameter instead.

</details>

A cx function can be passed where a C++ function
pointer is expected. `extern "C"` functions in C++ headers keep
their plain names and work as usual. Classes with methods and
exceptions are not supported.

Individual functions can also be declared with `extern "C++"`, without
importing a header. The compiler mangles the name with the Itanium C++
ABI, so it links against the C++ definition. Declare parameters and
return types with the `c_` prefixed types so the signature matches on
every target:

```cs {.noCompile}
extern "C++" c_int cpp_sum(c_int* values, c_int count);
```

C++ `const` on parameters is dropped, so cx cannot distinguish overloads
that differ only in constness. A hand-written declaration links against the
mutable overload when one exists. To reach a const-only overload, qualify
the pointee instead:

```cs {.noCompile}
extern "C++" c_int cpp_find(const c_int* values, c_int count);
```

`const T*` and `const T&` parameters mangle with const while staying
mutable in cx; callers pass pointers and borrows as usual.

<details>
<summary>Const qualifier restrictions</summary>

The qualifier
is mangle-only: it is rejected on by-value parameters, on definitions,
and on multi-level pointees, and declaring both the mutable and the
const overload collides as a redefinition.
When both overloads come from an imported header instead, the import
succeeds and calls resolve to whichever overload is declared first, mangled
with its original constness.

</details>

## Calling cx from C++

An `extern "C++"` function with a body is exported under its
Itanium-mangled name, so C++ callers declare and link it like any C++
function. It stays callable from cx too. The Itanium ABI is not
supported on Windows/MSVC targets, so this does not work there:

```cs {.noWindows}
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
See the [`cxx-interop` example](https://github.com/cx-language/cx/tree/main/examples/cxx-interop)
for a complete project calling in both directions.
