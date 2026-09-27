# Enum types

cx's enums are scoped and can optionally have associated values for each enum value.

```cs
enum JsonNode {
    String(StringBuf value),
    Array(List<JsonNode> value),
    Object(Map<StringBuf, JsonNode> value),
    Number(float64 value),
    Boolean(bool value),
    Null,
}

void main() {
    JsonNode a = JsonNode.Number(42);

    switch (a) {
        case String: println("it's a string");
        case Array: println("it's an array");
        case Object: println("it's an object");
        case Number: println("it's a number");
        case Boolean: println("it's a boolean");
        case Null: println("it's null");
    }
}
```

The standard library provides its own `JsonValue` tagged union,
with JSON parsing and printing, on the `json` reference page.

Enums can be generic, in which case the type parameters can be used
in the associated values of any case.
The type arguments are inferred from the associated values when possible,
specified explicitly on the case, or taken from the expected type.

```cs
enum Outcome<T> {
    Ok(T value),
    Err(int code),
}

void main() {
    var _a = Outcome.Ok(1); // Outcome<int>, inferred
    var _b = Outcome.Ok<bool>(true); // explicit type arguments
    Outcome<int> _c = Err(404); // from the declared type
}
```

The standard library provides a general `Result<T, E>` enum for error handling;
see [Error handling](error-handling).

The enum name can be omitted when the expected type determines which enum
is meant, such as in a return statement or an initializer with a declared type:

```cs
enum Outcome<T> {
    Ok(T value),
    Err(int code),
}

Outcome<int> check(bool ok) {
    if ok {
        return Ok(1);
    }
    return Err(404);
}

void main() {
    Outcome<int> _a = check(true);
    Outcome<int> _b = Err(500);
}
```

Enums can define member functions, which are called on enum values.
Inside a member function, `this` is the enum value the function was called on:

```cs
enum Outcome<T> {
    Ok(T value),
    Err(int code),

    bool isOk() {
        return this == Ok;
    }

    T unwrapOr(T fallback) {
        switch this {
            case Ok v: return v;
            case Err: return fallback;
        }
    }
}

void main() {
    var a = Outcome.Ok(1);
    println(a.isOk()); // prints true
    println(a.unwrapOr(0)); // prints 1
    println(Outcome.Err<int>(404).unwrapOr(0)); // prints 0
}
```

Enums can also implement interfaces by listing them after the enum name:

```cs
enum Color: Printable {
    Red,
    Green,
    Blue,

    void print(OutputStream& stream) {
        switch this {
            case Red: stream.append("red");
            case Green: stream.append("green");
            case Blue: stream.append("blue");
        }
    }
}

void main() {
    println(Color.Red); // prints red
}
```

A hand-written `print` is only needed for custom output like this;
plain enums print without any boilerplate, as described below.

## Iterating cases

A `for` loop over an enum without associated values visits each case
in declaration order:

```cs
enum Color {
    Red,
    Green,
    Blue,
}

void main() {
    for color in Color {
        println(color == Color.Red); // prints true, then false, false
    }
}
```

Enums with associated values and generic enums cannot be iterated;
looping over them is a compile error.

## Printing

Enums print generically: `print` renders the case name, followed by
the associated values in parentheses when the case carries any, so no
boilerplate is needed for logging and debugging.
The compiler generates the equivalent of a hand-written `print` method
the first time an enum value is printed, and the enum also gains
`toString` through the `Printable` interface:

```cs
enum Color {
    Red,
    Green,
    Blue,
}

enum Outcome<T> {
    Ok(T value),
    Err(int code),
}

void main() {
    println(Color.Red); // prints "Red"
    println(Outcome.Ok(42)); // prints "Ok(42)"
    println(Outcome.Err<int>(404)); // prints "Err(404)"
}
```

Cases with several associated values print them positionally:
`Click(3, 4)`. Nested enums, generic enums instantiated with
printable arguments, and containers of enums all work. Nullable
associated values print as `null` when null. An enum with its own
`print` method or a member named `print` keeps it; the compiler never
overrides user-declared members.
