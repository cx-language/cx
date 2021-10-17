# Pointers

Pointers, written `T*` (where `T` is the pointed-to type), are used to refer to other objects in memory.

C* pointers are mostly like C/C++ pointers, with the following differences:
- They cannot be null by default.
  To create nullable pointers, they need to be marked as [nullable](nullable-types.html).
- They don't support pointer arithmetic. For pointer arithmetic, array pointers have to be used, see below.
- When passing a `T*` where a `T` is expected, the pointer is automatically dereferenced. 
- When passing a `T` value where a `T*` is expected, the address is automatically passed.

To form a pointer, the `&` operator can be used.
To dereference a pointer, the `*` operator can be used.
Comparing pointers compares the stored memory addresses.
To compare the pointed values, the pointers need to be dereferenced first.

```cs
void main() {
    int i = 6;
    int* p = &i; // Make p point to i
    println(p); // Prints 6

    int* q = p; // Copy p to q, both point to i now
    println(p == q); // Prints true because both pointers point to the same memory address

    int j = 6;
    p = &j; // Change p to point to j
    println(p == q); // Prints false because pointers point to different memory addresses

    *p = 7; // Change the value of j
    println(*p + *q); // Sums the values pointed to by p and q, prints 13
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
