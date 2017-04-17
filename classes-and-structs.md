# Classes and Structs

New types can be defined either as classes or as structs. The different between
the two is that classes are _reference types_ while structs are _value types_.
Use class when instances of the type should be passed by reference, and struct
when they should be passed by value.

To declare a class:

```
class Foo {
    // member variables
}
```

To declare a struct:

```
struct Foo {
    // member variables
}
```

## Generic Classes and Structs

Classes and structs can have generic type parameters:

```
class Foo<T> { }
```
