# Arena

`Arena` is a move-only, chunked bump allocator for short-lived groups of allocations. It grows by adding chunks, so pointers returned by earlier allocations stay valid until the arena is destroyed or explicitly deinitialized.

Allocation results are nullable.

<details>
<summary>Allocation edge cases</summary>

A null result means that the system allocator could not provide another chunk. Array counts use `int64`. Returned storage is aligned to 16 bytes. A zero-count array returns a valid pointer to zero-length storage.

</details>

```cs
void main() {
    var arena = Arena(chunkSize = 4096);

    var value = arena.allocate<int>();
    if value != null {
        value.init(42);
        println(*value); // prints 42
    }

    var values = arena.allocate<int>(3);
    if values != null {
        for i in 0..3 {
            values[i] = i;
        }
    }
}
```

`allocate<T>()` and `allocate<T>(count)` return uninitialized storage. Initialize each value before reading it. `allocateValue(value)` is a convenience overload that moves one initialized value into the arena. The arena destructor frees its backing chunks; it does not infer or run destructors for values placed in that storage, so explicitly deinitialize values that own resources before the arena is destroyed. Do not use an arena after calling `deinit()`. An arena is not thread-safe; concurrent writers need separate arenas or external synchronization.

## Planned features

- A type-safe reallocation function for memory from `allocate`, alongside allocation and
  deallocation, which growable containers such as `List` use when growing.
