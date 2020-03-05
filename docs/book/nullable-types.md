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
        println("foo is non null"); // this is not executed because foo is null
    }

    // doesn't compile: using foo as an int without checking it for null
    // int bar = foo + 1;

    if (foo) {
        // fine
        int bar = foo + 1;
    }
}
```

## Non-null assertion operator

In some cases, we know that a value of nullable type is non-null, but the compiler cannot verify it.
We can tell the compiler that the value is non-null by suffixing it with an `!`.
This is called the non-null assertion operator.
If we use it on a value that's actually null, an assertion error will be triggered.

```cs
void main() {
    int? result = foo(true);

    // println(result + 1); // doesn't work, compiler says result may be null
    println(result! + 1); // fine, but only if result is indeed non-null
}

int? foo(bool b) {
    if (b) {
        return 42;
    } else {
        return null;
    }
}
```
