# Operators

## Arithmetic Operators

There are the usual arithmetic operators `+`, `-`, `*`, `/`, `%`, `&&`, `||`,
`!`, `&`, `|`, `^`, `~`, `<<`, `>>`, and the compound assignment counterparts
for the binary operators: `+=`, `-=`, `*=`, `/=`, `%=`, `&&=`, `||=`, `&=`,
`|=`, `^=`, `<<=`, `>>=`.

An important thing to note about `+`, `-`, and `*` is that they don't silently
wrap on overflow. Instead they raise an error (in checked mode), or invoke
undefined behavior (in unchecked mode) (not implemented yet). The wrapping
behavior can be enabled for individual operations with a special syntax (not
implemented yet).

## Increment and Decrement Operators

The increment and decrement operators, written as postfix `++` and `--`,
respectively, increment/decrement their operand by one. They can only be used as
standalone statements, not inside arbitrary expressions.

## Power Operator

The language might provide an exponentiation operator `**` in the future.

## Range Operators

The binary operators `..` and `...` create objects of type `Range` and
`ClosedRange`, respectively. `Range` has an exclusive upper bound, while
`ClosedRange` has an inclusive upper bound. These can be used e.g. in a
for-loop:

```
for (i in 0..5) {
    // iterates from 0 to 4
}

for (i in 0...5) {
    // iterates from 0 to 5
}
```

## Unwrap Operator

The unwrap operator, written as a postfix `!`, takes an operand of an optional
type, and returns the value wrapped by the operand. The operand must be
non-null, otherwise the operation triggers a runtime error (by default), or
invokes undefined behavior (in unchecked mode).
