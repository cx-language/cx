# Casting

cx has no C-style cast syntax. Instead, there are two explicit conversions:

- Converting between scalar types with a call to the target type: `int(x)`, `float(x)`, and so on.
- Reinterpreting pointers with the `cast` builtin: `cast<T>(x)`.

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

Any two pointer types convert directly, with no need to go through `void*`:

```cs
import "stdlib.h";

void main() {
    var p = malloc(4)!;
    int* ip = cast<int*>(p);
    *ip = 42;
    uint* up = cast<uint*>(ip);
    println(*up); // prints 42
    free(p);
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
Dropping `const` is rejected too: `cast<int*>` accepts `int*` and `void*`,
but not `const int*`.
Conversions that are always safe need no syntax at all:
integer literals convert to the expected numeric type automatically,
and values bind to `T&` borrow parameters automatically.
Forming a `T*` pointer needs an explicit `&`, and reading one needs an explicit `*`
(see [Pointers](pointers)).

Structs can also declare their own implicit conversions with `implicit`
constructors and member functions (see [Structs](structs)).
Those apply everywhere a value flows into an expected type,
such as arguments, return values, and initializers.
