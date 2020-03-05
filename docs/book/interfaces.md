# Interfaces

Interfaces are abstract types that define a set of requirements (methods and/or fields).
Other types can implement an interface, and the compiler checks that all the 
required methods and fields are provided by the implementing type.

Interfaces can also provide default implementations of methods.
The implementing types will automatically inherit them.
Fields defined in an interface are also automatically inherited.

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

void callFoo<T: Fooable>(T* f) {
    println(f.foo());
}

void main() {
    var x = X();
    println(x.fooSquared()); // prints 4
    callFoo(x); // prints 2
}
```
