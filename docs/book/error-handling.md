# Error handling

cx has no exceptions. Instead, the current idioms for reporting failures are
nullable return values and status codes, checked by the caller:

```cs
int? parseDigit(char c) {
    if c >= '0' && c <= '9' {
        return int(c) - int('0');
    }
    return null;
}

void main() {
    if parseDigit('7') {
        println("it's a digit");
    }

    var result = parseDigit('x');
    if result == null {
        println("not a digit");
    }
}
```

See [Nullable types](nullable-types) for how the compiler checks
that nullable values aren't used without a null check.

## Result type

When a failure carries information about what went wrong,
return a `Result<T, E>` holding either the success value (`Ok`)
or the error (`Err`):

```cs
Result<int, string> parseDigit(char c) {
    if c >= '0' && c <= '9' {
        return Ok(int(c) - int('0'));
    }
    return Err("not a digit");
}

void main() {
    switch (parseDigit('7')) {
        case Ok value:
            println(value.value); // prints 7
        case Err error:
            println(error.error);
    }
}
```

Use `== Ok` and `== Err` to test which case a result holds.
`unwrap` returns the success value, or aborts with the error:

```cs
Result<int, string> parseDigit(char c) {
    if c >= '0' && c <= '9' {
        return Ok(int(c) - int('0'));
    }
    return Err("not a digit");
}

void main() {
    var digit = parseDigit('7');
    if digit == Ok {
        println(unwrap(digit)); // prints 7
    }

    var notDigit = parseDigit('x');
    if notDigit == Err {
        println("not a digit");
    }
}
```

## Assertions

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

## Planned features

- Custom messages in `assert`.
