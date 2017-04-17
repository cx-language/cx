# Operators

## Arithmetic Operators

There are the usual arithmetic operators `+`, `-`, `*`, `/`, `%`, `&&`, `||`,
`!`, `&`, `|`, `^`, `~`, `<<`, `>>`, and the compound assignment counterparts
for the binary operators: `+=`, `-=`, `*=`, `/=`, `%=`, `&&=`, `||=`, `&=`,
`|=`, `^=`, `~=`, `<<=`, `>>=`.

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

## Unwrap Operator

The unwrap operator, written as a postfix `!`, converts a pointer to a
reference. The pointer must be non-null, otherwise the program crashes (if
built in checked mode), or invokes undefined behavior (in unchecked mode).
