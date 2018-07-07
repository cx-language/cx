# Functions

Functions are declared using the `def` keyword:

```
def foo() { }
```

Parameters go inside the parentheses, like so:

```
def foo(a: int, b: int) { }
```

If the function has a return type, it is specified after the parameter list:

```
def foo(a: int, b: int): int {
    return a + b;
}
```

To call the function and assign its return value to a variable:

```
let result = foo(1, 2);
```

## Multiple Return Values

Functions can return more than one value by returning a tuple of values:

```
def foo(): (key: int, value: bool) {
    let key = 42;
    let value = true;
    return (key, value);
}
```

## Default Parameter Values

Parameters can be declared to have a default value:

```
def foo(i: int = 42) {
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
def foo<T>() { }
```

`T` is a now a placeholder for a type that can be used in the definition of `foo`.
