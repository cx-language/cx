# Context

cx has a context: settings implicitly used by
the standard library, threaded through the call chain as a hidden function
parameter. The context holds an allocator, initially the default one, and
`allocate` and friends allocate through it. The default allocator uses malloc.

Install a custom allocator with `withAllocator` to route a stretch of code
through it, for example to count allocations or serve them from an arena:

```cs
int allocCount = 0;

void*? countingAlloc(void*? state, c_size_t size) {
    allocCount++;
    var def = defaultAllocator();
    return def.alloc(def.state, size);
}

void countingFree(void*? state, void* ptr) {
    var def = defaultAllocator();
    def.free(def.state, ptr);
}

void makeList() {
    var list = List<int>();
    list.push(1);
}

void main() {
    var allocator = Allocator(state = null, alloc = countingAlloc, free = countingFree);
    withAllocator(allocator, makeList);
    println(allocCount); // prints 1
}
```

Each allocation must be freed with the same allocator that allocated it, so
values must not outlive the `withAllocator` call that allocated them.
The override applies to `body` and everything it calls, without leaking back
to the caller: every cx function takes its caller's context as a hidden
parameter. Assigning `context().allocator` directly also works, but mutates
the shared context instead of pushing an override, so the change stays visible
to the caller until assigned back. The context doesn't cross language
boundaries: C callbacks and exported `extern` functions start from the
default context. Callbacks must be passed directly as a function or
non-capturing lambda; a function pointer held in a variable can't be
converted to a C callback.
Allocators must return 16-aligned pointers and tolerate freeing null.
A custom allocator must not call back into context-allocating standard library
functions, or allocation recurses forever; use `defaultAllocator()` for
passthrough as above. One exception: `Arena` chunks always use malloc, so an
arena installed as the context allocator never allocates from itself.

## Planned features

- A fuller context, carrying more than the allocator (for
  example a temporary allocator and a logger).
