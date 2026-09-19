# Closures

A lambda is an anonymous function, written as a parameter list followed by `->` and a body.
Lambdas are commonly passed to higher-order functions such as `map` and `filter`:

```cs
void main() {
    var numbers = List([0, 1, 2, 3, 4]);

    var even = numbers.filter(n -> n % 2 == 0);
    println(even); // prints [0, 2, 4]

    var doubled = even.map(n -> n * 2);
    println(doubled); // prints [0, 4, 8]
}
```

When a lambda is passed directly to a function, its parameter types are inferred
from the expected [function pointer](function-pointers) type.
When there is no expected type, for example when storing a lambda in a variable,
the parameter types must be written explicitly:

```cs
int apply(int(int) f, int x) {
    return f(x);
}

void main() {
    var triple = (int n) -> n * 3;
    println(triple(14)); // prints 42
    println(apply(n -> n + 1, 41)); // prints 42, parameter type inferred
}
```

A lambda body can also be a block with statements, like an ordinary function body:

```cs
void main() {
    var f = (int x, int y) -> {
        var sum = x + y;
        return sum * 2;
    };
    println(f(10, 11)); // prints 42
}
```

## Planned features

- Capturing variables from the enclosing scope; for now, lambdas can only use their parameters.
