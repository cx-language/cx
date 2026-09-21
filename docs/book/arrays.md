# Arrays

A static array stores a fixed number of elements of the same type.
The size is written next to the element type: `int[3]` is an array of three integers.

```cs
void main() {
    int[3] numbers = [1, 2, 3];

    println(numbers[0]); // prints the first element, 1
    println(numbers.size()); // prints 3

    numbers[0] = 42;
    println(numbers[0]); // prints 42

    for number in numbers {
        println(number);
    }
}
```

Out-of-bounds accesses are caught: constant indices are checked at compile time,
and the index operator does bounds checks in debug builds.

`data()` returns a pointer to the first element, for passing the contents
to C functions or doing pointer arithmetic:

```cs
void main() {
    int[3] numbers = [1, 2, 3];
    int[*] p = numbers.data();
    println(p[0]); // prints 1
}
```

Arrays are values: assigning an array copies its elements.
To pass an array to a function without copying, take it by an array reference (`T[]`),
which refers to the caller's elements in place:

```cs
int sum(int[] numbers) {
    var total = 0;
    for number in numbers {
        total += number;
    }
    return total;
}

void main() {
    int[3] numbers = [1, 2, 3];
    println(sum(numbers)); // prints 6
    println(sum([4, 5])); // array literals work too, prints 9
}
```

See [List](list) for a resizable array, and [Pointers](pointers)
for array pointers and pointer arithmetic.
