# Variables and constants

Delta has variables, declared using `var` or an explicit type, and constants, declared using `const` (possibly followed by an explicit type).

When using `var` or `const` without an explicit type, the type of the variable or constant will be inferred from the initializer.

```cs
void main() {
    int a = 1; // type is explicitly specified as 'int'
    var b = 2; // type is inferred as 'int'
    println(a + b); // prints '3'
    println(c + d); // prints '0.3'. Constants don't have to be declared before using them.
}

const c = 0.1; // global constant, type inferred as 'float'
const float d = 0.2; // global constant, explicit type
```
