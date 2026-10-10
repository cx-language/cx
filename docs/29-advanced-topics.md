# Advanced topics

Most programs never need the chapters below. They cover niche corners
of the language: grouping values without a declaration, overlapping
storage, callbacks, and custom operators. Read the everyday chapters
first, and come back here when you need one of these.

- [Anonymous structs](./anonymous-structs) group a fixed set of named
  fields into a single value without declaring a named struct.
- [Unions](./unions) overlap members in storage, for type punning and
  matching C unions imported from headers.
- [Function pointers](./function-pointers) store functions in values
  and pass callbacks, including C callbacks.
- [Operator overloading](./operator-overloading) defines operators
  for user-defined types.
- [Type aliases](./type-aliases) give short names to long types.
- [Private](./private) restricts declarations to their module.
- [Namespaces](./namespaces) scope a file's declarations under one name.
- [Iterators](./iterators) define how collections work with `for` loops.
- [Interfaces](./interfaces) declare behavior shared across types.
- [Generics](./generics) parameterize functions and types over other types.
- [`undefined`](./undefined) explicitly marks a variable as uninitialized.
- [Context](./context) carries implicit settings like the allocator.
- [Arena](./arena) bump-allocates short-lived groups of values.
- [Defer and destructors](./destructors) run statements and release resources at scope exit.
- [Assertions](./assertions) check conditions that must hold.
