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

To form a pointer, the `&` operator is used. Its operand must be an lvalue.
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

## Borrowed parameters

Functions that only use a value for the duration of the call take it by borrow, written `T&`.
(The `T&` type is also called a reference; compiler diagnostics say "reference type".)
Callers pass values as usual; the compiler borrows them automatically.
Temporaries and literals can be borrowed too; they live until the end of the call.
Passing a stored `T*` where a `T&` is expected reborrows it.
Inside the function, member access and operators use the borrowed value directly; `*` writes through the borrow or moves a value out explicitly.
Member functions use a `T&` borrow for `this`; use `&this` to obtain a storable `T*`.

Unlike pointers, borrows cannot be stored: `T&` may only appear as a function parameter type,
and reading a borrow into a variable copies the value out.

A borrow can be nullable, written `T&?`, for parameters that may or may not receive a value.
It accepts everything a `T&` accepts, plus `null` and nullable pointers.
Inside the function, test the parameter before dereferencing it.

```cs
void bump(int& x) {
    *x += 1;
}

int listSize(List<int>& list) {
    return list.size();
}

void main() {
    var i = 41;
    bump(i);
    println(i); // prints 42

    var list = List([1, 2, 3]);
    println(listSize(list)); // prints 3
    bump(list[0]);
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
