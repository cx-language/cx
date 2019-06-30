# Lists

In Delta, the `List` type is a dynamic array (like `std::vector` in C++) that
resizes dynamically and supports fast insertion to the end. It also contains
useful functions, some of which are listed here along with sample use cases.

### Access and bounds

Lists are accessed using the subscript operator `[]`, and implement bounds
checking to avoid index-out-of-bounds bugs. The subscript returns a pointer to
the element.

```
var a = List<int>();
var error = a[1];
// => List index 1 is out of bounds, size is 0
```

We can also access elements using the short-hand functions `first` and `last`:

```
var a = List([1, 2, 3]);
printf("%d", a.first());
// => 1
printf("%d", a.last());
// => 3
```

Elements can be added with the `push` function.

```
var a = List<int>();
a.push(7);
```

Elements can be removed with `removeLast`, `removeAt` or `removeFirst`.

Lists can also be compared with each other, elementwise, with the `==`
comparison operator


```
var a = List([1, 2, 3]);
var b = List([1, 2, 3]);
a == b; // true

var a = List([1]);
var b = List([2]);
a == b; // false
```

### Iteration

We can iterate over lists with the for-each style loop

```
var a = List([1, 2, 3]);
for (var e in a) {
    // ...
}
```

### Map and filter

We can use the functional `map` and `filter` operations on lists, using
function pointers or lambda expressions. First, let's see how filter works.

```
var a = List([1, 2, 3]);
var filtered = a.filter((e: int*) -> *e > 2);
printf("%d", *filtered[0]);
// => 3
```

Note that the element `e` in the lambda expression is a pointer, and as such
has to be dereferenced to get the value.

`map` works similarly, except it returns a list of the same size.

```
var a = List([1, 2, 3]);
var b = a.map((e: int*) -> *e * 2);
printf("%d, %d, %d", *b[0], *b[1], *b[2]);
// => 2, 4, 6
```
