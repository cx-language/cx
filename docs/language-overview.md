# Language overview

## Simple and expressive language

cx keeps the language small so there is less to learn and less that can surprise you.
There is only one composite value type (`struct`) instead of separate struct and class concepts,
member functions receive `this` as a non-null `T&` borrow, and member access always uses `.`,
never `->`. Fewer overlapping concepts means a C++ programmer is productive immediately,
without relearning which of several similar tools fits each situation.

Control flow follows the same principle. Switch cases break automatically, so a missing `break`
can never silently fall through the way it does in C and C++:

```cs
void main() {
    var x = 2;
    switch x {
        case 1: println("one");
        case 2: println("two");
        default: println("other");
    }
}
```

The language is also simpler to implement tooling for: parsing is straightforward, which makes
syntax highlighting, auto-formatting, and linting easier to write and keep correct.

## Safer by default

Bugs that C and C++ leave to testing and luck are caught automatically. Array accesses are
bounds-checked, integer arithmetic is overflow-checked, and dereferencing null is checked,
so corrupted output and security holes from reading past a buffer turn into loud failures
at the exact line that caused them. The checks can be disabled individually or globally
when measured performance requires it.

Values that may be absent are marked with `?`, which forces callers to handle both cases
instead of forgetting a null check:

```cs
int? readPort() {
    return 8080;
}

void main() {
    if readPort() != null {
        println(readPort()! + 1);
    }
}
```

The compiler also warns when a variable might be read before initialization, for example when
passing it as an out parameter to a C function. Assigning the keyword `undefined` to the
variable suppresses the warning when the programmer knows it is safe. With `--Wunused-result`,
ignored return values produce a warning as well; assigning to `_` marks a result as deliberately
unused instead of casting to void.

## Improved type system

Stronger typing catches bugs at compile time and makes refactoring safer. There are no
bug-prone implicit conversions between built-in types; safe ones are still allowed but can be
forbidden with a compiler flag. All types are non-nullable by default, with `?` marking the
ones that admit null, so an unexpected null becomes a compile error rather than a crash.

Fallible operations return `Result` instead of out parameters or sentinel values, and callers
handle each case explicitly with `switch`:

```cs
Result<int, string> fetch(bool ok) {
    if ok {
        return Result.Ok(value = 42);
    }
    return Result.Err(error = "boom");
}

void main() {
    switch fetch(true) {
        case Ok value: println(value);
        case Err error: println(error);
    }
}
```

