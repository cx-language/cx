# Builtin types

Type      | Meaning
:---------|:----------------------------------------------------------------------
`int`     | 32-bit signed integer, default type for integer literals
`int8`    | 8-bit signed integer
`int16`   | 16-bit signed integer
`int32`   | 32-bit signed integer
`int64`   | 64-bit signed integer
`uint`    | 32-bit unsigned integer
`uint8`   | 8-bit unsigned integer
`byte`    | 8-bit unsigned integer
`uint16`  | 16-bit unsigned integer
`uint32`  | 32-bit unsigned integer
`uint64`  | 64-bit unsigned integer
`float`   | 32-bit floating-point number, default type for floating-point literals
`float32` | 32-bit floating-point number
`float64` | 64-bit floating-point number
`bool`    | boolean
`char`    | C/C++ `char` type

## Numeric literals

Integer literals are decimal by default. Underscores may separate digits for
readability (`1_000_000`), and the prefixes `0x`, `0o`, and `0b` select
hexadecimal, octal, and binary.

Floating-point literals have a fraction (`.5` is written `0.5`), a base-10
exponent introduced by `e` or `E` with an optional sign, or both:

```cs
void main() {
    println(100.0);
    println(1e10);
    println(1.5e-3);
    println(2E+2);
}
```

## Arithmetic operators

There are the usual arithmetic operators `+`, `-`, `*`, `/`, `%`, `&&`, `||`,
`!`, `&`, `|`, `^`, `~`, `<<`, `>>`, and the compound assignment counterparts
for the binary operators: `+=`, `-=`, `*=`, `/=`, `%=`, `&&=`, `||=`, `&=`,
`|=`, `^=`, `<<=`, `>>=`.

An important thing to note about `+`, `-`, and `*` is that they don't silently
wrap on overflow. Instead they abort with an "integer overflow" error, except in
release-fast mode (`--mode=release-fast`), where overflow is unchecked and wraps.
Constant arithmetic is checked at compile time: a constant `+`, `-`, or `*`
whose result doesn't fit its type is an error in every build mode. The wrapping
behavior can be enabled for individual operations with a special syntax (not
implemented yet).

### Increment and decrement operators

The increment and decrement operators, written as postfix `++` and `--`,
respectively, increment/decrement their operand by one. They can only be used as
standalone statements, not inside arbitrary expressions.

---

## Planned features

- More builtin type aliases, such as `byte`, `double`, `long`, `short`
- Exponentiation operator
