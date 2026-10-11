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
Inside the branch, the variable narrows to the matched case's payload,
so the success value is available directly as `digit.value`.
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
        println(digit.value); // prints 7
    }
    println(parseDigit('7')!); // prints 7, shorthand for .unwrap()

    var notDigit = parseDigit('x');
    if notDigit == Err {
        println("not a digit");
    }
}
```

See [Assertions](assertions) for checking conditions that must hold.
