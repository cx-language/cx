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

`const` is a property of the declaration, not of the value's type: a constant
cannot be reassigned or mutated in place, but copying its value out always
works. Passing a constant where a value is expected (arguments, return
values, ternary arms, array elements, ...) never fails for constness
reasons. `const` only constrains aliases: pointers, borrows, and views
cannot drop the pointee's constness, and the elements of a `const` array
cannot be written through.

In a declaration, `const` before a pointer, borrow, or array-pointer type
qualifies the pointee instead of the binding, like C++: `const int* p`
can be reseated, but `*p` cannot be written through. Any other `const`
(including `const int[]` and `const int[3]`) makes the binding itself
immutable.

An explicit type on a global or member constant cannot be a slice or
optional type; omit it and let the type be inferred.

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
