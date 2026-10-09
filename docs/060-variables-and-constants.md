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
expression (literals, arithmetic on constants, references to other
constants, integer-to-pointer casts of integer constants, and constructor
calls of copyable structs with constant arguments, but not other function
calls), and the constant cannot be
reassigned or mutated afterwards. Reading or copying a constant's value
always works, but no alias to it can be formed: taking its address and
binding it to a borrow, pointer, or view are all rejected.

The same rule applies to method calls. Calling a method that may mutate its
receiver (the compiler infers this from the method body, transitively) on a
constant is rejected, as is reinitializing a constant with `init`. Read-only
methods keep working, including `size`, element reads, printing, and
iteration. Lazily evaluated calls (`map`, `filter`, `iterator`) are rejected
on constants unless given an inline lambda, whose borrows bind constant.
Storing a lazily evaluated passthrough iterator (such as a filter) over a
constant in a variable is rejected too, since later uses would traverse it
as mutable; `map` results are exempt because each output is materialized
into the iterator itself. The same goes for passing such an iterator to a
function that may write through it or return it, and for returning one.
Binding a borrow produced anywhere along a lazy chain over a constant is
rejected as well. Use `toList` to collect a fresh, mutable copy first; reads
such as printing a filter keep working, with or without collecting.
A variable initialized from a constant with a view type (such as a string
or slice member) becomes constant itself: reads keep working but mutating
calls on it are rejected, like for-loop elements over a constant range.
Passing such a view to a function that may write through it is rejected
like a passthrough iterator, and reassigning one over a constant is
rejected outright, since reassignment cannot mirror constness. The same
goes for returning one, and for constructing a value that stores it;
constructors that copy keep working.

`const` only appears at the start of a constant declaration; it never
appears in types. There is no `const T*`: pointers, borrows, and views
are always mutable, and `const`-qualified C types import as their mutable
counterparts.

A global of explicit slice type cannot be initialized with an array
literal; omit the type to infer a fixed-size array instead. Null and
empty slice globals work as usual.

Global variables are initialized before the program starts,
so their initializers must be constant expressions too.
A global always needs an initializer; without one it is an error, since
no storage would be emitted for it.

```cs
struct Color {
    int r
    int g
    int b
}

const threshold = 10;
int limit = threshold * 2;
string greeting = "hello";
const white = Color(255, 255, 255);

void main() {
    println(limit); // prints 20
    println(greeting); // prints hello
    println(white.g); // prints 255
}
```

Names starting with `_` suppress the unused-variable warning,
for placeholders that are declared but deliberately never read.
Top-level functions use the same prefix to suppress the
unused-declaration warning.

## Reserved words

The following words are reserved and cannot be used as identifiers
(for variables, parameters, functions, or types):

`break`, `case`, `const`, `continue`, `default`, `defer`, `do`, `else`,
`enum`, `extern`, `false`, `for`, `if`, `import`, `in`, `interface`,
`null`, `private`, `public`, `return`, `sizeof`, `struct`, `switch`,
`then`, `this`, `true`, `undefined`, `var`, `while`

## Planned features

- Compile-time-evaluable functions, so constant initializers can call functions.
