# Function pointers

A function pointer type is written as the return type followed by the parameter types in parentheses.
For example, `int(int)` is a pointer to a function taking an `int` and returning an `int`.

Function pointers are typically used for callbacks:
a function takes a function pointer parameter, and the caller passes a function
(or a [lambda](closures)) to be called back.

```cs
int apply(int(int) f, int x) {
    return f(x);
}

int triple(int n) {
    return n * 3;
}

void main() {
    println(apply(triple, 14)); // prints 42
}
```

A function pointer can also be stored in a variable or a struct field,
and called like an ordinary function:

```cs
int double(int n) {
    return n * 2;
}

struct Handler {
    int(int) callback;
}

void main() {
    var f = double;
    println(f(21)); // prints 42

    var handler = Handler(callback = double);
    println(handler.callback(21)); // prints 42
}
```

A cx function can be passed where a C function pointer is expected,
see [Using C libraries](low-level-programming). Pass the function directly:
a function pointer held in a variable can't be converted, and the callback
sees the default context rather than the caller's.

A C callback type can be named with an `extern` function type,
which uses the C calling convention (no hidden context parameter):

```cs
using Handler = extern void(c_int);

void onEvent(c_int code) {
    _ = code;
}

void main() {
    Handler h = onEvent;
    h(42);
}
```
