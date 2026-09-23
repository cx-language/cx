# Control flow

## for

The for-loop iterates over each element in the range expression.
The range may be anything that has an `iterator()` method that returns an [iterator](iterators).

```cs
void main() {
    println("the range can be any collection");
    for element in [1, 2, 3] {
        println(element);
    }

    println("or an exclusive numeric range");
    for element in 1..3 {
        println(element);
    }

    println("or an inclusive numeric range");
    for element in 1...3 {
        println(element);
    }
}
```

## while

The while-loop loops until its condition evaluates to false.

```cs
void main() {
    int i = 0;
    while i < 3 {
        println(i); // prints: 0 1 2
        i++;
    }
}
```

## break and continue

`break` exits the innermost enclosing loop or `switch` statement,
and `continue` skips to the next iteration of the innermost enclosing loop:

```cs
void main() {
    for i in 0..10 {
        if i == 3 {
            continue; // skips 3
        }
        if i == 5 {
            break; // stops the loop
        }
        println(i); // prints: 0 1 2 4
    }
}
```

Since `switch` cases don't fall through, `break` is only needed in a `switch` to skip
the rest of the matched case body. A `continue` inside a `switch` applies to the
enclosing loop.

## if

```cs
void main() {
    var answer = 42;

    if answer == 0 {
        println("answer is zero");
    } else if answer < 0 {
        println("answer is negative");
    } else {
        println("answer is ", answer);
    }

    // If-expression, aka conditional expression, aka ternary expression.
    println("answer is ", answer == 42 ? "right" : "wrong");
}
```

The `is` operator tests whether an enum value is a specific case.
Appending a name binds the case's payload in the then-branch,
using the same semantics as `switch` case bindings:

```cs
enum Outcome {
    Ok(int value),
    Err(string error),
}

string describe(Outcome* outcome) {
    if *outcome is Err e {
        return e;
    }
    if *outcome is Ok {
        return "ok";
    }
    return "unknown";
}

void main() {
    var ok = Outcome.Ok(value = 1);
    println(describe(ok)); // prints "ok"
}
```

## switch

Unlike in most C-based languages, the case bodies don't fall through to the next by default,
so you don't have to manually `break` after each case.

```cs
void main() {
    var value = 0;

    switch (value) {
        case 0:
            println("value is zero");
        case 1:
            println("value is one");
        default:
            println("value is something else");
    }
}
```

When switching over an enum without a `default`, the compiler inserts a safety check that aborts
with an error if the value is somehow not one of the enum's cases. Valid but unhandled cases still
fall through to the code after the switch. The check is emitted in every build mode except
release mode (`--release`), where the compiler instead assumes the value is always
a valid case.

A `switch` can also be used as an expression, in which case each arm is a single expression
and the whole `switch` evaluates to the matched arm's value.
Unlike statements, switch expressions must handle every case (or have a `default`),
and the arms must produce a value:

```cs
enum Outcome {
    Ok(int value),
    Err(string error),
}

string describe(Outcome* outcome) {
    return switch *outcome {
        case Ok: "ok",
        case Err error: error,
    };
}

void main() {
    var ok = Outcome.Ok(value = 1);
    println(describe(ok)); // prints "ok"
}
```

## defer

`defer` defers the execution of a statement to the exits of the current scope.
This is useful for example when we need to perform some cleanup before returning.
This avoids the mistake of forgetting to add necessary cleanup calls when we add a new return statement.

```cs
int main() {
    var p = safeAllocate<int>(0); // allocate some resource
    defer deallocate(p); // defer deallocation of the resource

    if p == null {
        return 1; // deallocate(p) will be called immediately before this return
    }

    return 0; // deallocate(p) will be called also before this return
}
```

## Planned features

- Add `fallthrough` keyword to explicitly enable switch case fallthrough behavior.
