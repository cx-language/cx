# List

`List` is a resizable array that grows automatically when elements are inserted.
It works like `std::vector` in C++ or `List` in C#.

Like builtin arrays, `List` elements can be accessed using the index operator `[]`,
and iterated over with a `for` loop.

Unlike in C++, the index operator does bounds checks in debug builds.

```cs
void main() {
    // Create an empty list of integers.
    var list = List<int>();

    // Add some numbers to the list.
    list.push(1);
    list.push(2);
    list.push(3);

    println(list[0]); // prints the first element, 1
    println(list.first()); // also prints the first element
    println(list.last()); // prints the last element, 3
    println(list[-1]); // also prints the last element
    // println(list[3]); // invalid index, this would cause an assertion error

    // Print each element in the list.
    for element in list {
        println(element);
    }
}
```

## Slice

The normal way to pass an array or `List` into a function is by slice,
written `T[]` where `T` is the element type.
Under the hood, a slice is a pointer to the array and a size.
It is known in other languages by the name slice, span, or array view.

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
    println(calculateSum(List([4, 5, 6])));
}
```

## Map, filter

We can use the functional `map` and `filter` operations on lists, with
function pointers or lambda expressions.
They are lazy: they return iterators that compute elements on demand,
so chained operations only traverse the list once.
Collect the results into a list with `toList()`.

```cs
bool isEven(int& n) {
    return n % 2 == 0;
}

void main() {
    var numbers = List([0, 1, 2, 3, 4]);

    var even = numbers.filter(isEven).toList();
    println(even); // prints [0, 2, 4]

    var doubled = even.map(n => n * 2).toList();
    println(doubled); // prints [0, 4, 8]
}
```
