# `undefined`

By default cx warns you when you use a variable before initializing it.
The `undefined` keyword allows you to explicitly mark a variable as uninitialized, suppressing the warning.
This is useful for example when a variable is initialized through an out parameter.

```cs
void init(int* value) {
    *value = 42;
}

void main() {
    int result = undefined;
    init(&result);
    println(result); // prints 42
}
```
