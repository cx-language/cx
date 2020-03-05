# Pointers

Pointers in Delta, written `T*`, are almost like in C and C++, except that they can't be null by default.
To create nullable pointers, the pointer type has to be marked as [nullable](nullable-types.html).

To form a pointer, the `&` operator can be used.
To dereference a pointer, the `*` operator can be used.

Comparing pointers always compares to pointed-to objects. To compare the memory addresses,
the reference equality operators `===` and `!==` can be used.

```cs
void main() {
    int i = 6;
    int* p = &i; // Make p point to i
    println(p); // Prints 6

    int* q = p; // Copy p to q, both point to i now
    println(p === q); // Prints true because both pointers point to the same memory address

    int j = 6;
    p = &j; // Change p to point to j
    println(p === q); // Prints false because pointers point to different memory addresses

    *p = 7; // Change the value of j
    println(p * q); // Multiplies the values pointed to by p and q, prints 42
}
```

## Array pointers

Array pointers, written `T[*]`, are pointers that point to an array of unknown length, or one step past the end of an array.
They support pointer arithmetic.

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
