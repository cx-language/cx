# Strings

C* has two string types:

- `string`, a reference to a sequence of characters, and the type of string literals.\
  Use it when you need to pass a string to a function without mutating it.\
  It's like `std::string_view` in C++ and `&str` in Rust.
- `StringBuffer`, a buffer storing a sequence of characters.\
  Use it when you need to build dynamic strings.\
  It's like `std::string` in C++ and `String` in Rust.

```cs
void main() {
    var a = "test";
    // a has type 'string'

    var b = StringBuffer(a);
    // b holds a copy of "test" that we can mutate

    b.push('!');
    // b now hold "test!", a is unchanged

    println(a);
    println(b);
}
```
