# Standard library reference

Auto-generated from the [standard library sources](https://github.com/emillaine/cx/tree/main/std)
by [generate_std_docs.py](https://github.com/emillaine/cx/blob/main/docs/generate_std_docs.py).
Do not edit by hand.

**Types:** [`ArrayIterator`](#type-ArrayIterator), [`ArrayRef`](#type-ArrayRef), [`AVLnode`](#type-AVLnode), [`bool`](#type-bool), [`Box`](#type-Box), [`ByteIterator`](#type-ByteIterator), [`char`](#type-char), [`ClosedRange`](#type-ClosedRange), [`ClosedRangeIterator`](#type-ClosedRangeIterator), [`Comparable`](#type-Comparable), [`Copyable`](#type-Copyable), [`EnumeratedIterator`](#type-EnumeratedIterator), [`EnumeratedIteratorEntry`](#type-EnumeratedIteratorEntry), [`FILE`](#type-FILE), [`float`](#type-float), [`float16`](#type-float16), [`float32`](#type-float32), [`float64`](#type-float64), [`float80`](#type-float80), [`Hashable`](#type-Hashable), [`int`](#type-int), [`int128`](#type-int128), [`int16`](#type-int16), [`int32`](#type-int32), [`int64`](#type-int64), [`int8`](#type-int8), [`Iterator`](#type-Iterator), [`LineIterator`](#type-LineIterator), [`List`](#type-List), [`Map`](#type-Map), [`MapEntry`](#type-MapEntry), [`MapIterator`](#type-MapIterator), [`never`](#type-never), [`Node`](#type-Node), [`Optional`](#type-Optional), [`OrderedMap`](#type-OrderedMap), [`OrderedMapIterator`](#type-OrderedMapIterator), [`OrderedSet`](#type-OrderedSet), [`OrderedSetIterator`](#type-OrderedSetIterator), [`Ordering`](#type-Ordering), [`OutputFileStream`](#type-OutputFileStream), [`Printable`](#type-Printable), [`Queue`](#type-Queue), [`Range`](#type-Range), [`RangeIterator`](#type-RangeIterator), [`Set`](#type-Set), [`SetIterator`](#type-SetIterator), [`string`](#type-string), [`StringBuffer`](#type-StringBuffer), [`StringIterator`](#type-StringIterator), [`uint`](#type-uint), [`uint128`](#type-uint128), [`uint16`](#type-uint16), [`uint32`](#type-uint32), [`uint64`](#type-uint64), [`uint8`](#type-uint8)

**Functions:** [`_fdopen`](#fn-_fdopen), [`abort`](#fn-abort), [`abortWrapper`](#fn-abortWrapper), [`abs`](#fn-abs), [`all`](#fn-all), [`allocate`](#fn-allocate), [`allocateArray`](#fn-allocateArray), [`any`](#fn-any), [`assertFail`](#fn-assertFail), [`clamp`](#fn-clamp), [`convertHash`](#fn-convertHash), [`deallocate`](#fn-deallocate), [`exit`](#fn-exit), [`fclose`](#fn-fclose), [`fdopen`](#fn-fdopen), [`feof`](#fn-feof), [`fflush`](#fn-fflush), [`fgetc`](#fn-fgetc), [`fopen`](#fn-fopen), [`fprintf`](#fn-fprintf), [`fputc`](#fn-fputc), [`fputs`](#fn-fputs), [`fread`](#fn-fread), [`free`](#fn-free), [`fseek`](#fn-fseek), [`ftell`](#fn-ftell), [`fwrite`](#fn-fwrite), [`getchar`](#fn-getchar), [`isalnum`](#fn-isalnum), [`isalpha`](#fn-isalpha), [`isblank`](#fn-isblank), [`iscntrl`](#fn-iscntrl), [`isdigit`](#fn-isdigit), [`isgraph`](#fn-isgraph), [`islower`](#fn-islower), [`isprint`](#fn-isprint), [`ispunct`](#fn-ispunct), [`isspace`](#fn-isspace), [`isupper`](#fn-isupper), [`isxdigit`](#fn-isxdigit), [`malloc`](#fn-malloc), [`max`](#fn-max), [`min`](#fn-min), [`none`](#fn-none), [`operator!=`](#fn-operator-ne), [`operator+`](#fn-operator-plus), [`operator<`](#fn-operator-lt), [`operator<=`](#fn-operator-le), [`operator==`](#fn-operator-eq), [`operator>`](#fn-operator-gt), [`operator>=`](#fn-operator-ge), [`print`](#fn-print), [`printf`](#fn-printf), [`println`](#fn-println), [`puts`](#fn-puts), [`readFile`](#fn-readFile), [`readLine`](#fn-readLine), [`remove`](#fn-remove), [`rewind`](#fn-rewind), [`safeAllocate`](#fn-safeAllocate), [`safeAllocateArray`](#fn-safeAllocateArray), [`sort`](#fn-sort), [`sprintf`](#fn-sprintf), [`stderr`](#fn-stderr), [`stdout`](#fn-stdout), [`strlen`](#fn-strlen), [`swap`](#fn-swap), [`tolower`](#fn-tolower), [`toupper`](#fn-toupper), [`ungetc`](#fn-ungetc), [`writeFile`](#fn-writeFile)

**Constants:** [`SEEK_CUR`](#const-SEEK_CUR), [`SEEK_END`](#const-SEEK_END), [`SEEK_SET`](#const-SEEK_SET), [`int16_max`](#const-int16_max), [`int16_min`](#const-int16_min), [`int32_max`](#const-int32_max), [`int32_min`](#const-int32_min), [`int64_max`](#const-int64_max), [`int64_min`](#const-int64_min), [`int8_max`](#const-int8_max), [`int8_min`](#const-int8_min), [`int_max`](#const-int_max), [`int_min`](#const-int_min), [`uint16_max`](#const-uint16_max), [`uint16_min`](#const-uint16_min), [`uint32_max`](#const-uint32_max), [`uint32_min`](#const-uint32_min), [`uint64_max`](#const-uint64_max), [`uint64_min`](#const-uint64_min), [`uint8_max`](#const-uint8_max), [`uint8_min`](#const-uint8_min), [`uint_max`](#const-uint_max), [`uint_min`](#const-uint_min)

## `struct ArrayIterator<Element>: Copyable, Iterator<Element*>` {#type-ArrayIterator}

Source: [ArrayIterator.cx](https://github.com/emillaine/cx/blob/main/std/ArrayIterator.cx)

### `ArrayIterator` {#ArrayIterator-ArrayIterator}

`ArrayIterator(Element[] array)`

### `current` {#ArrayIterator-current}

`Element[*] current;`

### `end` {#ArrayIterator-end}

`Element[*] end;`

### `hasValue` {#ArrayIterator-hasValue}

`bool hasValue()`

### `increment` {#ArrayIterator-increment}

`void increment()`

### `value` {#ArrayIterator-value}

`Element* value()`

## `struct ArrayRef<Element>: Copyable` {#type-ArrayRef}

Source: [ArrayRef.cx](https://github.com/emillaine/cx/blob/main/std/ArrayRef.cx)

### `all` {#ArrayRef-all}

`bool all(bool(Element*) predicate)`

### `any` {#ArrayRef-any}

`bool any(bool(Element*) predicate)`

### `ArrayRef` {#ArrayRef-ArrayRef}

`ArrayRef()`

Initializes an ArrayRef referring to an empty array.

`ArrayRef(List<Element>* list)`

Initializes an ArrayRef referring to the elements of the given List.

`ArrayRef(Element* data, int size)`

Initializes an ArrayRef referring to an array of the given size located at the given memory
address.

### `data` {#ArrayRef-data}

`Element[*] data;`

`Element[*] data()`

### `empty` {#ArrayRef-empty}

`bool empty()`

### `find` {#ArrayRef-find}

`Element*? find(bool(Element*) predicate)`

### `front` {#ArrayRef-front}

`Element* front()`

Returns the first element in the array.

### `iterator` {#ArrayRef-iterator}

`ArrayIterator<Element> iterator()`

### `none` {#ArrayRef-none}

`bool none(bool(Element*) predicate)`

### `operator[]` {#ArrayRef-operator-index}

`Element* operator[](int index)`

Returns a reference to the element at the given index.

### `size` {#ArrayRef-size}

`int size;`

`int size()`

Returns the number of elements in the array.

## `struct AVLnode<Key, Value>` {#type-AVLnode}

One node in the AVL tree

Source: [OrderedMap.cx](https://github.com/emillaine/cx/blob/main/std/OrderedMap.cx)

### `AVLnode` {#AVLnode-AVLnode}

`AVLnode(Key key, AVLnode<Key, Value>*? parent, Value value)`

### `balance` {#AVLnode-balance}

`int balance;`

### `deallocate` {#AVLnode-deallocate}

`void deallocate()`

### `entry` {#AVLnode-entry}

`MapEntry<Key, Value> entry;`

### `left` {#AVLnode-left}

`AVLnode<Key, Value>*? left;`

### `parent` {#AVLnode-parent}

`AVLnode<Key, Value>*? parent;`

### `right` {#AVLnode-right}

`AVLnode<Key, Value>*? right;`

## `struct bool: Copyable, Comparable, Printable, Hashable` {#type-bool}

Source: [bool.cx](https://github.com/emillaine/cx/blob/main/std/bool.cx)

### `compare` {#bool-compare}

`Ordering compare(bool* other)`

### `hash` {#bool-hash}

`uint64 hash()`

### `print` {#bool-print}

`void print(StringBuffer* stream)`

## `struct Box<T>` {#type-Box}

Source: [Box.cx](https://github.com/emillaine/cx/blob/main/std/Box.cx)

### `Box` {#Box-Box}

`Box(T value)`

### `get` {#Box-get}

`T* get()`

### `pointer` {#Box-pointer}

`T* pointer;`

### `~Box` {#Box-dtor}

`~Box()`

## `struct ByteIterator: Copyable, Iterator<uint8>` {#type-ByteIterator}

Source: [ByteIterator.cx](https://github.com/emillaine/cx/blob/main/std/ByteIterator.cx)

### `ByteIterator` {#ByteIterator-ByteIterator}

`ByteIterator(string stream)`

### `hasValue` {#ByteIterator-hasValue}

`bool hasValue()`

### `increment` {#ByteIterator-increment}

`void increment()`

### `iterator` {#ByteIterator-iterator}

`ByteIterator iterator()`

### `stream` {#ByteIterator-stream}

`string stream;`

### `value` {#ByteIterator-value}

`uint8 value()`

## `struct char: Copyable, Comparable, Printable, Hashable` {#type-char}

Source: [char.cx](https://github.com/emillaine/cx/blob/main/std/char.cx)

### `compare` {#char-compare}

`Ordering compare(char* other)`

### `hash` {#char-hash}

`uint64 hash()`

### `isAlpha` {#char-isAlpha}

`bool isAlpha()`

### `isDigit` {#char-isDigit}

`bool isDigit()`

### `isWhitespace` {#char-isWhitespace}

`bool isWhitespace()`

### `print` {#char-print}

`void print(StringBuffer* stream)`

## `struct ClosedRange<T>: Copyable` {#type-ClosedRange}

Source: [ClosedRange.cx](https://github.com/emillaine/cx/blob/main/std/ClosedRange.cx)

### `all` {#ClosedRange-all}

`bool all(bool(T) predicate)`

### `any` {#ClosedRange-any}

`bool any(bool(T) predicate)`

### `ClosedRange` {#ClosedRange-ClosedRange}

`ClosedRange(T start, T end)`

`ClosedRange(T end)`

### `end` {#ClosedRange-end}

`T end;`

`T end()`

### `iterator` {#ClosedRange-iterator}

`ClosedRangeIterator<T> iterator()`

### `none` {#ClosedRange-none}

`bool none(bool(T) predicate)`

### `size` {#ClosedRange-size}

`T size()`

### `start` {#ClosedRange-start}

`T start;`

`T start()`

## `struct ClosedRangeIterator<T>: Copyable, Iterator<T>` {#type-ClosedRangeIterator}

Source: [ClosedRangeIterator.cx](https://github.com/emillaine/cx/blob/main/std/ClosedRangeIterator.cx)

### `ClosedRangeIterator` {#ClosedRangeIterator-ClosedRangeIterator}

`ClosedRangeIterator(ClosedRange<T> range)`

### `current` {#ClosedRangeIterator-current}

`T current;`

### `end` {#ClosedRangeIterator-end}

`T end;`

### `hasValue` {#ClosedRangeIterator-hasValue}

`bool hasValue()`

### `increment` {#ClosedRangeIterator-increment}

`void increment()`

### `value` {#ClosedRangeIterator-value}

`T value()`

## `interface Comparable` {#type-Comparable}

A type whose instances have an inherent sort-order, i.e. can be compared with the `<`, `<=`,
`>`, `>=` operators.

Source: [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx)

### `compare` {#Comparable-compare}

`Ordering compare(This* other);`

Returns `Ordering.Less` if `this` is less than `other`, `Ordering.Equal` if `this` is equal
to `other`, or `Ordering.Greater` if `this` is greater than `other`.

## `interface Copyable` {#type-Copyable}

Source: [Copyable.cx](https://github.com/emillaine/cx/blob/main/std/Copyable.cx)

## `struct EnumeratedIterator<Element>: Copyable, Iterator<EnumeratedIteratorEntry<Element>>` {#type-EnumeratedIterator}

Source: [EnumeratedIterator.cx](https://github.com/emillaine/cx/blob/main/std/EnumeratedIterator.cx)

### `base` {#EnumeratedIterator-base}

`ArrayIterator<Element> base;`

### `EnumeratedIterator` {#EnumeratedIterator-EnumeratedIterator}

`EnumeratedIterator(ArrayIterator<Element> iterator)`

### `hasValue` {#EnumeratedIterator-hasValue}

`bool hasValue()`

### `increment` {#EnumeratedIterator-increment}

`void increment()`

### `index` {#EnumeratedIterator-index}

`int index;`

### `value` {#EnumeratedIterator-value}

`EnumeratedIteratorEntry<Element> value()`

## `struct EnumeratedIteratorEntry<Value>: Copyable` {#type-EnumeratedIteratorEntry}

Source: [EnumeratedIterator.cx](https://github.com/emillaine/cx/blob/main/std/EnumeratedIterator.cx)

### `index` {#EnumeratedIteratorEntry-index}

`int index;`

### `value` {#EnumeratedIteratorEntry-value}

`Value* value;`

## `struct FILE` {#type-FILE}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

## `struct float: Copyable, Comparable, Printable` {#type-float}

Source: [floats.cx](https://github.com/emillaine/cx/blob/main/std/floats.cx)

### `compare` {#float-compare}

`Ordering compare(float* other)`

### `print` {#float-print}

`void print(StringBuffer* stream)`

## `struct float16: Copyable, Comparable, Printable` {#type-float16}

Source: [floats.cx](https://github.com/emillaine/cx/blob/main/std/floats.cx)

### `compare` {#float16-compare}

`Ordering compare(float16* other)`

### `print` {#float16-print}

`void print(StringBuffer* stream)`

## `struct float32: Copyable, Comparable, Printable` {#type-float32}

Source: [floats.cx](https://github.com/emillaine/cx/blob/main/std/floats.cx)

### `compare` {#float32-compare}

`Ordering compare(float32* other)`

### `print` {#float32-print}

`void print(StringBuffer* stream)`

## `struct float64: Copyable, Comparable, Printable` {#type-float64}

Source: [floats.cx](https://github.com/emillaine/cx/blob/main/std/floats.cx)

### `compare` {#float64-compare}

`Ordering compare(float64* other)`

### `print` {#float64-print}

`void print(StringBuffer* stream)`

## `struct float80: Copyable, Comparable` {#type-float80}

Source: [floats.cx](https://github.com/emillaine/cx/blob/main/std/floats.cx)

### `compare` {#float80-compare}

`Ordering compare(float80* other)`

## `interface Hashable` {#type-Hashable}

Source: [Hashable.cx](https://github.com/emillaine/cx/blob/main/std/Hashable.cx)

### `hash` {#Hashable-hash}

`uint64 hash();`

## `struct int: Copyable, Comparable, Printable, Hashable` {#type-int}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#int-compare}

`Ordering compare(int* other)`

### `hash` {#int-hash}

`uint64 hash()`

### `print` {#int-print}

`void print(StringBuffer* stream)`

## `struct int128: Copyable, Comparable` {#type-int128}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#int128-compare}

`Ordering compare(int128* other)`

## `struct int16: Copyable, Comparable, Printable, Hashable` {#type-int16}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#int16-compare}

`Ordering compare(int16* other)`

### `hash` {#int16-hash}

`uint64 hash()`

### `print` {#int16-print}

`void print(StringBuffer* stream)`

## `struct int32: Copyable, Comparable, Printable, Hashable` {#type-int32}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#int32-compare}

`Ordering compare(int32* other)`

### `hash` {#int32-hash}

`uint64 hash()`

### `print` {#int32-print}

`void print(StringBuffer* stream)`

## `struct int64: Copyable, Comparable, Printable, Hashable` {#type-int64}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#int64-compare}

`Ordering compare(int64* other)`

### `hash` {#int64-hash}

`uint64 hash()`

### `print` {#int64-print}

`void print(StringBuffer* stream)`

## `struct int8: Copyable, Comparable, Printable, Hashable` {#type-int8}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#int8-compare}

`Ordering compare(int8* other)`

### `hash` {#int8-hash}

`uint64 hash()`

### `print` {#int8-print}

`void print(StringBuffer* stream)`

## `interface Iterator<Element>` {#type-Iterator}

An iterator is an object used to traverse the elements of a sequence.

Source: [Iterator.cx](https://github.com/emillaine/cx/blob/main/std/Iterator.cx)

### `hasValue` {#Iterator-hasValue}

`bool hasValue();`

Returns true if there's at least one element remaining in the sequence, i.e. if calling
`value()` is allowed.

### `increment` {#Iterator-increment}

`void increment();`

Advances the iterator to the next element in the sequence. Calling this function is not
allowed if `hasValue()` returns false.

### `value` {#Iterator-value}

`Element value();`

Returns the next element from the sequence. Calling this function is not allowed if
`hasValue()` returns false.

## `struct LineIterator: Copyable, Iterator<string>` {#type-LineIterator}

Source: [LineIterator.cx](https://github.com/emillaine/cx/blob/main/std/LineIterator.cx)

### `end` {#LineIterator-end}

`int end;`

### `hasValue` {#LineIterator-hasValue}

`bool hasValue()`

### `increment` {#LineIterator-increment}

`void increment()`

### `iterator` {#LineIterator-iterator}

`LineIterator iterator()`

### `LineIterator` {#LineIterator-LineIterator}

`LineIterator(string stream)`

### `stream` {#LineIterator-stream}

`string stream;`

### `value` {#LineIterator-value}

`string value()`

## `struct List<Element>` {#type-List}

Source: [List.cx](https://github.com/emillaine/cx/blob/main/std/List.cx)

### `all` {#List-all}

`bool all(bool(Element*) predicate)`

### `any` {#List-any}

`bool any(bool(Element*) predicate)`

### `buffer` {#List-buffer}

`Element[*] buffer;`

### `capacity` {#List-capacity}

`int capacity;`

`int capacity()`

Returns the number of elements the list can store without allocating more memory.

### `data` {#List-data}

`Element[*] data()`

### `empty` {#List-empty}

`bool empty()`

Returns true if the list has no elements, otherwise false

### `enumerate` {#List-enumerate}

`EnumeratedIterator<Element> enumerate()`

### `filter` {#List-filter}

`List<Element> filter(bool(Element*) include)`

`List<Element> filter(bool(Element) include)`

### `find` {#List-find}

`Element*? find(bool(Element*) predicate)`

### `first` {#List-first}

`Element* first()`

### `iterator` {#List-iterator}

`ArrayIterator<Element> iterator()`

### `last` {#List-last}

`Element* last()`

### `List` {#List-List}

`List()`

Initializes an empty list.

`List(public int capacity)`

Initializes an empty list with pre-allocated capacity.

`List(int size, Element value)`

Initializes a list containing `size` copies of the given value.

`List(Element[] elements)`

Initializes an list containing the elements of the given array.

`List(public int uninitializedSize)`

Initializes the list to contain the given number of uninitialized elements.

`List<Output> map<Output>(Output(Element*) transform)`

`List<Output> map<Output>(Output(Element) transform)`

### `none` {#List-none}

`bool none(bool(Element*) predicate)`

### `operator[]` {#List-operator-index}

`Element* operator[](int index)`

Returns the element at the given index.

### `operator[]=` {#List-operator-index-assign}

`void operator[]=(int index, Element element)`

### `pop` {#List-pop}

`Element pop()`

Removes and returns the last element.

### `push` {#List-push}

`void push(Element element)`

Adds the given element to the end of the list.

### `removeAt` {#List-removeAt}

`void removeAt(int index)`

Removes the element at the given index from the list.
Elements following the removed element are moved towards the beginning of the list by one index.

### `removeFirst` {#List-removeFirst}

`void removeFirst()`

Removes the first element from the list.
Other elements are moved towards the beginning of the list by one index.

`void removeFirst(bool(Element*) shouldRemove)`

Removes the first element satisfying the given condition.

### `removeLast` {#List-removeLast}

`void removeLast()`

Removes the last element from the list.

### `reserve` {#List-reserve}

`void reserve(int minimumCapacity)`

Ensures that the capacity is large enough to store the given number of elements.

### `size` {#List-size}

`int size;`

`int size()`

Returns the number of elements in the list.

### `~List` {#List-dtor}

`~List()`

## `struct Map<Key: Hashable, Value>` {#type-Map}

Source: [Map.cx](https://github.com/emillaine/cx/blob/main/std/Map.cx)

### `capacity` {#Map-capacity}

`int capacity()`

### `contains` {#Map-contains}

`bool contains(Key* e)`

Checks if e is part of the map.

### `empty` {#Map-empty}

`bool empty()`

### `hashTable` {#Map-hashTable}

`List<List<MapEntry<Key, Value>>> hashTable;`

### `insert` {#Map-insert}

`void insert(Key key, Value value)`

Inserts an element into the map. If the element exists already, nothing is done.

### `iterator` {#Map-iterator}

`MapIterator<Key, Value> iterator()`

Iterate over the map

### `loadFactor` {#Map-loadFactor}

`float64 loadFactor()`

Returns the load factor for the map. This is used to increase the map size once
the load factor gets too big.

### `Map` {#Map-Map}

`Map()`

Initializes an empty map

### `operator[]` {#Map-operator-index}

`Value*? operator[](Key* e)`

### `remove` {#Map-remove}

`void remove(Key* e)`

Removes an element from the map, if it exists there.

### `resize` {#Map-resize}

`void resize()`

Resizes the map. This includes copying the old table into a new, bigger one

### `set` {#Map-set}

`void set(Key key, Value value)`

Inserts, or updates an existing value.

### `size` {#Map-size}

`int size;`

`int size()`

## `struct MapEntry<Key: Hashable, Value>` {#type-MapEntry}

Source: [Map.cx](https://github.com/emillaine/cx/blob/main/std/Map.cx)

### `key` {#MapEntry-key}

`Key key;`

### `value` {#MapEntry-value}

`Value value;`

## `struct MapIterator<Key, Value>: Copyable, Iterator<MapEntry<Key, Value>*>` {#type-MapIterator}

Source: [MapIterator.cx](https://github.com/emillaine/cx/blob/main/std/MapIterator.cx)

### `current` {#MapIterator-current}

`MapEntry<Key, Value>[*] current;`

### `hashTableIterator` {#MapIterator-hashTableIterator}

`ArrayIterator<List<MapEntry<Key, Value>>> hashTableIterator;`

### `hasValue` {#MapIterator-hasValue}

`bool hasValue()`

### `increment` {#MapIterator-increment}

`void increment()`

### `MapIterator` {#MapIterator-MapIterator}

`MapIterator(Map<Key, Value>* map)`

### `value` {#MapIterator-value}

`MapEntry<Key, Value>* value()`

## `struct never` {#type-never}

Placeholder type used as the return type for functions that do not return.

Source: [never.cx](https://github.com/emillaine/cx/blob/main/std/never.cx)

## `struct Node<T>` {#type-Node}

Source: [Queue.cx](https://github.com/emillaine/cx/blob/main/std/Queue.cx)

### `next` {#Node-next}

`Node<T>*? next;`

### `Node` {#Node-Node}

`Node(T val)`

### `value` {#Node-value}

`T value;`

## `struct Optional<T>` {#type-Optional}

Source: [Optional.cx](https://github.com/emillaine/cx/blob/main/std/Optional.cx)

### `hasValue` {#Optional-hasValue}

`bool hasValue;`

### `Optional` {#Optional-Optional}

`Optional()`

`Optional(T value)`

### `value` {#Optional-value}

`T value;`

## `struct OrderedMap<Key, Value>` {#type-OrderedMap}

An ordered key-value container that supports fast insertion, deletion and lookup

Source: [OrderedMap.cx](https://github.com/emillaine/cx/blob/main/std/OrderedMap.cx)

### `contains` {#OrderedMap-contains}

`bool contains(Key* key)`

Checks if the given element is in the Map. Returns true if it is, otherwise false

### `empty` {#OrderedMap-empty}

`bool empty()`

Returns true if the Map is empty, otherwise false

### `find` {#OrderedMap-find}

`MapEntry<Key, Value>*? find(Key* key)`

Returns the key-value pair corresponding to `key`, or null if it doesn't exist in the Map

### `findNode` {#OrderedMap-findNode}

`AVLnode<Key, Value>*? findNode(Key* key)`

Returns the node corresponding to `key`, or null if it doesn't exist in the Map

### `first` {#OrderedMap-first}

`Key*? first()`

Returns the smallest element in the Map, or null if the Map is empty.

### `firstNode` {#OrderedMap-firstNode}

`AVLnode<Key, Value>*? firstNode()`

Returns the node with the smallest key in the Map, or null if the Map is empty.

### `higherKey` {#OrderedMap-higherKey}

`Key*? higherKey(Key* key)`

Returns the key immediately higher than the one given,
or null, if `key` is the largest in the Map.

### `higherNode` {#OrderedMap-higherNode}

`AVLnode<Key, Value>*? higherNode(AVLnode<Key, Value>* node)`

Returns the node that contains the key immediately larger than the key of the given node,
or null, if the key of `node` is the largest value.

### `insert` {#OrderedMap-insert}

`bool insert(Key key, Value value)`

Inserts a key-value pair into the Map.

### `iterator` {#OrderedMap-iterator}

`OrderedMapIterator<Key, Value> iterator()`

Iterate over the map, in order

### `last` {#OrderedMap-last}

`Key*? last()`

Returns the largest element in the Map, or null if the map is empty.

### `lastNode` {#OrderedMap-lastNode}

`AVLnode<Key, Value>*? lastNode()`

Returns the node with the largest key in the Map, or null if the Map is empty.

### `lowerKey` {#OrderedMap-lowerKey}

`Key*? lowerKey(Key* key)`

Returns the key immediately smaller than the one given,
or null, if `key` is the smallest in the Map.

### `lowerNode` {#OrderedMap-lowerNode}

`AVLnode<Key, Value>*? lowerNode(AVLnode<Key, Value>* node)`

Returns the node that contains the key immediately smaller than the key of the given node,
or null, if the key of `node` is the smallest value.

### `operator[]` {#OrderedMap-operator-index}

`Value*? operator[](Key* key)`

Returns the value of the given key, if it exists in the Map. Otherwise, returns null.

### `OrderedMap` {#OrderedMap-OrderedMap}

`OrderedMap()`

### `remove` {#OrderedMap-remove}

`void remove(Key* key)`

Removes a key and it's associated value from the Map

### `root` {#OrderedMap-root}

`AVLnode<Key, Value>*? root;`

### `size` {#OrderedMap-size}

`int size;`

`int size()`

Returns the size of the Map

### `~OrderedMap` {#OrderedMap-dtor}

`~OrderedMap()`

## `struct OrderedMapIterator<Key, Value>: Copyable, Iterator<MapEntry<Key, Value>*>` {#type-OrderedMapIterator}

Source: [OrderedMapIterator.cx](https://github.com/emillaine/cx/blob/main/std/OrderedMapIterator.cx)

### `current` {#OrderedMapIterator-current}

`AVLnode<Key, Value>*? current;`

### `hasValue` {#OrderedMapIterator-hasValue}

`bool hasValue()`

### `increment` {#OrderedMapIterator-increment}

`void increment()`

### `map` {#OrderedMapIterator-map}

`OrderedMap<Key, Value>* map;`

### `OrderedMapIterator` {#OrderedMapIterator-OrderedMapIterator}

`OrderedMapIterator(OrderedMap<Key, Value>* map)`

### `value` {#OrderedMapIterator-value}

`MapEntry<Key, Value>* value()`

## `struct OrderedSet<Element>` {#type-OrderedSet}

An ordered Set that supports fast lookup, insertion and deletion.

Source: [OrderedSet.cx](https://github.com/emillaine/cx/blob/main/std/OrderedSet.cx)

### `contains` {#OrderedSet-contains}

`bool contains(Element* e)`

Returns true if the Set contains `e`, otherwise false.

### `empty` {#OrderedSet-empty}

`bool empty()`

Returns true if the Set is empty, otherwise false.

### `first` {#OrderedSet-first}

`Element*? first()`

Returns the smallest element in the Set, or `null` if the Set is empty.

### `higher` {#OrderedSet-higher}

`Element*? higher(Element* e)`

Returns the value immediately higher than `e`, or `null` if
`e` is the largest element in the Set.

### `insert` {#OrderedSet-insert}

`bool insert(Element e)`

Inserts an element into the Set.

### `iterator` {#OrderedSet-iterator}

`OrderedSetIterator<Element> iterator()`

Iterate over the map, in order

### `last` {#OrderedSet-last}

`Element*? last()`

Returns the largest element in the Set, or `null` if the Set is empty.

### `lower` {#OrderedSet-lower}

`Element*? lower(Element* e)`

Returns the value immediately lower than `e`, or `null` if
`e` is the smallest element in the Set.

### `map` {#OrderedSet-map}

`OrderedMap<Element, bool> map;`

### `OrderedSet` {#OrderedSet-OrderedSet}

`OrderedSet()`

Initiates an empty Set.

### `remove` {#OrderedSet-remove}

`void remove(Element* e)`

Removes an element from the Set, if it exists.

### `size` {#OrderedSet-size}

`int size()`

Returns the size of the Set.

## `struct OrderedSetIterator<Element>: Copyable, Iterator<Element*>` {#type-OrderedSetIterator}

Source: [OrderedSetIterator.cx](https://github.com/emillaine/cx/blob/main/std/OrderedSetIterator.cx)

### `hasValue` {#OrderedSetIterator-hasValue}

`bool hasValue()`

### `increment` {#OrderedSetIterator-increment}

`void increment()`

### `mapIter` {#OrderedSetIterator-mapIter}

`OrderedMapIterator<Element, bool> mapIter;`

### `OrderedSetIterator` {#OrderedSetIterator-OrderedSetIterator}

`OrderedSetIterator(OrderedSet<Element>* set)`

### `value` {#OrderedSetIterator-value}

`Element* value()`

## `enum Ordering` {#type-Ordering}

Source: [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx)

### `Equal` {#Ordering-Equal}

`Equal`

### `Greater` {#Ordering-Greater}

`Greater`

### `Less` {#Ordering-Less}

`Less`

## `struct OutputFileStream` {#type-OutputFileStream}

Source: [FileStream.cx](https://github.com/emillaine/cx/blob/main/std/FileStream.cx)

### `file` {#OutputFileStream-file}

`FILE*? file;`

### `OutputFileStream` {#OutputFileStream-OutputFileStream}

`OutputFileStream(public int fileDescriptor)`

### `write` {#OutputFileStream-write}

`void write<T: Printable>(T* value)`

### `~OutputFileStream` {#OutputFileStream-dtor}

`~OutputFileStream()`

## `interface Printable` {#type-Printable}

Source: [Printable.cx](https://github.com/emillaine/cx/blob/main/std/Printable.cx)

### `print` {#Printable-print}

`void print(StringBuffer* stream);`

### `toString` {#Printable-toString}

`StringBuffer toString()`

## `struct Queue<T>` {#type-Queue}

First-in-first-out data structure

Source: [Queue.cx](https://github.com/emillaine/cx/blob/main/std/Queue.cx)

### `empty` {#Queue-empty}

`bool empty()`

Check if queue is empty

### `first` {#Queue-first}

`T* first()`

Access element in the front of the queue

### `head` {#Queue-head}

`Node<T>*? head;`

### `pop` {#Queue-pop}

`T pop()`

Retrieve and remove head of the queue

### `push` {#Queue-push}

`void push(T value)`

Add to the back of the queue

### `Queue` {#Queue-Queue}

`Queue()`

Initialize an empty Queue

### `size` {#Queue-size}

`int size;`

`int size()`

Returns the number of elements in the queue

### `tail` {#Queue-tail}

`Node<T>*? tail;`

### `~Queue` {#Queue-dtor}

`~Queue()`

## `struct Range<T>: Copyable` {#type-Range}

Source: [Range.cx](https://github.com/emillaine/cx/blob/main/std/Range.cx)

### `all` {#Range-all}

`bool all(bool(T) predicate)`

### `any` {#Range-any}

`bool any(bool(T) predicate)`

### `end` {#Range-end}

`T end;`

`T end()`

### `iterator` {#Range-iterator}

`RangeIterator<T> iterator()`

### `none` {#Range-none}

`bool none(bool(T) predicate)`

### `Range` {#Range-Range}

`Range(T start, T end)`

`Range(T end)`

### `size` {#Range-size}

`T size()`

### `start` {#Range-start}

`T start;`

`T start()`

## `struct RangeIterator<T>: Copyable, Iterator<T>` {#type-RangeIterator}

Source: [RangeIterator.cx](https://github.com/emillaine/cx/blob/main/std/RangeIterator.cx)

### `current` {#RangeIterator-current}

`T current;`

### `end` {#RangeIterator-end}

`T end;`

### `hasValue` {#RangeIterator-hasValue}

`bool hasValue()`

### `increment` {#RangeIterator-increment}

`void increment()`

### `RangeIterator` {#RangeIterator-RangeIterator}

`RangeIterator(Range<T> range)`

### `value` {#RangeIterator-value}

`T value()`

## `struct Set<Key: Hashable>` {#type-Set}

Source: [Set.cx](https://github.com/emillaine/cx/blob/main/std/Set.cx)

### `contains` {#Set-contains}

`bool contains(Key* key)`

Checks if e is part of the set.

### `empty` {#Set-empty}

`bool empty()`

### `insert` {#Set-insert}

`void insert(Key key)`

Inserts an element into the set. If the element exists already, nothing is done.

### `iterator` {#Set-iterator}

`SetIterator<Key> iterator()`

### `map` {#Set-map}

`Map<Key, bool> map;`

### `remove` {#Set-remove}

`void remove(Key* key)`

Removes an element from the set, if it exists there.

### `Set` {#Set-Set}

`Set()`

Initializes an empty set

### `size` {#Set-size}

`int size()`

## `struct SetIterator<Element>: Copyable, Iterator<Element*>` {#type-SetIterator}

Source: [SetIterator.cx](https://github.com/emillaine/cx/blob/main/std/SetIterator.cx)

### `hasValue` {#SetIterator-hasValue}

`bool hasValue()`

### `increment` {#SetIterator-increment}

`void increment()`

### `iterator` {#SetIterator-iterator}

`MapIterator<Element, bool> iterator;`

### `SetIterator` {#SetIterator-SetIterator}

`SetIterator(Set<Element>*  set)`

### `value` {#SetIterator-value}

`Element* value()`

## `struct string: Copyable, Comparable, Hashable, Printable` {#type-string}

Source: [string.cx](https://github.com/emillaine/cx/blob/main/std/string.cx)

### `all` {#string-all}

`bool all(bool(char) predicate)`

### `any` {#string-any}

`bool any(bool(char) predicate)`

### `bytes` {#string-bytes}

`ByteIterator bytes()`

Returns an iterator over each byte in the string.

### `characters` {#string-characters}

`ArrayRef<char> characters;`

### `compare` {#string-compare}

`Ordering compare(string* other)`

### `contains` {#string-contains}

`bool contains(char c)`

### `data` {#string-data}

`char[*] data()`

### `empty` {#string-empty}

`bool empty()`

### `escape` {#string-escape}

`StringBuffer escape()`

### `find` {#string-find}

`int find(char c)`

Returns the index of the given character, or the size if it's not found.

`int find(char c, int start)`

Returns the index of the given character, or the size if it's not found. Starts from `start`.

### `front` {#string-front}

`char front()`

Returns the first character.

### `hash` {#string-hash}

`uint64 hash()`

Supports using strings with sets and dicts

### `iterator` {#string-iterator}

`StringIterator iterator()`

### `lines` {#string-lines}

`LineIterator lines()`

Returns an iterator over each line in the string.

### `none` {#string-none}

`bool none(bool(char) predicate)`

### `operator[]` {#string-operator-index}

`char operator[](int index)`

Returns the character at the given index.

### `parseInt` {#string-parseInt}

`int? parseInt()`

### `print` {#string-print}

`void print(StringBuffer* stream)`

### `repeat` {#string-repeat}

`StringBuffer repeat(int count)`

### `size` {#string-size}

`int size()`

### `string` {#string-string}

`string(char* pointer, int length)`

`string(char* cString)`

Initializes a string referring to a null-terminated C-style string.

__Note:__ This constructor has a time complexity of O(n) because it has to calculate the
length of the argument string. If you know the length beforehand, consider using
`string(char* pointer, int length)` instead.


`string(StringBuffer* stringBuffer)`

### `substr` {#string-substr}

`string substr(int start)`

Returns the substring of the string starting from the given index, until the end of the string.

`string substr(Range<int> range)`

Returns the substring of the string in the given range, [inclusive, exclusive]

## `struct StringBuffer: Comparable, Hashable, Printable` {#type-StringBuffer}

Source: [StringBuffer.cx](https://github.com/emillaine/cx/blob/main/std/StringBuffer.cx)

### `all` {#StringBuffer-all}

`bool all(bool(char) predicate)`

### `any` {#StringBuffer-any}

`bool any(bool(char) predicate)`

### `characters` {#StringBuffer-characters}

`List<char> characters;`

### `compare` {#StringBuffer-compare}

`Ordering compare(StringBuffer* other)`

### `contains` {#StringBuffer-contains}

`bool contains(char c)`

### `cString` {#StringBuffer-cString}

`char* cString()`

Returns the string as a C-style, i.e. null-terminated, string.
Modifying `this` after calling this function invalidates the returned pointer.

### `data` {#StringBuffer-data}

`char[*] data()`

Returns a pointer to the first character in the string.

### `empty` {#StringBuffer-empty}

`bool empty()`

### `escape` {#StringBuffer-escape}

`StringBuffer escape()`

### `find` {#StringBuffer-find}

`int find(char c)`

Returns the index of the given character, or the size if it's not found.

`int find(char c, int start)`

Returns the index of the given character, or the size if it's not found. Starts from `start`.

### `hash` {#StringBuffer-hash}

`uint64 hash()`

Supports using strings with sets and dicts

### `iterator` {#StringBuffer-iterator}

`StringIterator iterator()`

### `none` {#StringBuffer-none}

`bool none(bool(char) predicate)`

### `operator[]` {#StringBuffer-operator-index}

`char operator[](int index)`

Returns the character at the given index.

### `operator[]=` {#StringBuffer-operator-index-assign}

`void operator[]=(int index, char c)`

Sets the character at the given index.

### `parseInt` {#StringBuffer-parseInt}

`int? parseInt()`

### `print` {#StringBuffer-print}

`void print(StringBuffer* stream)`

### `push` {#StringBuffer-push}

`void push(char c)`

### `removeFirst` {#StringBuffer-removeFirst}

`void removeFirst()`

Removes the first character from the string.
Other characters are moved towards the beginning of the string by one index.

### `removeLast` {#StringBuffer-removeLast}

`void removeLast()`

Removes the last character from the string.

### `size` {#StringBuffer-size}

`int size()`

### `split` {#StringBuffer-split}

`List<string> split(char delim)`

`List<string> split()`

### `StringBuffer` {#StringBuffer-StringBuffer}

`StringBuffer()`

Initializes an empty string.

`StringBuffer(public int capacity)`

Initializes an empty string with pre-allocated capacity.

`StringBuffer(string s)`

`StringBuffer(char* pointer, int length)`

Initializes a string with the characters from a character array of known length.

`StringBuffer(char* cString)`

Initializes a string with the contents of a null-terminated C-style string.

__Note:__ This constructor has a time complexity of O(n) because it has to calculate the
length of the argument string. If you know the length beforehand, consider using
`StringBuffer(char* pointer, int length)` instead.


`StringBuffer(public int uninitializedSize)`

Initializes the buffer to contain the given number of uninitialized bytes.

### `substr` {#StringBuffer-substr}

`string substr(int start)`

Returns the substring of the string starting from the given index, until the end of the string.

`string substr(Range<int> range)`

Returns the substring of the string in the given range, [inclusive, exclusive]

### `write` {#StringBuffer-write}

`bool write(string s)`

## `struct StringIterator: Copyable, Iterator<char>` {#type-StringIterator}

Source: [StringIterator.cx](https://github.com/emillaine/cx/blob/main/std/StringIterator.cx)

### `current` {#StringIterator-current}

`char[*] current;`

### `end` {#StringIterator-end}

`char[*] end;`

### `hasValue` {#StringIterator-hasValue}

`bool hasValue()`

### `increment` {#StringIterator-increment}

`void increment()`

### `StringIterator` {#StringIterator-StringIterator}

`StringIterator(string s)`

### `value` {#StringIterator-value}

`char value()`

## `struct uint: Copyable, Comparable, Printable, Hashable` {#type-uint}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#uint-compare}

`Ordering compare(uint* other)`

### `hash` {#uint-hash}

`uint64 hash()`

### `print` {#uint-print}

`void print(StringBuffer* stream)`

## `struct uint128: Copyable, Comparable` {#type-uint128}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#uint128-compare}

`Ordering compare(uint128* other)`

## `struct uint16: Copyable, Comparable, Printable, Hashable` {#type-uint16}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#uint16-compare}

`Ordering compare(uint16* other)`

### `hash` {#uint16-hash}

`uint64 hash()`

### `print` {#uint16-print}

`void print(StringBuffer* stream)`

## `struct uint32: Copyable, Comparable, Printable, Hashable` {#type-uint32}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#uint32-compare}

`Ordering compare(uint32* other)`

### `hash` {#uint32-hash}

`uint64 hash()`

### `print` {#uint32-print}

`void print(StringBuffer* stream)`

## `struct uint64: Copyable, Comparable, Printable, Hashable` {#type-uint64}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#uint64-compare}

`Ordering compare(uint64* other)`

### `hash` {#uint64-hash}

`uint64 hash()`

### `print` {#uint64-print}

`void print(StringBuffer* stream)`

## `struct uint8: Copyable, Comparable, Printable, Hashable` {#type-uint8}

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

### `compare` {#uint8-compare}

`Ordering compare(uint8* other)`

### `hash` {#uint8-hash}

`uint64 hash()`

### `print` {#uint8-print}

`void print(StringBuffer* stream)`

## `_fdopen` {#fn-_fdopen}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern FILE*? _fdopen(int fd, const char* mode);`

## `abort` {#fn-abort}

Source: [error.cx](https://github.com/emillaine/cx/blob/main/std/error.cx), [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`never abort<T0: Printable>(T0* _0)`

`never abort<T0: Printable, T1: Printable>(T0* _0, T1* _1)`

`never abort<T0: Printable, T1: Printable, T2: Printable>(T0* _0, T1* _1, T2* _2)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable>(T0* _0, T1* _1, T2* _2, T3* _3)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable, T13: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12, T13* _13)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable, T13: Printable, T14: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12, T13* _13, T14* _14)`

`never abort<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable, T13: Printable, T14: Printable, T15: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12, T13* _13, T14* _14, T15* _15)`

`extern never abort();`

## `abortWrapper` {#fn-abortWrapper}

Source: [error.cx](https://github.com/emillaine/cx/blob/main/std/error.cx)

`never abortWrapper()`

## `abs` {#fn-abs}

Source: [math.cx](https://github.com/emillaine/cx/blob/main/std/math.cx)

`T abs<T>(T value)`

## `all` {#fn-all}

Source: [Iterator.cx](https://github.com/emillaine/cx/blob/main/std/Iterator.cx)

`bool all<T, I>(I* it, bool(T) predicate)`

## `allocate` {#fn-allocate}

Source: [allocate.cx](https://github.com/emillaine/cx/blob/main/std/allocate.cx)

`Type* allocate<Type>(Type value)`

Allocates a block of dynamic memory to hold the given value, moves the value into the memory
block, and returns a pointer to it. The block can be freed by passing the returned pointer to a
call to `deallocate`.

If the memory allocation fails, the program crashes (when compiled in checked mode) or invokes
undefined behavior (when compiled in unchecked mode).


## `allocateArray` {#fn-allocateArray}

Source: [allocate.cx](https://github.com/emillaine/cx/blob/main/std/allocate.cx)

`Type[*] allocateArray<Type>(int size)`

Allocates a block of dynamic memory to hold an array with the given element type and size, and
returns a pointer to the array. The elements of the array are uninitialized. The block can be
freed by passing the returned pointer to a call to `deallocate`.

If the memory allocation fails, the program crashes (when compiled in checked mode) or invokes
undefined behavior (when compiled in unchecked mode).


## `any` {#fn-any}

Source: [Iterator.cx](https://github.com/emillaine/cx/blob/main/std/Iterator.cx)

`bool any<T, I>(I* it, bool(T) predicate)`

## `assertFail` {#fn-assertFail}

Source: [stdio.cx](https://github.com/emillaine/cx/blob/main/std/stdio.cx)

`void assertFail(const char* message)`

## `clamp` {#fn-clamp}

Source: [math.cx](https://github.com/emillaine/cx/blob/main/std/math.cx)

`T clamp<T>(T value, T min, T max)`

## `convertHash` {#fn-convertHash}

Source: [Map.cx](https://github.com/emillaine/cx/blob/main/std/Map.cx)

`int convertHash(uint64 hash)`

## `deallocate` {#fn-deallocate}

Source: [allocate.cx](https://github.com/emillaine/cx/blob/main/std/allocate.cx)

`void deallocate<Type>(Type allocation)`

Deallocates a block of dynamic memory that was previously allocated by a call to `allocate`,
`allocateArray`, `safeAllocate`, or `safeAllocateArray`. If the argument is null, no operation
is performed.

## `exit` {#fn-exit}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern never exit(int status);`

## `fclose` {#fn-fclose}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int fclose(FILE* file);`

## `fdopen` {#fn-fdopen}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`FILE*? fdopen(int fd, const char* mode)`

`extern FILE*? fdopen(int fd, const char* mode);`

## `feof` {#fn-feof}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int feof(FILE* file);`

## `fflush` {#fn-fflush}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int fflush(FILE* file);`

## `fgetc` {#fn-fgetc}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int fgetc(FILE* file);`

## `fopen` {#fn-fopen}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern FILE*? fopen(const char* filename, const char* mode);`

## `fprintf` {#fn-fprintf}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int fprintf(FILE* file, const char* format, ...);`

## `fputc` {#fn-fputc}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int fputc(int c, FILE* file);`

## `fputs` {#fn-fputs}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int fputs(const char* str, FILE* file);`

## `fread` {#fn-fread}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern uint64 fread(void* data, uint64 size, uint64 count, FILE* file);`

## `free` {#fn-free}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern void free(void*? ptr);`

## `fseek` {#fn-fseek}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern void fseek(FILE* file, int64 offset, int origin);`

## `ftell` {#fn-ftell}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int64 ftell(FILE* file);`

## `fwrite` {#fn-fwrite}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern void fwrite(const void* data, uint64 size, uint64 count, FILE* file);`

## `getchar` {#fn-getchar}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int getchar();`

## `isalnum` {#fn-isalnum}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isalnum(int ch);`

## `isalpha` {#fn-isalpha}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isalpha(int ch);`

## `isblank` {#fn-isblank}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isblank(int ch);`

## `iscntrl` {#fn-iscntrl}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int iscntrl(int ch);`

## `isdigit` {#fn-isdigit}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isdigit(int ch);`

## `isgraph` {#fn-isgraph}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isgraph(int ch);`

## `islower` {#fn-islower}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int islower(int ch);`

## `isprint` {#fn-isprint}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isprint(int ch);`

## `ispunct` {#fn-ispunct}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int ispunct(int ch);`

## `isspace` {#fn-isspace}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isspace(int ch);`

## `isupper` {#fn-isupper}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isupper(int ch);`

## `isxdigit` {#fn-isxdigit}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int isxdigit(int ch);`

## `malloc` {#fn-malloc}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern void*? malloc(uint64 size);`

## `max` {#fn-max}

Source: [algorithm.cx](https://github.com/emillaine/cx/blob/main/std/algorithm.cx)

`T* max<T>(T* a, T* b)`

Gives the maximum of the two elements

## `min` {#fn-min}

Source: [algorithm.cx](https://github.com/emillaine/cx/blob/main/std/algorithm.cx)

`T* min<T>(T* a, T* b)`

Gives the minimum of the two elements

## `none` {#fn-none}

Source: [Iterator.cx](https://github.com/emillaine/cx/blob/main/std/Iterator.cx)

`bool none<T, I>(I* it, bool(T) predicate)`

## `operator!=` {#fn-operator-ne}

Source: [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx), [Optional.cx](https://github.com/emillaine/cx/blob/main/std/Optional.cx), [StringBuffer.cx](https://github.com/emillaine/cx/blob/main/std/StringBuffer.cx), [string.cx](https://github.com/emillaine/cx/blob/main/std/string.cx)

`bool operator!= <T: Comparable>(T* a, T* b)`

`bool operator!= <T: Comparable>(Optional<T>* a, Optional<T>* b)`

`bool operator!= <T: Comparable>(Optional<T>* a, T* b)`

`bool operator!=(StringBuffer* a, string b)`

`bool operator!=(string a, StringBuffer* b)`

`bool operator!=(char[*] a, StringBuffer* b)`

`bool operator!=(StringBuffer* a, char[*] b)`

`bool operator!=(char* a, string b)`

`bool operator!=(string a, char* b)`

## `operator+` {#fn-operator-plus}

Source: [StringBuffer.cx](https://github.com/emillaine/cx/blob/main/std/StringBuffer.cx)

`StringBuffer operator+(string a, string b)`

`StringBuffer operator+(string a, char b)`

`StringBuffer operator+(string a, char[*] b)`

`StringBuffer operator+(StringBuffer a, string b)`

`StringBuffer operator+(StringBuffer a, char b)`

`StringBuffer operator+(StringBuffer a, char[*] b)`

## `operator<` {#fn-operator-lt}

Source: [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx)

`bool operator< <T: Comparable>(T* a, T* b)`

## `operator<=` {#fn-operator-le}

Source: [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx)

`bool operator<= <T: Comparable>(T* a, T* b)`

## `operator==` {#fn-operator-eq}

Source: [ArrayRef.cx](https://github.com/emillaine/cx/blob/main/std/ArrayRef.cx), [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx), [List.cx](https://github.com/emillaine/cx/blob/main/std/List.cx), [Optional.cx](https://github.com/emillaine/cx/blob/main/std/Optional.cx), [StringBuffer.cx](https://github.com/emillaine/cx/blob/main/std/StringBuffer.cx), [string.cx](https://github.com/emillaine/cx/blob/main/std/string.cx)

`bool operator==<T>(ArrayRef<T> a, ArrayRef<T> b)`

`bool operator== <T: Comparable>(T* a, T* b)`

`bool operator==<T>(List<T>* a, List<T>* b)`

`bool operator== <T: Comparable>(Optional<T>* a, Optional<T>* b)`

`bool operator== <T: Comparable>(Optional<T>* a, T* b)`

`bool operator==(StringBuffer* a, StringBuffer* b)`

`bool operator==(StringBuffer* a, string b)`

`bool operator==(string a, StringBuffer* b)`

`bool operator==(char[*] a, StringBuffer* b)`

`bool operator==(StringBuffer* a, char[*] b)`

`bool operator==(string a, string b)`

`bool operator==(char* a, string b)`

`bool operator==(string a, char* b)`

## `operator>` {#fn-operator-gt}

Source: [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx)

`bool operator> <T: Comparable>(T* a, T* b)`

## `operator>=` {#fn-operator-ge}

Source: [Comparable.cx](https://github.com/emillaine/cx/blob/main/std/Comparable.cx)

`bool operator>= <T: Comparable>(T* a, T* b)`

## `print` {#fn-print}

Source: [stdio.cx](https://github.com/emillaine/cx/blob/main/std/stdio.cx)

`void print<T: Printable>(T value)`

`void print<T: Printable>(T* value)`

`void print<T: Printable>(List<T>* list)`

`void print<T: Printable>(ArrayRef<T> array)`

`void print<T: Printable>(Optional<T>* optional)`

`void print(const char[*]? cString)`

`void print(char[*]? cString)`

## `printf` {#fn-printf}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int printf(const char* format, ...);`

## `println` {#fn-println}

Source: [stdio.cx](https://github.com/emillaine/cx/blob/main/std/stdio.cx)

`void println<T>(T* value)`

`void println<T: Copyable>(T value)`

`void println(const char[*]? value)`

`void println(char[*]? value)`

`void println<T0: Printable, T1: Printable>(T0* _0, T1* _1)`

`void println<T0: Printable, T1: Printable, T2: Printable>(T0* _0, T1* _1, T2* _2)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable>(T0* _0, T1* _1, T2* _2, T3* _3)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable, T13: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12, T13* _13)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable, T13: Printable, T14: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12, T13* _13, T14* _14)`

`void println<T0: Printable, T1: Printable, T2: Printable, T3: Printable, T4: Printable, T5: Printable, T6: Printable, T7: Printable, T8: Printable, T9: Printable, T10: Printable, T11: Printable, T12: Printable, T13: Printable, T14: Printable, T15: Printable>(T0* _0, T1* _1, T2* _2, T3* _3, T4* _4, T5* _5, T6* _6, T7* _7, T8* _8, T9* _9, T10* _10, T11* _11, T12* _12, T13* _13, T14* _14, T15* _15)`

## `puts` {#fn-puts}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int puts(const char* str);`

## `readFile` {#fn-readFile}

Source: [stdio.cx](https://github.com/emillaine/cx/blob/main/std/stdio.cx)

`StringBuffer readFile(string path)`

## `readLine` {#fn-readLine}

Source: [stdio.cx](https://github.com/emillaine/cx/blob/main/std/stdio.cx)

`StringBuffer readLine()`

## `remove` {#fn-remove}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int remove(const char* filename);`

## `rewind` {#fn-rewind}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern void rewind(FILE* file);`

## `safeAllocate` {#fn-safeAllocate}

Source: [allocate.cx](https://github.com/emillaine/cx/blob/main/std/allocate.cx)

`Type*? safeAllocate<Type>(Type value)`

Allocates a block of dynamic memory to hold the given value, moves the value into the memory
block, and returns a pointer to it. The block can be freed by passing the returned pointer to a
call to `deallocate`.

If the memory allocation fails, null is returned.


## `safeAllocateArray` {#fn-safeAllocateArray}

Source: [allocate.cx](https://github.com/emillaine/cx/blob/main/std/allocate.cx)

`Type[*]? safeAllocateArray<Type>(int size)`

Allocates a block of dynamic memory to hold an array with the given element type and size, and
returns a pointer to the array. The elements of the array are uninitialized. The block can be
freed by passing the returned pointer to a call to `deallocate`.

If the memory allocation fails, null is returned.


## `sort` {#fn-sort}

Source: [algorithm.cx](https://github.com/emillaine/cx/blob/main/std/algorithm.cx)

`void sort<E: Comparable>(List<E>* array)`

Sorts a array. Uses insertion sort if the array is small enough, and quickSort otherwise.

## `sprintf` {#fn-sprintf}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int sprintf(char* str, const char* format, ...);`

## `stderr` {#fn-stderr}

Source: [FileStream.cx](https://github.com/emillaine/cx/blob/main/std/FileStream.cx)

`OutputFileStream stderr()`

## `stdout` {#fn-stdout}

Source: [FileStream.cx](https://github.com/emillaine/cx/blob/main/std/FileStream.cx)

`OutputFileStream stdout()`

## `strlen` {#fn-strlen}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern uint64 strlen(const char* string);`

## `swap` {#fn-swap}

Source: [algorithm.cx](https://github.com/emillaine/cx/blob/main/std/algorithm.cx)

`void swap<E>(E* a, E* b)`

Swaps two items in place

## `tolower` {#fn-tolower}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int tolower(int ch);`

## `toupper` {#fn-toupper}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int toupper(int ch);`

## `ungetc` {#fn-ungetc}

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

`extern int ungetc(int c, FILE* file);`

## `writeFile` {#fn-writeFile}

Source: [stdio.cx](https://github.com/emillaine/cx/blob/main/std/stdio.cx)

`bool writeFile(string path, string content)`

## `SEEK_CUR` {#const-SEEK_CUR}

`const SEEK_CUR = 1;`

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

## `SEEK_END` {#const-SEEK_END}

`const SEEK_END = 2;`

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

## `SEEK_SET` {#const-SEEK_SET}

`const SEEK_SET = 0;`

Source: [libc.cx](https://github.com/emillaine/cx/blob/main/std/libc.cx)

## `int16_max` {#const-int16_max}

`const int16 int16_max = 32767;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int16_min` {#const-int16_min}

`const int16 int16_min = -32768;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int32_max` {#const-int32_max}

`const int32 int32_max = 2147483647;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int32_min` {#const-int32_min}

`const int32 int32_min = -2147483648;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int64_max` {#const-int64_max}

`const int64 int64_max = 9223372036854775807;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int64_min` {#const-int64_min}

`const int64 int64_min = -9223372036854775808;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int8_max` {#const-int8_max}

`const int8 int8_max = 127;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int8_min` {#const-int8_min}

`const int8 int8_min = -128;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int_max` {#const-int_max}

`const int int_max = 2147483647;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `int_min` {#const-int_min}

`const int int_min = -2147483648;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint16_max` {#const-uint16_max}

`const uint16 uint16_max = 65535;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint16_min` {#const-uint16_min}

`const uint16 uint16_min = 0;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint32_max` {#const-uint32_max}

`const uint32 uint32_max = 4294967295;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint32_min` {#const-uint32_min}

`const uint32 uint32_min = 0;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint64_max` {#const-uint64_max}

`const uint64 uint64_max = 18446744073709551615;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint64_min` {#const-uint64_min}

`const uint64 uint64_min = 0;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint8_max` {#const-uint8_max}

`const uint8 uint8_max = 255;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint8_min` {#const-uint8_min}

`const uint8 uint8_min = 0;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint_max` {#const-uint_max}

`const uint uint_max = 4294967295;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)

## `uint_min` {#const-uint_min}

`const uint uint_min = 0;`

Source: [integers.cx](https://github.com/emillaine/cx/blob/main/std/integers.cx)
