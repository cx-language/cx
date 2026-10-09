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

A type parameter can be constrained to only accept types that implement one or more
[interfaces](interfaces). Constraints are written after a colon following the
parameter name, joined with `+`.

```cs
interface Fooable {
    int foo();
}

struct X: Fooable {
    int foo() {
        return 42;
    }
}

void callFoo<T: Fooable>(T& f) {
    println(f.foo());
}

void main() {
    var x = X();
    callFoo(x); // prints 42
}
```

Multiple constraints are all required:

```cs
interface Fooable {
    int foo();
}

struct Both: Fooable, Hashable {
    int foo() { return 42; }
    uint64 hash() { return 0; }
}

void callBoth<T: Fooable + Hashable>(T& value) {
    println(value.foo());
    println(value.hash());
}

void main() {
    callBoth(Both());
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

A type parameter can also be constrained to a function signature, written as a
[function pointer](function-pointers) type. The argument must then be a function
or lambda with a matching signature, including capturing lambdas. Untyped lambda
parameters are inferred from the signature:

```cs
void runTwice<Pred: bool(int&)>(Pred shouldRun) {
    for i in 0..2 {
        if shouldRun(i) {
            println(i);
        }
    }
}

void main() {
    int threshold = 1;
    runTwice(n => n >= threshold); // prints 1
}
```

## Integer parameters

Generic parameters can also take integer values instead of types.
An integer parameter is declared like a function parameter, with an integer
type and a name (e.g. `int N`). It can then size an array (`T[N]`) within
the declaration. Integer arguments are given explicitly or inferred from
matching array sizes. The standard library uses this for fixed-size arrays:

```cs
void main() {
    int[3] a = [10, 20, 30];
    println(a.size()); // prints 3
    println(a[2]); // prints 30
}
```

Only non-negative integer literals (and references to other integer
parameters) are accepted as integer arguments. Integer parameters can be used
as compile-time values, including as an array size.

## Generic constructors

Constructors can declare their own type and integer parameters, inferred
from the call arguments like any other generic call. On non-generic types
they can also be given explicitly (`Fixed<int>()`); on generic types explicit
arguments bind the type's parameters instead:

```cs
struct Counter {
    int total;

    Counter() {
        total = 0;
    }

    Counter<int N>(int[N] values) {
        total = N;
        for v in values {
            total += v;
        }
    }
}

void main() {
    var c = Counter([1, 2, 3]);
    println(c.total); // prints 9
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
int sumFirst<It: Iterator<int>>(It& iterator, int n) {
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
