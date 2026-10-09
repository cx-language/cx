# Pointers

Pointers, written `T*` (where `T` is the pointed-to type), are used to refer to other objects in memory.

cx pointers are mostly like C/C++ pointers, with the following differences:

- They cannot be null by default.
  To create nullable pointers, they need to be marked as [nullable](nullable-types).
- They don't support pointer arithmetic. For pointer arithmetic, array pointers have to be used, see below.
- There is no implicit conversion from a pointer to a value: dereference it with `*p`
  where a value is expected. Member access and indexing remain direct through a pointer.
- Forming one from a value needs an explicit `&`.
  Borrow parameters (`T&`) borrow values implicitly; they do not form a stored `T*`.

To form a pointer, the `&` operator is used. Its operand must be an lvalue or a borrow,
so the address of a returned reference can be taken directly (e.g. `&list.first()`).
To dereference a pointer, the `*` operator is used.
Comparing two pointers compares the stored memory addresses.
To compare pointed-to values, dereference the pointers first.

```cs
void main() {
    int i = 6;
    int* p = &i; // Make p point to i
    println(*p); // Prints 6

    int* q = p; // Copy p to q, both point to i now
    println(p == q); // Prints true because both pointers point to the same memory address

    int j = 6;
    p = &j; // Change p to point to j
    println(p == q); // Prints false because pointers point to different memory addresses
    println(*p == 6); // Prints true because the pointed-to value is compared

    *p = 7; // Change the value of j
    println(*p + *q); // Sums the values pointed to by p and q, prints 13
}
```

## Const in C APIs

cx has no const-qualified pointees: `const` never appears in types. C and
C++ `const` is dropped when declarations are imported, so a C `const char*`
becomes plain `char*`. Passing a mutable pointer where C expects a const
one is safe (C only reads through it); writing through a pointer that C
considers const is undefined behavior, and the compiler cannot check it.
Named constants still use the `const` declarator (`const x = ...`).

## Borrowed parameters

Functions that use a value for the duration of the call without replacing it take it by borrow, written `T&`.
(The `T&` type is also called a reference; compiler diagnostics say "reference type".)
Callers pass values as usual; the compiler borrows them automatically.
Temporaries and literals can be borrowed too; the borrow refers
to a temporary that lives until the end of the call. Constants cannot be
borrowed: they are compile-time values with no address, so bind them to a
local first if a borrow is needed. Computed values derived from constants
(e.g. `c + 1`) are temporaries and borrow normally, as do const operands
of operator overloads.
Passing a stored `T*` where a `T&` is expected is an error; dereference it
explicitly (`*p`) to borrow the pointee.
Inside the function, member access and operators use the borrowed value directly; `*` moves a value out explicitly.
Mutating fields or elements through a borrowed parameter is fine, but reassigning the
whole value (`*p = v`) warns: take `T*` instead, so the `&` at the call site shows
that the argument is modified.
Member functions use a `T&` borrow for `this` (writing its fields is fine); use `&this` to obtain a storable `T*`.

Unlike pointers, borrows cannot be stored in fields or globals: besides function parameters,
return types, and interface arguments, `T&` may only appear as a local variable type. A reference
local aliases its referent in place; it must be initialized with an lvalue, since a temporary
would dangle. Reading a borrow into an inferred `var` keeps the borrow: `var r = list[0]`
deduces `Element&` and aliases the element, so `*r = v` writes it back. Name an explicit
value type (`int r = p;`) to copy the value out instead.
Unlike pointers, borrows cannot be reseated by assignment either: assigning to a borrow would rebind
it, so the compiler rejects it. A local borrow is written through with `*`;
writing through a borrowed parameter this way warns (take `T*` instead).

```cs
void main() {
    var i = 41;
    int& r = i;
    *r += 1;
    println(i); // prints 42
    println(r); // prints 42
}
```

Borrows are memory-safe by default: the compiler rejects code where a borrow could
dangle, with no lifetime annotations. A function cannot return a borrow, slice,
string, or iterator derived from a local variable, a by-value parameter, or a
temporary; only borrows of parameters, `this`, and globals may escape. While a view
is alive (from its declaration to its last use), its owner is frozen: reassigning,
moving, destroying, or mutating it is an error, as is mutating a collection while
iterating it. Within a method, freezing is per-field: assigning one field while a
view of another is alive is fine. Writing an element in place (`list[i] = v`) stays
legal. A borrow of the owner itself (`List<int>& a = list`) only conflicts with
moves and destruction: interior mutation leaves the borrowed slot in place. Raw
pointers are exempt from all of this: they are the explicit unsafe hatch for when
the checker is wrong or in the way.

```cs
void main() {
    var list = List([1, 2, 3]);
    int& r = list[0];
    println(r); // last use of r ends the borrow
    list.push(4); // fine: no view of list is alive anymore
}
```

A borrow can be nullable, written `T&?`, for parameters that may or may not receive a value.
It accepts everything a `T&` accepts, plus `null`.
Inside the function, test the parameter before dereferencing it.
Nullable borrows also work as local variable types, both inferred
(`var count = counts[key];` deduces `int&?`) and explicit (`int&? r = ...`).

```cs
void bump(int* x) {
    *x += 1;
}

int listSize(List<int>& list) {
    return list.size();
}

void main() {
    var i = 41;
    bump(&i);
    println(i); // prints 42

    var list = List([1, 2, 3]);
    println(listSize(list)); // prints 3
    bump(&list[0]);
    println(list[0]); // prints 2
}
```

```cs
int getOrDefault(int&? o, int fallback) {
    if o {
        return *o;
    }
    return fallback;
}

void main() {
    var i = 41;
    println(getOrDefault(i, 0)); // prints 41
    println(getOrDefault(null, 0)); // prints 0
}
```

## Array pointers

Array pointers, written `T[*]`, are pointers that point to an array of unknown length, or one step past the end of an array.
They are the only pointer type that supports pointer arithmetic, since pointer arithmetic only makes sense for arrays.

```cs
void main() {
    int[3] a = [1, 2, 3];

    int[*] p = a; // p points to a
    println(p[0]); // prints 1

    p++; // increment pointer to next element
    println(p[0]); // prints 2

    p = &p[1]; // increment pointer to next element
    println(*p); // *p is equivalent to p[0], prints 3
}
```

## Planned features

- Declaring pointers non-aliasing, either individually or globally with a compiler flag,
  to give the optimizer more freedom.
