# Low-level programming

## `undefined`

By default Delta warns you when you use a variable before initializing it.
The `undefined` keyword allows you to explicitly mark a variable as uninitialized, suppressing the warning.
This is useful for example when you need to pass a variable to a C function to be initialized as an out parameter.

```cs
void main() {
    char[3] result = undefined;
    sprintf(result, "%d", 42);
    // result is ['4', '2', '\0']
}
```

## Using C libraries

TODO
