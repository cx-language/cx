# Functions

Functions are declared as follows:

```
void foo() { }
```

Parameters go inside the parentheses, like so:

```
void foo(int a, int b) { }
```

If the function has a return type, it is specified after the parameter list:

```
int foo(int a, int b) {
    return a + b;
}
```

To call the function and assign its return value to a variable:

```
var result = foo(1, 2);
```

## Multiple Return Values

Functions can return more than one value by returning a tuple of values:

```
(int key, bool value) foo() {
    var key = 42;
    var value = true;
    return (key, value);
}
```

## Default Parameter Values

Parameters can be declared to have a default value:

```
void foo(int i = 42) {
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
void foo<T>() { }
```

`T` is a now a placeholder for a type that can be used in the definition of `foo`.
