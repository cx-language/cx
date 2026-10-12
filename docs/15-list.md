# List

`List` is a resizable array that grows automatically when elements are inserted.

The index operator does bounds checks in debug builds.

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

See [Slice](arrays#slice) for passing a `List` to a function.

## Map, filter

We can use the functional `map` and `filter` operations on lists, with
function pointers or lambda expressions.
They are lazy: they return iterators that compute elements on demand.
Collect the results into a list with `toList()`.
To just print the results, pass the iterator to `println` directly.

```cs
bool isEven(int n) {
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

## SmallList

`SmallList<T, N>` is a `List`-like container that stores up to `N` elements
inline, without heap allocation, and spills to the heap when it grows past that.
Use it when a list usually stays
small and you want to avoid the allocator in the common case.

```cs
void main() {
    var list = SmallList<int, 4>();
    list.push(1);
    list.push(2);
    println(list.size()); // prints 2, no heap allocation so far
}
```

The API mirrors `List` (`push`, `pop`, indexing, iteration,
`map`/`filter`/`find`, etc.).

<details>
<summary>SmallList layout and slice restriction</summary>

Elements stay contiguous in both modes - inline elements sit in a fixed
array, spilled ones in a heap buffer just like `List` - so `data()` hands
out a pointer to the first element. One difference: unlike `List`, a
`SmallList` cannot be passed as a slice (`T[]`).

</details>
