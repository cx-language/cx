# Generics

Functions, structs, and enums can be parameterized over types.
Type parameters are declared in angle brackets after the function or type name,
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
    println(b); // prints bar
}
```

Generic structs work the same way.
The type arguments are specified when instantiating the struct,
or inferred when possible.

```cs
void main() {
    var _list = List<int>();
    var _boolList = List([true, false]);
    var _map = Map<int, int>();
}
```

Generic enums work the same way.
The type parameters can be used in the associated values of any case.

```cs
enum Opt<T> {
    Some(T value),
    None,
}

void main() {
    var _present = Opt.Some(1);
    Opt<int> _absent = None;
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

```cs {.noRun}
struct MapEntry<Key: Hashable, Value> {
    Key key;
    Value value;
}
```

## Generic interfaces

Interfaces can also take type parameters.
This pays off when the interface needs to talk about a type:
`Iterator<T>`'s `value()` returns `T`, so one interface serves every element type
while keeping everything statically typed.
A generic algorithm constrained on `Iterator<int>` accepts any implementation,
standard or hand-written, and knows `value()` returns `int`:

```cs
int sumFirst<It: Iterator<int>>(It* iterator, int n) {
    var total = 0;
    var i = 0;
    while i < n && iterator.hasValue() {
        total += iterator.value();
        iterator.increment();
        i++;
    }
    return total;
}

struct Counter: Iterator<int> {
    int current;

    bool hasValue() {
        return current > 0;
    }

    int value() {
        return current;
    }

    void increment() {
        current--;
    }
}

void main() {
    println(sumFirst((1..10).iterator(), 3)); // prints 6
    var counter = Counter(3);
    println(sumFirst(counter, 2)); // prints 5
}
```

Without the type parameter, each element type would need its own interface,
and `value()` couldn't declare what it returns.
See [Iterators](iterators) for more on the `Iterator` interface.
