# Arrays

In Delta, the `Array` type is a dynamic array (like `std::vector` in C++) that
resizes dynamically and supports fast insertion to the end. It also contains
useful functions, some of which are listed here along with sample use cases.

### Access and bounds

Arrays are accessed using the subscript operator `[]`, and implement bounds
checking to avoid index-out-of-bounds bugs. The subscript returns a pointer to
the array.

```swift
var a = Array<int>();
let error = a[1];
// => Array index 1 is out of bounds, size is 0
```

We can also access elements using the short-hand functions `first` and `last`:

```swift
var a = Array([1, 2, 3]);
printf("%d", a.first());
// => 1
printf("%d", a.last());
// => 3
```

Elements can be added with the `push` function.

```swift
var a = Array<int>();
a.push(7);
```

Elements can be removed with `removeLast`, `removeAt` or `removeFirst`.

Arrays can also be compared with each other, elementwise, with the `==`
comparison operator


```swift
let a = Array([1, 2, 3]);
let b = Array([1, 2, 3]);
a == b; // true

let a = Array([1]);
let b = Array([2]);
a == b; // false
```

### Iteration

We can iterate over arrays with the for-each style loop

```swift
var a = Array([1, 2, 3]);
for (let e in a) {
    // ...
}
```

### Map and filter

We can use the functional `map` and `filter` operations on Arrays, using
function pointers or lambda expressions. First, let's see how filter works.

```swift
var a = Array([1, 2, 3]);
let filtered = a.filter((e: int*) -> *e > 2);
printf("%d", *filtered[0]);
// => 3
```

Note that the element `e` in the lambda expression is a pointer, and as such
has to be dereferenced to get the value.

`map` works similarly, except it returns an Array of the same size.

```swift
var a = Array([1, 2, 3]);
let b = a.map((e: int*) -> *e * 2);
printf("%d, %d, %d", *b[0], *b[1], *b[2]);
// => 2, 4, 6
```
