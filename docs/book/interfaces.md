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

If we now try to call `sort` with an array whose element type is not comparable
with `<`, we'll get a nice error message:

```
main.delta:4:9: error: type 'X' doesn't implement interface 'Comparable' because it doesn't have member function '<'
    sort(array);
        ^
```

## Declaring Interfaces

Interfaces are types that specify a set of member functions. Other types can
implement an interface by providing those member functions. For example:

```go
interface Fooable {
    func foo() -> int;
}

struct X {
    func foo() -> int {
        return 42;
    }
}
```

Now `X` implements the interface `Fooable`. We can also explicitly declare `X`
to implement `Fooable` like so:

```go
struct X: Fooable {
    // ...
}
```

Now the compiler sees that we intended `X` to be `Fooable`, and so it can check
that we provided all the member functions required by `Fooable`.

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

Now every type that explicitly implements `Fooable` gets the `fooSquared()`
member function for free.
