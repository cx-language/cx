# Classes and Structs

New types can be defined either as classes or as structs. The different between
the two is that classes are _reference types_ while structs are _value types_.
Use class when instances of the type should be passed by reference, and struct
when they should be passed by value.

To declare a class:

```
class Foo {
    // member variables and functions
}
```

To declare a struct:

```
struct Foo {
    // member variables and functions
}
```

## Member Functions

Member functions are just like non-member functions, except that they receive a
`this` parameter (aka the receiver) which is passed using the dot notation, when
calling the function.

To define a member function for a type, write a function inside the type's
definition:

```
class Foo {
    func bar() {
        // ...
    }
}
```

To call the member function on a `Foo` instance:

```
foo.bar();
```

Inside member functions, the receiver and its member variables must be accessed
via `this`.

### Mutating Member Functions

To declare a member function that can mutate `this`, the function has to be
declared `mutating`:

```
class Foo {
    mutating func bar() {
        // ...
    }
}
```

### Static Member Functions

Member functions that are scoped to a type, but that are not called on a
receiver, can be declared with the `static` keyword (not implemented yet):

```
class Foo {
    static func bar() {
        // ...
    }
}
```

They are called as follows:

```
Foo.bar();
```

## Generic Classes and Structs

Classes and structs can have generic type parameters:

```
class Foo<T> { }
```
