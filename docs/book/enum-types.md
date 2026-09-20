# Enum types

C*'s enums are scoped and can optionally have associated values for each enum value.

```cs
enum JsonValue {
    String(StringBuffer value),
    Array(List<JsonValue> value),
    Object(Map<StringBuffer, JsonValue> value),
    Number(float64 value),
    Boolean(bool value),
    Null,
}

void main() {
    JsonValue a = JsonValue.Array(List([
        JsonValue.Boolean(false), 
        JsonValue.Number(42), 
        JsonValue.Null
    ]));

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
