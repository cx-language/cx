# Variables

## Declaration

A simple variable declaration looks like this:

```
let foo = 42;
```

Because this declaration uses the `let` keyword, the variable is immutable.

We can also declare variables using the `var` keyword:

```
var foo = 42;
```

This declares a mutable variable.

We can also specify the type of the variable explicitly:

```
let foo: int8 = 42;
```

## Assignment

To reassign a new value to a mutable variable:

```
foo = 0;
```

If the variable we're trying to reassign wasn't declared as mutable, we get a
compile error:

```
main.delta:3:9: error: cannot assign to immutable variable 'foo'
    foo = 0;
        ^
```
