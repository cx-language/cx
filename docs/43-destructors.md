# Defer and destructors

## defer

`defer` defers the execution of a statement to the exits of the current scope.
This is useful for example when we need to perform some cleanup before returning.
This avoids the mistake of forgetting to add necessary cleanup calls when we add a new return statement.
`defer` is not allowed in constructors.

```cs
int main() {
    var p = allocateValue<int>(0); // allocate some resource
    defer deallocate(p); // runs before each return below

    if *p != 0 {
        return 2; // deallocate(p) runs before this return too
    }

    return 0; // ... and before this return
}
```

## Destructors

A struct can declare a destructor, written `~` followed by the struct name.
It runs automatically when an instance goes out of scope,
which makes it a good place to release resources:

```cs
struct Logger {
    string prefix;

    ~Logger() {
        println(prefix, " closed");
    }
}

void main() {
    var _log = Logger(prefix = "app");
    println("working"); // prints "working", then "app closed" at scope exit
}
```

A temporary - a struct value that isn't stored anywhere - is destroyed at the
end of the statement that creates it:

```cs
struct Logger {
    string prefix;

    ~Logger() {
        println(prefix, " closed");
    }
}

void main() {
    _ = Logger(prefix = "tmp"); // prints "tmp closed" at the end of this statement
    println("working");
}
```

A value moved into a variable, argument, or return value belongs to its new
home instead, so its destructor still runs exactly once, when the owner dies.

A destructor can also be invoked explicitly with `.deinit()`.
This consumes the value like a move: the destructor does not run again
at scope exit, so the value cannot be used afterwards unless it is
reassigned first.

Explicit destruction is for values the compiler won't destroy,
such as elements in manually allocated memory.
Destroying through a local pointer is tracked: when `p` points at a
stack variable, `(*p).deinit()` consumes it like a direct deinit.
Through a function parameter or struct member the compiler cannot know
the target, so it stays unchecked: if the pointer reaches a stack
variable that later dies at scope exit, the value is destroyed twice.
Only deinit through such pointers when nothing else owns the target,
as container internals do with heap memory.
This is how containers destroy their elements:

```cs
void main() {
    var buffer = allocate<List<int>>(1);

    var slot = &buffer[0];
    slot.init(List([1, 2, 3]));
    println(buffer[0]); // prints "[1, 2, 3]"

    buffer[0].deinit(); // destroys the element; nothing will destroy it again
    deallocate(buffer);
}
```

The dual of `.deinit()` is `.forget()`: it also consumes the value, but
runs no destructor at all, neither now nor at scope exit. It is for bytes
already moved elsewhere by hand, such as elements copied out of a temporary
array into a container. Forgetting a live value leaks it; like `.deinit()`,
the value cannot be used afterwards.

## Manual destruction

Fields are destroyed automatically when the destructor finishes. Marking a
field `@manuallyDestroy` opts it out: the destructor must then destroy the
field explicitly with `.deinit()`. This is for storage that is only
partially initialized, such as the inline buffer of `SmallList`, where
destroying every slot would destroy uninitialized memory. A struct with a
manually-destroyed owning field but no destructor is a compile error. The
compiler does not check that the destructor actually destroys the field;
forgetting `.deinit()` leaks it silently.

A local variable marked `@manuallyDestroy` stays alive when its scope ends,
including on `return`. Assignment still destroys the previous value, so
assigning over uninitialized scratch would destroy uninitialized memory.
This is for storage that must use the value's alignment and must not own
the bytes, such as swapping two values by copying their representation. A
`uint8` buffer is aligned as a byte, so it is the wrong scratch for that.
The compiler does not check that the variable is destroyed; forgetting
`.deinit()` on a value that should be destroyed leaks it.

```cs
struct Buffer {
    @manuallyDestroy
    List<int> items;

    Buffer() {
        items = List([1, 2, 3]);
    }

    ~Buffer() {
        items.deinit();
    }
}

void main() {
    var buffer = Buffer();
    println(buffer.items.size()); // prints 3
}
```

```cs
struct Probe {
    int id;

    ~Probe() {
        println("drop");
    }
}

void main() {
    {
        var tracked = Probe(id = 1);
        println(tracked.id); // prints 1
    } // prints "drop"

    @manuallyDestroy
    var scratch = Probe(id = 2);
    println(scratch.id); // prints 2
    println("done"); // prints "done"
}
```
