# Language overview

A quick tour of cx for programmers who already know a C-family language. Each section
shows the basics of one area and links to the page that covers it in detail. For the
reasoning behind the design, see [Design principles](./design-principles); for how cx
differs from C++, Rust, and others, see [Comparison with related languages](./comparison).

## Variables and functions

Local variables are declared with an explicit type or with `var` to infer it, and `const`
declares compile-time constants. Semicolons are optional, as are the parentheses around
the conditions of `if`, `for`, `while`, and `switch`. Arguments can be labeled with their
parameter names, in any order, which keeps calls like `foo(true, false)` readable:

```cs
void greet(string name, int times) {
    for _ in 0..times {
        println("hi ", name);
    }
}

void main() {
    const greeting = "hello";
    var count = 2; // inferred as int
    println(greeting);
    greet(times = count, name = "Bo"); // prints "hi Bo" twice
}
```

Functions can be overloaded on parameter types and names. See [Variables and
constants](./variables-and-constants) and [Functions](./functions).

## Control flow

`for` loops iterate over collections and ranges, and `switch` cases break automatically,
so a missing `break` can never silently fall through:

```cs
void main() {
    for i in 1...3 {
        switch i {
            case 1: println("one");
            case 2: println("two");
            default: println("other");
        }
    }
}
```

See [Control flow](./control-flow).

## Structs

`struct` is the only composite value type; there is no separate `class`. Member functions
receive `this` as a non-null borrow, and member access always uses `.`, even through
pointers. Constructors take positional or named arguments:

```cs
struct Point {
    int x;
    int y;

    int lengthSquared() {
        return x * x + y * y;
    }
}

void main() {
    var p = Point(y = 4, x = 3);
    var ptr = &p;
    println(ptr.lengthSquared()); // prints 25
}
```

Structs can have destructors, and `defer` runs cleanup when leaving the current scope,
including early returns:

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

See [Structs](./structs), [Anonymous structs](./anonymous-structs), and
[Pointers](./pointers).

## Arrays, lists, and strings

Fixed-size arrays are values that can be passed and returned like any other, `List` is the
resizable array, and strings come as `string` views and owned `StringBuf` buffers:

```cs
void main() {
    int[3] sizes = [64, 128, 32];
    var list = List([1, 2, 3]);
    list.push(sizes[1]);
    println(list); // prints [1, 2, 3, 128]

    var csv = StringBuf("a,b,c");
    println(join(csv.split(','), ";")); // prints a;b;c
}
```

See [Arrays](./arrays), [List](./list), and [Strings](./strings).

## Nullable types

All types are non-nullable unless marked with `?`. The compiler narrows a nullable value to
its non-null type inside a null check, and warns when a possibly-null value is used
without one:

```cs
int? parsePort(string scheme) {
    if scheme == "http" {
        return 80;
    }
    return null;
}

void main() {
    var port = parsePort("http");
    if port {
        println(port + 1); // ok, port is an int here
    }
}
```

See [Nullable types](./nullable-types).

## Enums and tagged unions

Enum cases can carry payloads, making `enum` a type-safe tagged union. `switch` matches on
the case and narrows the value, so each branch can access that case's fields:

```cs
enum Shape {
    Circle(float radius),
    Rect(float width, float height),
}

float area(Shape shape) {
    switch shape {
        case Circle: return 3.0 * shape.radius * shape.radius;
        case Rect: return shape.width * shape.height;
    }
}

void main() {
    println(area(Shape.Circle(1.0))); // prints 3
    println(area(Shape.Rect(width = 2.0, height = 3.0))); // prints 6
}
```

See [Enum types](./enum-types).

## Closures and iterators

Lambdas infer their parameter types and can capture local variables. Functions like
`filter` and `map` return lazy views that chain without allocating:

```cs
void main() {
    var numbers = List([1, 2, 3, 4, 5, 6]);
    int limit = 4;
    var total = numbers.filter(n => n % 2 == 0 && n <= limit).map(n => n * n).sum();
    println(total); // prints 20
}
```

See [Closures](./closures) and [Iterators](./iterators).

## Interfaces and generics

Interfaces declare required methods (and optionally fields and default implementations),
and serve as bounds on generic type parameters, so a mismatch reports which requirement
failed:

```cs
interface HasArea {
    float area();
}

struct Square: HasArea {
    float side;

    float area() {
        return side * side;
    }
}

void printArea<T: HasArea>(T& shape) {
    println(shape.area());
}

void main() {
    var square = Square(side = 2.0);
    printArea(square); // prints 4
}
```

See [Interfaces](./interfaces) and [Generics](./generics).

## Error handling

Fallible functions return `Result` by convention, and callers handle both cases with
`switch`, so every error path is visible in the code:

```cs
Result<int, string> parseDigit(char c) {
    if c >= '0' && c <= '9' {
        return Ok(int(c) - int('0'));
    }
    return Err("not a digit");
}

void main() {
    switch parseDigit('x') {
        case Ok value: println(value);
        case Err error: println(error); // prints "not a digit"
    }
}
```

See [Error handling](./error-handling).

## Safety checks

Debug builds check array bounds, integer overflow, and null dereferences, and report
memory leaks at exit. Where measured performance requires it, checks can be disabled with
attributes on a function, statement, or expression: `@unchecked` disables all checks in
its scope, while `@noOverflowCheck`, `@noBoundsCheck`, and `@noNullCheck` disable one
check each:

```cs
@noOverflowCheck
int wrapAdd(int a, int b) {
    return a + b; // wraps instead of aborting on overflow
}

void main() {
    var x = 2147483647;
    println(wrapAdd(x, 1)); // prints -2147483648
    var y = @noOverflowCheck x + 1; // expression-level
    println(y); // prints -2147483648
}
```

See [Builtin types](./builtin-types) and [Build modes](./build-system#build-modes).

## Modules, builds, and C interop

All `.cx` files in a project compile as one module, with no header files, forward
declarations, or imports between them, and the standard library needs no import either.
`cx run` and `cx build` work without any configuration. C headers import directly, with no
bindings to write:

```cs
import "stdlib.h";

void main() {
    println(atoi("42") + 1); // prints 43
}
```

See [Modules and imports](./modules), [Build system](./build-system), and [Low-level
programming](./low-level-programming).

## More topics

The remaining guide pages cover [Builtin types](./builtin-types), [Type
aliases](./type-aliases), [Unions](./unions), [Access specifiers](./access-specifiers), [Function
pointers](./function-pointers), [Operator overloading](./operator-overloading),
[Casting](./casting), and the [Language server](./lsp).
