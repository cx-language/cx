# Arrays

A static array stores a fixed number of elements of the same type.
The size is written next to the element type: `int[3]` is an array of three integers.
The size can be any constant integer expression, such as `uint8[64 * 64 * 4]`.
Top-level named constants work too, including member access on them, so
`const res = [800, 600];` allows `uint8[res.x * res.y * 3]`. Sizes resolve
in their own scope: inside a function, local constants shadow globals,
and generic signatures see the callee's globals and generic parameters.

The size may also be `sizeof(T)`, which folds once the operand type is known.
Generic code uses this to reserve scratch space for a value of unknown type,
as in `uint8[sizeof(E)]`. Such arrays cannot hold owning elements or be
initialized from literals, since their size is only known to the backends.
The buffer is aligned as `uint8`. Scratch that must hold an `E` with `E`'s
alignment is a local `@manuallyDestroy E` (see [Structs](structs)).

```cs
void main() {
    int[3] numbers = [1, 2, 3];

    println(numbers[0]); // prints the first element, 1
    println(numbers.size()); // prints 3
    println(numbers[-1]); // prints the last element, 3

    numbers[0] = 42;
    println(numbers[0]); // prints 42

    for number in numbers {
        println(number);
    }
}
```

Out-of-bounds accesses are caught: constant indices are checked at compile time,
and the index operator does bounds checks in debug builds.

A minus inside the brackets indexes from the end with no runtime overhead:
`numbers[-1]` is the last element and `numbers[-numbers.size()]` is the first.

`data()` returns a pointer to the first element, for passing the contents
to C functions or doing pointer arithmetic:

```cs
void main() {
    int[3] numbers = [1, 2, 3];
    int[*] p = numbers.data();
    println(p[0]); // prints 1
}
```

Array literals must contain exactly as many elements as the declared array
size. Calling `data()` on a constant array is rejected: the pointer would
alias frozen storage. Read constant elements by index instead.

Arrays are values: assigning an array copies its elements.
To pass an array to a function without copying, take it by a slice view (`T[]`),
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

## Slice

A slice, written `T[]` where `T` is the element type, refers to array
elements in place. Under the hood, a slice is a pointer to the array
and a size.

```cs
// Function to calculate the sum of an array's elements.
int calculateSum(int[] array) {
    var sum = 0;

    for element in array {
        sum += element;
    }

    return sum;
}

void main() {
    println(calculateSum([1, 2, 3]));
    // A List converts to a slice view implicitly.
    var list = List([4, 5, 6]);
    println(calculateSum(list));
}
```

A list can also be built from an array: `List` moves the elements out,
so an owning array passed by name is consumed. Copyable arrays are copied
and stay usable.

See [Vector math](vector-math) for element-wise arithmetic, vector
operations, and swizzles. See [List](list) for a resizable array, and
[References and pointers](references#array-pointers) for array pointers
and pointer arithmetic.
