# Strings

cx has two string types:

- `string`, a reference to a sequence of characters, and the type of string literals.\
  Use it when you need to pass a string to a function without mutating it.\
  It's like `std::string_view` in C++ and `&str` in Rust.
- `StringBuf`, a buffer storing a sequence of characters.\
  Use it when you need to build dynamic strings.\
  It's like `std::string` in C++ and `String` in Rust.

```cs
void main() {
    var a = "test";
    // a has type 'string'

    var b = StringBuf(a);
    // b holds a copy of "test" that we can mutate

    b.append('!');
    // b now holds "test!", a is unchanged

    println(a); // prints "test"
    println(b); // prints "test!"
}
```

## Interpolation

`$name` embeds a value and `${expr}` an arbitrary expression in a string literal.
Interpolated values must implement `Printable`. Write `$$` for a literal dollar sign.

```cs
void main() {
    var name = "world";
    println("hello $name!"); // prints "hello world!"
    println("1 + 2 = ${1 + 2}"); // prints "1 + 2 = 3"
    println("$$5"); // prints "$5"
}
```
