# Move semantics

Struct and enum values are copied by default: passing one to a function,
returning it, or assigning it to a new variable copies it, and the
original stays usable.

```cs
struct Point {
    int x;
    int y;
}

void main() {
    var a = Point(1, 2);
    var b = a; // copies; both a and b stay usable
    println(a.x + b.x); // prints 2
}
```

A [destructor](./destructors) opts a type out of copying. Values of such types move
instead: the source cannot be used afterwards. Holding a non-copyable
value has the same effect, so ownership spreads outward: a struct
holding a `List` cannot be copied either.

Types without a destructor that must still not be copied - unique
handles or tokens, for example - can declare an empty destructor to
opt out:

```cs
struct Token {
    int id;

    ~Token() {}
}

void main() {
    var t = Token(1);
    println(t.id); // prints 1
}
```

Generic code can require copyability with a `Copyable` constraint:

```cs
void printTwice<T: Copyable>(T value) {
    println(value);
    println(value);
}

void main() {
    printTwice(42); // prints 42 twice
}
```

Values stored in a struct field or behind a pointer can be moved out
explicitly with `take()`, which leaves a fresh default value behind so
the container stays usable. The stdlib collections provide it
(`List`, `Map`, `Set`, `SmallList`, `StringBuf`, `Optional`, `Arena`):

```cs
struct Team {
    string tag;
    List<string> members;
}

void main() {
    var team = Team("red", List(["a", "b"]));
    var members = team.members.take(); // moves; team.members is now empty
    println(members.size()); // prints 2
    println(team.members.size()); // prints 0
}
```

<details>
<summary>Moving out without take()</summary>

Moving a field out without `take()` is rejected: `var members =
team.members;` fails to compile, suggesting `.take()`, a borrow, or a
copy instead. The same holds for moving elements out of collections and
payloads out of optionals.

</details>

<details>
<summary>Conditional-move destruction</summary>

Moving a value on only one branch of an `if` or `switch` statement destroys
it on the other paths, so it dies exactly once; the value is moved after
the statement, and later uses are rejected. Moves in ternary or `switch`
expression arms still warn instead: no statement there can destroy the
live paths, so the value may leak and later uses warn. Those warnings
suggest `.take()` where the type offers it. The same holds for moves in
`&&`, `||`, and `??` right-hand sides, which may not execute. Reassigning
the value clears the moved state, overwriting without destroying the old
one, which is already gone on every path.

</details>

<details>
<summary>Loop move restrictions</summary>

Moving a value declared outside a loop inside the loop is rejected:
the loop may run more than once, moving it again. Values declared inside
the body and moves into `return` are still allowed, as is reassigning a
pre-loop accumulator itself; moving a different pre-loop value into it
(`result = x`) is still rejected. The check is lexical: reassigning a
value before moving it later in the same iteration is still rejected.

</details>
