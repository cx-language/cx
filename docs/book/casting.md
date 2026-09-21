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

Casts that don't make sense are rejected at compile time, for example `cast<int**>(false)`.
Conversions that are always safe need no syntax at all:
integer literals convert to the expected numeric type automatically,
pointers are dereferenced automatically when a value is expected,
and values bind to `T&` borrow parameters automatically.
Forming a `T*` pointer needs an explicit `&` (see [Pointers](pointers)).
