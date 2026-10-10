# Assertions

Use `assert` to check conditions that must hold if the program is correct.
If the condition is false, the program prints the location and aborts.

```cs
int divide(int a, int b) {
    assert(b != 0);
    return a / b;
}

void main() {
    println(divide(7, 2)); // prints 3
}
```

`assert` takes an optional second argument, a string literal used as the
failure message. It is printed along with the location of the assertion.

```cs
void main() {
    var x = 2 + 2;
    assert(x == 4, "math is broken");
    println("math works"); // prints math works
}
```

`assert(false)` marks unreachable code: the compiler knows execution cannot continue past it,
so no return is needed after it.
To abort unconditionally with a message, call `abort`:

```cs
void fail(string message) {
    abort("fatal error: ", message);
}

void main() {
    var ok = true;
    if !ok {
        fail("something went wrong");
    }
    println("continuing");
}
```

Asserts run in debug and release-safe builds. `--release` skips them without
evaluating the condition, except in `@test` functions, which keep their asserts
in all modes so `cx test --release` still checks them.
Code that must abort in release builds should call `abort` explicitly.
