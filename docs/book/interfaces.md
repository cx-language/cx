# Interfaces

Interfaces are abstract types that define a set of requirements (methods).
Other types can implement an interface, and the compiler checks that all the
required methods are provided by the implementing type.

```cs
interface Fooable {
    int foo();
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
    callFoo(x); // prints 2
}
```
