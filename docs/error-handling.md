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
            println(value); // prints 7
        case Err error:
            println(error);
    }
}
```

Use `== Ok` and `== Err` to test which case a result holds.
The `unwrap` method returns the success value, or aborts with the error:

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
        println(digit.unwrap()); // prints 7
    }
    println(parseDigit('7')!); // prints 7, shorthand for .unwrap()

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

## Unimplemented code

Use `todo` to mark code that isn't written yet.
It aborts with "Not implemented", or with the given reason,
so unfinished paths fail loudly instead of silently doing the wrong thing.
Since `todo` never returns, the compiler accepts it anywhere a value is expected.

```cs
int parsePort(string service) {
    if service == "http" {
        return 80;
    }
    todo("only http is supported for now");
}

void main() {
    println(parsePort("http")); // prints 80
}
```
