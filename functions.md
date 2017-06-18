# Functions

Functions are declared using the `func` keyword:

```
func foo() { }
```

Parameters go inside the parentheses, like so:

```
func foo(a: int, b: int) { }
```

If the function has a return type, it is specified after the parameter list:

```
func foo(a: int, b: int) -> int {
    return a + b;
}
```

To call the function and assign its return value to a variable:

```
const result = foo(1, 2);
```

## Member Functions

You can declare member functions for any type. Member functions are just like
non-member functions, except that they receive a `this` parameter (aka the
receiver) which is passed using the dot notation.

To declare a member function for type `Foo`:

```
func Foo::bar() {
    // ...
}
```

To call the member function on a `Foo` instance:

```
foo.bar();
```

Inside member functions, the receiver and its member variables must be accessed
via `this`.

## Mutating Member Functions

To declare a member function that can mutate its receiver, the function has to
be declared `mutating`:

```
mutating func Foo::bar() {
    // ...
}
```

## Static Member Functions

Member functions that are scoped to a type, but that are not called on a
receiver, can be declared with the `static` keyword:

```
static func Foo::bar() {
    // ...
}
```

They are called as follows:

```
Foo::bar();
```

## Multiple Return Values

Functions can return more than one value (not fully implemented yet):

```
func foo() -> int, bool {
    return 42, true;
}
```

## Default Parameter Values

Parameters can be declared to have a default value:

```
func foo(i: int = 42) {
    // ...
}
```

This allows the parameter to be omitted from the function call:

```
foo();
```

in which case `foo` will receive `i` with the value `42`.

## Generic Functions

To define generic functions, specify the type parameters inside angle brackets
after the function name:

```
func foo<T>() { }
```

`T` is a now a placeholder for a type that can be used in the definition of `foo`.
