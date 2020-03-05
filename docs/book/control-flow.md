# Control flow

## for

The for-loop iterates over each element in the range expression.
The range may be anything that has an `iterator()` method that returns an [iterator](iterators.html).

```cs
void main() {
    println("the range can be any collection");
    for (var element in [1, 2, 3]) {
        println(element);
    }

    println("or an inclusive numeric range");
    for (var element in 4...6) {
        println(element);
    }

    println("or an exclusive numeric range");
    for (var element in 7..9) {
        println(element);
    }
}
```

## while

The while-loop loops until its condition evaluates to false.

```cs
void main() {
    int i = 0;
    while (i < 3) {
        println(i); // prints: 0 1 2
        i++;
    }
}
```

## if

```cs
void main() {
    var answer = 42;

    if (answer == 0) {
        println("answer is zero");
    } else if (answer < 0) {
        println("answer is negative");
    } else {
        println("answer is ", answer);
    }

    // If-expression, aka conditional expression, aka ternary expression.
    println("answer is ", answer == 42 ? "right" : "wrong");
}
```

## switch

Unlike in most C-based languages, the case bodies don't fall through to the next by default,
so you don't have to manually `break` after each case.

```cs
switch (value) {
    case 0:
        // code to execute if value equals 0
    case 1:
        // code to execute if value equals 1
    default:
        // code to execute if value equals something else
}
```

## defer

`defer` defers the execution of a statement to the exits of the current scope.
This is useful for example when we need to perform some cleanup before returning.
This avoids the mistake of forgetting to add necessary cleanup calls when we add a new return statement.

```cs
int main() {
    var p = malloc(sizeof(int)); // allocate some resource
    defer free(p); // defer deallocation of the resource

    if (p == null) {
        return 1; // free(p) will be called immediately before this return
    }

    return 0; // free(p) will be called also before this return
}
```

## Future additions

- Add `fallthrough` keyword to explicitly enable switch case fallthrough behavior.
