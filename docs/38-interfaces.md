# Interfaces

Interfaces are abstract types that define a set of method requirements.
Other types can implement an interface, and the compiler checks that all the
required methods are provided by the implementing type.

Interfaces can provide default implementations of methods.
The implementing types will automatically inherit them.

```cs
interface Fooable {
    int foo();

    int fooSquared() {
        return foo() * foo();
    }
}

struct X: Fooable {
    int foo() {
        return 2;
    }
}

void callFoo<T: Fooable>(T& f) {
    println(f.foo());
}

void main() {
    var x = X();
    println(x.fooSquared()); // prints 4
    callFoo(x); // prints 2
}
```

Enums can implement interfaces as well; see [Enum types](./enum-types).
