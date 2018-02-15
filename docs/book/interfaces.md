# Interfaces

Let's suppose we are writing a generic sort function that can sort an array of
any type:

```go
func sort<T>(array: mutable Array<T>*) {
    // ...
}
```

Somewhere in our implementation of `sort` we are going to have to compare the
elements e.g. with the `<` operator. But what happens if the element type is a
user-defined type that doesn't provide a `<` operator? Luckily we'll get a
compile-time error saying that no matching `<` operator could be found. However
the location of this error is inside the implementation of `sort`, possibly
inside some helper function that `sort` calls.

As a user of the `sort` function, this is not a very nice way to find out you
did something wrong. In this specific example the error message will be quite
clear, but in other cases the issue might not be immediately obvious, and you
could have to traverse deep template instantiation stack traces to find the root
of the problem. Ideally we would like to get the error right where we tried to
call `sort`. Fortunately there's a way to do this.

## Generic Constraints

When declaring the `sort` function, we can specify that the types that will be
substituted for `T` must be comparable with the `<` operator using the
`Comparable` interface as a _generic constraint_:

```go
func sort<T: Comparable>(array: mutable Array<T>*) {
    // ...
}
```

`Comparable` is an interface which specifies that instances of the type can be
compared using e.g. `<`, among other things. If we now try to call `sort` with
an array whose element type doesn't implement `Comparable`, we'll get a nice
error message:

```
main.delta:4:9: error: type 'X' doesn't implement interface 'Comparable'
    sort(array);
        ^
```

## Declaring Interfaces

Interfaces are types that specify a set of member functions. Struct types can
implement an interface by listing the interface name after a colon following the
struct name, and providing the member functions required by the interface:

```go
interface Fooable {
    func foo() -> int;
}

struct X: Fooable {
    func foo() -> int {
        return 42;
    }
}
```

## Default Member Functions

Interfaces may also contain member functions with implementations, called
_default member functions_:

```go
interface Fooable {
    func foo() -> int;

    func fooSquared() -> int {
        return foo() * foo();
    }
}
```

Now every type that implements `Fooable` gets the `fooSquared()` member function
for free.
