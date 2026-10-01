# Unions

A `union` declares overlapping members: every member starts at offset zero,
so writing one member is visible through the others. The size is the largest
member's size, like in C. Unions are for type punning and for matching C
unions imported from headers.

```cs
union IntOrFloat {
    int32 i;
    float32 f;
}

void main() {
    IntOrFloat u = undefined;
    u.i = 42;
    println(u.i); // prints 42
    u.f = 1.5; // overwrites the same 4 bytes
    println(u.i); // prints 1069547520, the bits of 1.5f
    println(sizeof(IntOrFloat)); // prints 4
}
```

Unions have no constructors: declare a value and assign its members. Unions
can have methods and can be generic, but they cannot implement interfaces
and members cannot have default values. Unions copy by value unless they
declare a destructor, in which case their values move like structs. A
`union` in cx has the same layout as the equivalent C `union`, so values can
cross `extern "C"` boundaries the same way as imported C unions. There is no
`print` overload for unions: the compiler cannot know which member is
active, so print a member instead.

## Destruction

Members overlap, so at most one member holds a live value at a time. An
owning member is therefore rejected unless it is marked `@manuallyDestroy`,
in which case the union must declare a destructor that destroys it
explicitly. Assigning to a member never destroys the old bytes, since they
may hold a different member; destroy the active member with `.deinit()`
before switching to another one. A `.deinit()` on a union member does not
consume the union, but the destructor the union declares still runs when the
scope ends, so the member it destroys must be the active one then. The
compiler does not track which member is active: destroying a member twice,
reading a member after `.deinit()`, or writing through a pointer taken to a
member while another member is active all destroy or read bytes as the wrong
type. Only take a member's address when that member is active and stays
active for the pointer's lifetime.

```cs
union Manual {
    @manuallyDestroy List<int> items;
    int32 tag;

    ~Manual() {
        items.deinit();
    }
}

void main() {
    Manual m = undefined;
    m.tag = 1;
    m.items = List([1, 2, 3]); // overwrites tag without destroying it
    m.items.deinit(); // destroy before switching back
    m.tag = 2;
    m.items = List([4]); // 'items' must be active when 'm' is destroyed
}
```
