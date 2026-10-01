# Casting

cx has no C-style cast syntax. Instead, there are two explicit conversions:

- Converting between scalar types with a call to the target type: `int(x)`, `float(x)`, and so on.
- Reinterpreting pointers, array pointers, and references with the `cast` builtin: `cast<T>(x)`.

```cs
void main() {
    var x = 7;
    println(float(x) / 2); // prints 3.5

    float pi = 3.14;
    println(int(pi)); // prints 3
}
```

`cast` converts between pointer types, for example from `void*` to a concrete pointer type:

```cs
import "stdlib.h";

void main() {
    var p = malloc(sizeof(int))!;
    int* numbers = cast<int*>(p);
    *numbers = 42;
    println(*numbers); // prints 42
    free(p);
}
```

`cast` reinterprets between pointers (`T*`), array pointers (`T[*]`), and references (`T&`).
Any of those may have one optional (`T*?`, `T[*]?`, `T&?`). A second `?` is a struct, not a null pointer, so `T*??` cannot be cast this way.
The pointee type may change, and `const` may be added or dropped:

```cs
void main() {
    int x = 42;
    int* ip = &x;
    uint* up = cast<uint*>(ip);
    println(*up); // prints 42

    int[*] ap = cast<int[*]>(ip);
    println(ap[0]); // prints 42

    int& r = cast<int&>(ap);
    println(r); // prints 42

    int* mp = cast<int*>(cast<const int*>(ip));
    println(*mp); // prints 42
}
```

`cast` also converts between pointers and integers in both directions.
This supports a `uintptr_t`-style round trip for inspecting an address; prefer
`c_size_t` when the integer must hold a pointer-sized value:

```cs
void main() {
    int x = 42;
    int* p = &x;
    c_size_t address = cast<c_size_t>(p);
    int* q = cast<int*>(address);
    println(q == p); // prints true
}
```

Casts that don't make sense are rejected at compile time, for example `cast<int**>(false)`.
Conversions that are always safe need no syntax at all:
integer literals convert to the expected numeric type automatically,
and values bind to `T&` borrow parameters automatically.
Forming a `T*` pointer needs an explicit `&`, and reading one needs an explicit `*`
(see [Pointers](pointers)).

Structs can also declare their own implicit conversions with `implicit`
constructors and member functions (see [Structs](structs)).
Those apply everywhere a value flows into an expected type,
such as arguments, return values, and initializers.
