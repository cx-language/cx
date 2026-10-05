# Builtin types

Type      | Meaning
:---------|:----------------------------------------------------------------------
`int`     | alias of `int32`, default type for integer literals
`int8`    | 8-bit signed integer
`sbyte`   | alias of `int8`
`int16`   | 16-bit signed integer
`int32`   | 32-bit signed integer
`int64`   | 64-bit signed integer
`uint`    | alias of `uint32`
`uint8`   | 8-bit unsigned integer
`byte`    | alias of `uint8`
`uint16`  | 16-bit unsigned integer
`uint32`  | 32-bit unsigned integer
`uint64`  | 64-bit unsigned integer
`c_size_t` | C `size_t` type (pointer-sized unsigned integer: 32-bit on wasm32, 64-bit on 64-bit targets)
`c_schar` | C `signed char` type (8-bit signed integer)
`c_uchar` | C `unsigned char` type (8-bit unsigned integer)
`c_short` | C `short` type (16-bit signed integer)
`c_ushort` | C `unsigned short` type (16-bit unsigned integer)
`c_int` | C `int` type (32-bit signed integer)
`c_uint` | C `unsigned int` type (32-bit unsigned integer)
`c_long` | C `long` type (32-bit signed integer on Windows and wasm32, 64-bit on 64-bit Unix targets)
`c_ulong` | C `unsigned long` type (32-bit unsigned integer on Windows and wasm32, 64-bit on 64-bit Unix targets)
`c_longlong` | C `long long` type (64-bit signed integer)
`c_ulonglong` | C `unsigned long long` type (64-bit unsigned integer)
`c_float` | C `float` type (32-bit floating-point number)
`c_double` | C `double` type (64-bit floating-point number)
`float`   | alias of `float32`, default type for floating-point literals
`float32` | 32-bit floating-point number
`float64` | 64-bit floating-point number
`double`  | alias of `float64`
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
    println(1_000_000); // prints 1000000
    println(0xFF); // prints 255
    println(0o17); // prints 15
    println(0b101); // prints 5
    println(100.0); // prints 100
    println(1e10); // prints 1e+10
    println(1.5e-3); // prints 0.0015
    println(2E+2); // prints 200
}
```

## Arithmetic operators

There are the usual arithmetic operators `+`, `-`, `*`, `/`, `%`, `%%`, `&&`, `||`,
`!`, `&`, `|`, `^`, `~`, `<<`, `>>`, wrapping operators `+%`, `-%`, `*%`, saturating
operators `+|`, `-|`, `*|`, `<<|`, and the compound assignment counterparts
for the binary operators: `+=`, `-=`, `*=`, `/=`, `%=`, `&&=`, `||=`, `&=`,
`|=`, `^=`, `<<=`, `>>=`, `+%=`, `-%=`, `*%=`, `+|=`, `-|=`, `*|=`, `<<|=`.

An important thing to note about `+`, `-`, and `*` is that they don't silently
wrap on overflow. Instead they abort with an "integer overflow" error, except in
release mode (`--release`), where overflow is unchecked and wraps.
Overflow checks can also be disabled for a single function, statement, or
expression with `@noOverflowCheck` (or `@unchecked` for all safety checks).
Unary `-` is checked the same way: negating the minimum value of a signed
type, or any nonzero unsigned value, aborts.
Constant arithmetic is checked at compile time: a constant `+`, `-`, `*`,
or unary `-` whose result doesn't fit its type is an error in every build mode.

Wrapping operators `+%`, `-%`, and `*%` (and `+%=`, `-%=`, `*%=`) wrap on overflow
with two's-complement wraparound. Unary `-%` is wrapping
negation. Saturating operators `+|`, `-|`, `*|`, and `<<|` (and `+|=`, `-|=`,
`*|=`, `<<|=`) clamp to the type's minimum or maximum instead. These operators
work on integers only, in every build mode, including on constant expressions:

```cs
void main() {
    println(byte(255) +% 1); // prints 0
    println(byte(255) +| 1); // prints 255
    println(byte(200) *% 2); // prints 144
    println(byte(200) *| 2); // prints 255
    int8 min = -128;
    println(-%min); // prints -128
    println(byte(1) <<| 8); // prints 255
}
```

The `%` operator is a truncated remainder: its result takes the sign of
the dividend, so `-7 % 3` is `-1`. The `%%` operator is a positive
remainder instead: its result takes the sign of the divisor, so with a
positive divisor the result is always in the range 0 to divisor - 1.

```cs
void main() {
    println(7 % 3); // prints 1
    println(-7 % 3); // prints -1
    println(7 %% 3); // prints 1
    println(-7 %% 3); // prints 2
}
```

### Increment and decrement operators

The increment and decrement operators, written as postfix `++` and `--`,
respectively, increment/decrement their operand by one. They can only be used as
standalone statements, not inside arbitrary expressions.

## `sizeof`

`sizeof(T)` and `sizeof(x)` are constant integer expressions. When the type
has to be inferred, as in `var s = sizeof(T)`, it is `int`. In an arithmetic
expression the sizeof operand takes the other operand's numeric type, so
`sizeof(Element) * size` has the type of `size`.

```cs
void main() {
    int n = 3;
    int64 wide = 3;
    var bytes = sizeof(int); // int
    var scaled = sizeof(int) * n; // int
    var scaledWide = sizeof(int) * wide; // int64
    println(bytes); // prints 4
    println(scaled); // prints 12
    println(scaledWide); // prints 12
}
```

---

## Planned features

- Exponentiation operator
- Integer overflow of the checked operators becomes undefined behavior in release mode
  (`--release`), so the optimizer can assume it doesn't happen; for now it wraps. Debug
  and `--release-safe` builds keep checking it, and the wrapping operators wrap in every
  build mode.
