# Functions

Functions don't have to be declared before they're called:

```cs
void main() {
    println(sum(1, 2)); // prints 3
}

int sum(int a, int b) {
    return a + b;
}
```

Function parameters can have a default value, allowing the parameter to be omitted when calling the function:

```cs
void foo(int i = 0) {
    println(i);
}

void main() {
    foo(42); // prints 42
    foo(); // prints 0
}
```

Functions may be overloaded on parameter types:

```cs
void foo(int i) {
    println("int");
}

void foo(bool b) {
    println("bool");
}

void main() {
    foo(42); // prints "int"
    foo(false); // prints "bool"
}
```

Functions may be overloaded on parameter names:

```cs
void foo(a: int) {
    println("a");
}

void foo(b: int) {
    println("b");
}

void main() {
    foo(a: 42); // prints "a"
    foo(b: 42); // prints "b"
}
```

Functions can return multiple values:

```cs
(int, bool) foo() {
    return (42, true);
}

void main() {
    var (i, b) = foo();
    println(i); // prints 42
    println(b); // prints true
}
```
