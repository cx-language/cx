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
void printValue(int value) {
    println("integer: ", value);
}

void printValue(bool value) {
    println("boolean: ", value);
}

void main() {
    printValue(42); // prints "integer: 42"
    printValue(false); // prints "boolean: false"
}
```

Functions may be overloaded on parameter names:

```cs
void move(public int x) {
    println("x: ", x);
}

void move(public int y) {
    println("y: ", y);
}

void main() {
    move(x = 3); // prints "x: 3"
    move(y = 4); // prints "y: 4"
}
```

Arguments can be passed positionally or by name.
Named arguments can be given in any order:

```cs
StringBuf greet(string greeting, string name) {
    return greeting + " " + name;
}

void main() {
    var message = greet(name = "world", greeting = "hi");
    println(message); // prints "hi world"
}
```

Argument expressions always evaluate in the order written.

Parameters can have default values, which may be omitted. Later
arguments can still be passed by name when an earlier default
is omitted:

```cs
void greet(string greeting = "hi", string name = "world") {
    println(greeting, " ", name);
}

void main() {
    greet(); // prints "hi world"
    greet(name = "cx"); // prints "hi cx"
}
```

Functions can return multiple values using anonymous structs:

```cs
(int quotient, int remainder) divmod(int a, int b) {
    return (quotient = a / b, remainder = a % b);
}

void main() {
    var result = divmod(7, 2);
    println(result.quotient); // prints 3
    println(result.remainder); // prints 1
}
```

A generic function can take a variable number of arguments with a variadic
parameter. The variadic parameter must be the last parameter, and each argument
matching it can have a different type. Inside the function, iterate over the
parameter with a for loop:

```cs
void printAll<T: Printable>(T&... args) {
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

- Destructuring for multiple return values
- Anonymous structs with unnamed members
