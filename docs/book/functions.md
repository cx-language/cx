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
let result = foo(1, 2);
```

## Multiple Return Values

Functions can return more than one value by returning a tuple of values:

```
func foo() -> (key: int, value: bool) {
    let key = 42;
    let value = true;
    return (key, value);
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
