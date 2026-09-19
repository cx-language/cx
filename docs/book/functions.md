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
void foo(public int a) {
    println("a");
}

void foo(public int b) {
    println("b");
}

void main() {
    foo(a = 42); // prints "a"
    foo(b = 42); // prints "b"
}
```

Functions can return multiple values using tuples:

```cs
(int a, bool b) foo() {
    return (a = 42, b = true);
}

void main() {
    var result = foo();
    println(result.a); // prints 42
    println(result.b); // prints true
}
```

A generic function can take a variable number of arguments with a variadic
parameter. The variadic parameter must be the last parameter, and each argument
matching it can have a different type. Inside the function, iterate over the
parameter with a for loop:

```cs
void printAll<T: Printable>(T*... args) {
    for arg in args {
        print(arg);
    }
    print('\n');
}

void main() {
    printAll("a", 1, "b", 2); // prints "a1b2"
}
```

## Planned features

- Default parameter values
- Destructuring for multiple return values
- Tuples with anonymous members
