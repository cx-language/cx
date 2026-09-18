# Generics

Functions and structs can be parameterized over types.
Type parameters are declared in angle brackets after the function or struct name,
and can then be used like any other type within the declaration.

```cs
void foo<T>(T t) {
}

T bar<T>(T t) {
    return t;
}

void main() {
    foo<int>(1);
    foo<bool>(false);

    // Type arguments can usually be inferred from the call arguments,
    // so they don't have to be specified explicitly.
    var b = bar("bar"); // b has type 'string'
}
```

Generic structs work the same way.
The type arguments are specified when instantiating the struct,
or inferred when possible.

```cs
void main() {
    var list = List<int>();
    var boolList = List([true, false]);
    var map = Map<int, int>();
}
```

## Generic constraints

A type parameter can be constrained to only accept types that implement a given [interface](interfaces).
The constraint is written after a colon following the parameter name.

```cs
interface Fooable {
    int foo();
}

struct X: Fooable {
    int foo() {
        return 42;
    }
}

void callFoo<T: Fooable>(T* f) {
    println(f.foo());
}

void main() {
    var x = X();
    callFoo(x); // prints 42
}
```

Standard library types use constraints in the same way,
for example `Map` requires its keys to be hashable:

```cs
struct MapEntry<Key: Hashable, Value> {
    Key key;
    Value value;
}
```
