# Variables and constants

cx has variables, declared using `var` or an explicit type, and constants, declared using `const` (possibly followed by an explicit type).

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

Global variables are initialized before the program starts,
so their initializers must be constant expressions:
literals, arithmetic on constants, and references to other constants,
but not function calls.

```cs
const threshold = 10;
int limit = threshold * 2;
string greeting = "hello";

void main() {
    println(limit); // prints 20
    println(greeting); // prints hello
}
```

Names starting with `_` suppress the unused-variable warning,
for placeholders that are declared but deliberately never read.

## Reserved words

The following words are reserved and cannot be used as identifiers
(for variables, parameters, functions, or types):

`break`, `case`, `const`, `continue`, `default`, `defer`, `do`, `else`,
`enum`, `extern`, `false`, `for`, `if`, `import`, `in`, `interface`,
`null`, `private`, `public`, `return`, `sizeof`, `struct`, `switch`,
`then`, `this`, `true`, `undefined`, `var`, `while`
