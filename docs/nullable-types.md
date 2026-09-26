# Nullable types

Nullable types, also known as optional types, are used represent values that may or may not exist.
Values of nullable type allow `null` to be assigned to them.
To specify that a type is nullable, you append `?` to it.

The compiler enforces null-safety by default, i.e. checks that we don't forget to handle the case when the value is null.
This means you need to check nullable values for null before accessing them.

```cs
void main() {
    int? foo = 42;
    println(foo); // prints 42

    foo = null;
    println(foo); // prints null

    if foo {
        println("foo is non-null"); // this is not executed because foo is null
    }

    // Uncommenting the following line would warn about foo being potentially null.
    // It would also abort at runtime since foo is null here.
    // takeInt(foo);

    if foo {
        takeInt(foo); // fine, foo was checked for null
    }
}

void takeInt(int n) {}
```

## Non-null assertion operator

In some cases, we know that a value of nullable type is non-null, but the compiler cannot verify it.
We can tell the compiler that the value is non-null by suffixing it with an `!`.
This is called the non-null assertion operator.
If we use it on a value that's actually null, an assertion error will be triggered.

```cs
void main() {
    int? x = 1;

    // println(x + 1); // doesn't work, cannot add to potentially null value
    println(x! + 1); // fine if x is non-null, otherwise will abort at runtime
}
```

Unwrapping preserves the value category of the operand:
unwrapping a variable (or another lvalue) designates the payload in place,
so it can be assigned to, while unwrapping a temporary is still an rvalue.

```cs
void main() {
    float[2]? opt = [1.0, 2.0];
    opt![0] = 5.0; // writes through to the payload
}
```

Postfix `!` desugars into an `unwrap` method call,
so it also works on any other type with an `unwrap` method,
such as [`Result`](error-handling):

```cs
void main() {
    Result<int, string> digit = Ok(7);
    println(digit!); // prints 7, aborts with the error if Err
}
```

## Switching on nullable values

The `?` suffix is shorthand for the generic `Optional` enum,
so a nullable value can also be matched with a `switch` statement.
The `Some` case binds the wrapped value, if any.

```cs
void main() {
    int? x = 1;

    switch (x) {
        case Some value:
            println(value); // prints 1
        case None:
            println("x is null");
    }
}
```

Only value types can be switched on this way;
to check a nullable pointer for null, compare it against `null` instead.

## Null coalescing operator

The `??` operator provides a default value for a nullable expression:
if the left side is non-null, its value is used, otherwise the right
side is evaluated and used instead.

```cs
void main() {
    int? a = null;
    println(a ?? 7); // prints 7

    int? b = 42;
    println(b ?? 7); // prints 42

    string? name = null;
    string displayName = name ?? "anonymous";
    println(displayName); // prints anonymous
}
```

The left side is evaluated once, and the right side only when the left
side is null. If the right side is also nullable, the result stays
nullable:

```cs
void main() {
    int? a = null;
    int? b = null;
    int? c = a ?? b;
    println(c); // prints null
}
```
