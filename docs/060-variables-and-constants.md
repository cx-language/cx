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

`const` declares a compile-time constant: the initializer must be a constant
expression (literals, arithmetic on constants, and references to other
constants, but not function calls), and the constant cannot be reassigned
or mutated afterwards. Reading or copying a constant's value always works,
but no alias to it can be formed: taking its address and binding it to a
borrow, pointer, or view are all rejected.

`const` only appears at the start of a constant declaration; it never
appears in types. There is no `const T*`: pointers, borrows, and views
are always mutable, and `const`-qualified C types import as their mutable
counterparts.

A global of explicit slice type cannot be initialized with an array
literal; omit the type to infer a fixed-size array instead. Null and
empty slice globals work as usual.

Global variables are initialized before the program starts,
so their initializers must be constant expressions too.

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
