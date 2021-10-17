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

    if (foo) {
        println("foo is non-null"); // this is not executed because foo is null
    }

    // This should warn about foo being potentially null, but does currently not due to a bug.
    // This will also abort at runtime if foo is null.
    // takeInt(foo);

    if (foo) {
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
    int? x = 1

    // println(x + 1); // doesn't work, cannot add to potentially null value
    println(x! + 1); // fine if x is non-null, otherwise will abort at runtime
}
```