Generic code constrains its parameters with `interface`s (cx's equivalent of C++ concepts),
so instantiating a template with an unsuitable type reports which requirement failed instead
of pages of substitution errors. Unconstrained parameters stay available for flexibility.
Arrays are first-class values with a known size that can be returned and passed by value,
and anonymous structs provide lightweight syntax for grouping values. Tagged unions allow
runtime polymorphism without dynamic allocation and virtual function calls.

## Standard library covers common use cases better

String handling works on views and owned buffers with practical helpers. Splitting user input
and rejoining it needs no manual loop:

```cs
void main() {
    var csv = StringBuf("a,b,c");
    var fields = csv.split(',');
    var text = join(fields, ";");
    println(text);
}
```

Collections offer the transformations used in everyday data processing. Computing a total or
reshaping a list reads as a pipeline instead of nested loops and temporary buffers:

```cs
void main() {
    var nums = List([1, 2, 3, 4]);
    println(sum(nums));
    var doubled = nums.map(n => n * 2).toList();
    println(doubled);
}
```

Math support includes constants such as Pi:

```cs
void main() {
    println(pi);
}
```

Beyond that, the standard library covers file system access and process control, and its
algorithms take range objects instead of iterator pairs for ease of use.

## Improved syntax

cx's syntax stays close to the C family while removing verbose or cryptic spellings. There is
no C-style cast syntax, only explicit conversions such as `int(x)`. Array sizes sit next to
the element type, semicolons are optional, and lambdas infer their argument types, which keeps
callbacks readable:

```cs
void main() {
    var nums = List([1, 2, 3, 4]);
    var doubled = nums.map(n => n * 2).toList();
    println(doubled);
}
```

## Better compilation model

No header files and no forward declarations: a program is compiled as a whole instead of one
translation unit at a time, which leaves more room for optimization and avoids link-time
surprises. Libraries are imported as a whole, so users never hunt for which file declares a
feature, and library authors can reorganize files without breaking compatibility. Every
library lives in its own namespace automatically, with no per-file declarations to maintain.
Together this means faster builds and less time wrestling the build when a project grows past
a handful of files.

## Standard build system and package manager

Building a cx project is done with a single command, `cx build`, which works out of the box without any build
configuration, and dependencies are fetched from Git repositories. See the [Build system](./build-system) page for details.

Long-term goal: include a built-in linting tool that runs during compilation to enforce a specific coding style, or to
disallow uses of certain language features.

## Faster than C++

More optimization opportunities:

- Moves are destructive, removing the need to reset moved-from objects and call their destructors, as is required in
  C++.
- Pointers can be declared non-aliasing either individually or globally with a compiler flag.
- The language provides a type-safe reallocation function for arrays, instead of just allocation and deallocation
  functions like C++. For example, the cx equivalent of C++'s `std::vector` makes use of this function.
- Unsigned integer overflow is undefined. There are still functions to do wrapping arithmetic on both signed and
  unsigned integers, when needed.
- Compiler is allowed to reorder struct fields. This can be prevented using an attribute or compiler flag.

cx uses the open-source LLVM library as a code generation back-end, benefiting from all current and future optimizations
implemented in LLVM.

No hidden expensive operations, such as implicit calls to copy constructors and copy assignment operators like in C++.

## Transparent interoperation with existing C APIs

Existing C libraries stay usable: headers import directly, so calling into battle-tested code
such as parsers, codecs, or operating system APIs needs no bindings layer. A command-line tool
that needs one C helper does not have to drop down to C for the whole program:

```cs
import "stdlib.h";

void main() {
    println(atoi("42") + 1);
}
```

Support for some level of interoperability with C++ APIs is a longer-term goal.

## Additional language features

__Destructuring__ binds enum payloads and multiple values to separate variables at the point of
use, so dispatching on a result reads linearly instead of nesting accessors:

```cs
void main() {
    Result<int, string> r = Result.Ok(value = 42);
    switch r {
        case Ok value: println(value);
        case Err error: println(error);
    }
}
```

__Named arguments__ label call sites that would otherwise be cryptic sequences like
`foo(true, false)`. Labels are checked against parameter names and may come in any order,
which keeps calls readable when a function takes several same-typed parameters:

```cs
struct Point: Copyable {
    int x;
    int y;
}

void main() {
    var p = Point(y = 2, x = 1);
    println(p.x + p.y);
}
```

__Defer statement__ runs cleanup when leaving the current scope, including early returns.
Resource handling no longer needs an RAII wrapper class per resource just to guarantee release:

```cs
void process(bool fail) {
    defer println("releasing resource");
    if fail {
        println("failed, returning early");
        return;
    }
    println("working");
}

void main() {
    process(false);
    process(true);
}
```

__Simple type inference__ for local and global variables removes redundant annotations, which
keeps code shorter and makes type changes during refactoring touch fewer lines. The strong
type system ensures inference never silently picks an unexpected type.

__Compile-time reflection__ (for example iterating over enum cases or rendering an enum case
as text) is a longer-term goal. It should follow the "pay only for what you use" principle.

## ...and all the good parts from C and C++

- Performance and control.
- Pay only for what you use.
- No garbage collection.
- Don't force the programmer to use a specific style.
- Have some implicit conversions and other features for convenience. Allow the programmer to customize these to their preference via compiler warnings and/or linter options.
- Backwards compatibility eventually once the language design has stabilized.
