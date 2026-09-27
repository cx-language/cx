# Introduction

cx is a C-based hybrid low-level/high-level general-purpose programming language.
It is designed for applications where runtime performance and programmer productivity matter.
The language is simple and unopinionated, supporting imperative, generic, data-oriented, functional, and object-oriented programming.
The language also aims to support interoperability and side-by-side usage with existing C/C++ code to allow gradual adoption.

## Goals

The primary goals of cx are:

- Improve productivity and expressivity compared to C/C++.
- Improve performance of standard library and common programming patterns (such as runtime polymorphism) compared to C/C++.
- Fix various design flaws and footguns present in C/C++.
- Don't change too much from C/C++, especially in terms of syntax, so that C/C++ programmers are immediately productive in cx.
- Don't introduce any "big new ideas" or dogma. Instead, focus on what has been proven to work and implement those in the best way possible.

## Why cx?

C++ is a huge and complex language that has accumulated many problems over the years, for example:

- No standard build system.
- No standard way to manage dependencies.
- Header files and the code duplication they cause.
- Functions and types must be declared before using them, resulting in forward declarations as a language feature.
- Source code compatibility with C.
- Slow compilation times for large projects.
- Syntax is overly verbose in some cases (think lambdas, smart pointers).
- No compile-time reflection (ever wanted to iterate over the values of an enum?).
- Too many implicit conversion between data types, which cause bugs too easily.
- No proper tagged union type (`std::variant` [is too tedious][8]).
- Poor support for functional programming.
- Deep copying is implicit, while moving is explicit (in most cases).
- Switch case-statement fallthrough is implicit, requires explicit `break` to avoid.
- No named parameters.
- Writing cross-platform code is harder than it needs to be (mostly as a result of different platforms and compilers doing things differently).
- Has both pointers and references, and problems associated with both of them
  (e.g. no way to express non-nullable reassignable pointers, without resorting
  to a class like `std::reference_wrapper`). No non-nullable smart pointers.
- String literals have array type (should be string slices).
- No buffer or arithmetic overflow checks in debug builds.
- Standard library severely lacking:
  - No standard file system API (added in C++17).
  - No Unicode-aware string type.
  - No string slice type (added in C++17).
  - No array slice type.
  - Some very common functionality missing, like a string split function.
  - All standard library algorithms take pairs of iterators rather than ranges.
- Many minor design flaws kept for backwards compatibility.
- ... and [more](http://www.yosefk.com/c++fqa/defective.html).

It is clear that we can't keep adding more and more features to C++ and writing
new code in it forever. Sooner or later it will be replaced by another language
or languages (for writing new code).

> "Within C++, there is a much smaller and cleaner language struggling to get out."
> - _Bjarne Stroustrup_

cx is trying to be that language.

Let's take a look at another popular systems programming language, Rust,
which also solves the majority of the above problems. Rust differs from cx in the following aspects:

- Guaranteed memory safety is useful for certain types of projects, but for
  non-safety-critical projects it comes at a cost in productivity (e.g. fighting
  the borrow checker) and language complexity (e.g. lifetime annotations). Also
  for preventing bugs caused by memory unsafety, there are great tools like
  AddressSanitizer which help enormously (and could be built in to the language
  itself).
- Syntax deviates too much from C++ for little or no benefit.
- Module system could be better (e.g. to not have to explicitly declare a
  project's module structure in the source code instead of the compiler
  automatically determining it from the project's directory structure).
- The language is verbose in some cases. (e.g. `let mut` instead of `var`, etc.).
- Too few implicit conversions (e.g. cannot assign value of type `T` directly to
  a variable of type `Option<T>`, instead must wrap it in `Some(...)`).
- Naming convention for standard library and language keywords favors cryptic
  abbreviated names, instead of clear non-abbreviated ones.
- The Rust compiler complains if you use non-snake-case names for variables or
  functions, or non-camel-case names for types. cx, like C++, should not
  force programmers to use a specific style.
- Forces the programmer to write very explicit code (e.g. [console I/O][6]).
- Numeric literals don't work well in generic code ([example][7]).

So in summary, cx is intended to be used over Rust for non-safety-critical
applications where programmer productivity, ergonomics, and performance are more
important than Rust's safety and explicitness.

## Not to be confused with

cx shares its name with several unrelated languages:

- [CX](https://github.com/skycoin/cx), Skycoin's general-purpose language with Go-like syntax.
- [Cx](https://github.com/fernandothedev/cx), a language that transpiles to C99.
- [Cx](https://github.com/COMMENTERTHE9/Cx_lang), a GC-free systems language for game engines.
- [C++/CX](https://learn.microsoft.com/en-us/cpp/cppcx/type-system-c-cx?view=msvc-170&redirectedfrom=MSDN), Microsoft's C++ extensions for the Windows Runtime.

## Further reading

- [Language overview](./language-overview)
- [Design principles](./design-principles)
- [Comparison with related projects](./comparison)

[6]: https://www.reddit.com/r/rust/comments/3kmrl8/new_to_rust_the_language_seems_really_verbose_is/?ref=share&ref_source=link
[7]: https://www.reddit.com/r/rust/comments/2zu3eo/what_is_rust_bad_at/cpmgbrx/
[8]: https://bitbashing.io/std-visit.html
