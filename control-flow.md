# Control Flow

## if

An if-statement looks like this:

```
if (a) {
    // ...
} else if (b) {
    // ...
} else {
    // ...
}
```

## for

There is no C-style for-loop. Instead we have a for-each, aka a range-based for-
loop (not fully implemented yet):

```
for (element in range) {
    // ...
}
```

This iterates over each element in the range-expression.

## while

A while-loop has the following syntax:

```
while (a) {
    // ...
}
```

## do-while

A do-while-loop (not implemented yet) is the same as a while-loop, except that
it checks the condition at the end of each iteration:

```
do {
    // ...
} while (a)
```

## switch

```
switch (someValue) {
    case 0:
        // code to execute if someValue equals 0
    case 1:
        // code to execute if someValue equals 1
    default:
        // code to execute if someValue equals something else
}
```

Unlike in most C-based languages, the case bodies don't fall through to the one
below by default. This behavior can be enabled for individual cases using the
`fallthrough` keyword (not implemented yet).

## break

Inside loops and case-blocks, you can transfer the control flow outside the
loop/switch with the `break` statement.

## continue

Inside loops, you can skip the rest of the loop body and start executing the
next iteration with the `continue` statement (not implemented yet).
