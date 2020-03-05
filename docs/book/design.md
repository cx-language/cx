# Delta Language Design Document

This document describes the design decisions and rationales for the Delta
programming language, and provides guidelines and resources for further design.

__Note:__ This document is incomplete.

## Introduction

> Within C++, there is a much smaller and cleaner language struggling to get out.
>
> — Bjarne Stroustrup

Delta is a general-purpose programming language with a bias towards systems
programming, intended primarily as a replacement for C++ and C. It is designed
for applications where runtime performance and programmer productivity matter.
It should also support large-scale development, and limited interoperability
with existing C++ and C code to make gradual adoption feasible.

### Principles

- [Progressive disclosure](https://en.wikipedia.org/wiki/Progressive_disclosure)
  of complexity, i.e. it should be easy to write most of the code (the parts
  that aren't performance bottlenecks) to perform reasonably efficiently, and
  the language should provide full control to optimize the bottlenecks.
- Code compiled in release mode must be as fast as possible on modern hardware.
- Compilation in debug mode should be as fast as possible to speed up the
  edit-compile-run cycle during development.
- Be able to interoperate with C++ code to a certain degree, to avoid having to
  write a C wrapper for everything. However this should not restrict the design
  of the language.
- Safe by default, but allow disabling safety checks (both individually and
  collectively) easily where necessary. E.g. array access bounds checking,
  integer overflow detection.
- The language should be relatively simple, but most of the perceived
  simplicity should come from the aforementioned progressive disclosure
  principle. For simple things: "There should be one — and preferably only one
  — obvious way to do it."
- Should support large-scale development, e.g. fast build times, strong type
  system, easy greppability of function/type/variable definitions.
- Should be familiar and attractive to C++ and C developers. I.e. try not to
  change too much. Only change things that are an objective improvement from C++
  or otherwise well justified.

## Why Delta instead of C++ or Rust?

C++ is a large and complex language that has accumulated many problems over the
years, for example:

- No standard build system.
- No standard way to manage dependencies.
- Header files and the code duplication they cause.
- Functions and types must be declared before using them, resulting in forward
  declarations as a language feature.
- Source code compatibility with C.
- Slow compilation times for large projects.
- Syntax is overly verbose in some cases (think lambdas, smart pointers).
- No compile-time reflection (ever wanted to iterate over the values of an
  enum?).
- Too many implicit conversion between data types, which cause bugs too easily.
- Mutability by default.
- No proper tagged union type (no, `std::variant` [doesn't count][8]).
- Poor support for functional programming.
- Deep copying is implicit, while moving is explicit (in most cases).
- Switch case-statement fallthrough is implicit, requires explicit `break` to
  avoid.
- No named parameters.
- Writing cross-platform code is harder than it needs to be (mostly as a result
  of different platforms and compilers doing things differently).
- Has both pointers and references, and problems associated with both of them
  (e.g. no way to express non-nullable reassignable pointers, without resorting
  to a class like `std::reference_wrapper`). No non-nullable smart pointers.
- String literals have array type (should be string slices).
- No array/vector range checks in debug builds.
- Standard library severely lacking:
    - No standard file system API (added in C++17).
    - No Unicode-aware string type.
    - No string slice type (added in C++17).
    - No array slice type.
    - Some very common functionality missing, like a string split function.
    - All standard library algorithms take pairs of iterators rather than ranges.
- And many more minor design flaws kept for backwards compatibility.

It is clear that we can't keep adding more and more features to C++ and writing
new code in it forever. Sooner or later it will be replaced by another language
or languages (at least for writing new code).

Delta is designed to solve all of the above problems. Let's take a look at
another popular systems programming language, Rust, which also solves (almost)
all of the above problems (excluding e.g. the lack of named parameters). Rust
differs from Delta in the following aspects:

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
  abbreviated names, instead of clear unabbreviated ones.
- The Rust compiler complains if you use non-snake-case names for variables or
  functions, or non-camel-case names for types. Delta, like C++, should not
  force programmers to use a specific style.
- Forces the programmer to write very explicit code (e.g. [console I/O][6]).
- Numeric literals don't work well in generic code ([example][7]).

So in summary, Delta is intended to be used over Rust for non-safety-critical
applications where programmer productivity, ergonomics, and performance are more
important than Rust's safety and explicitness.

## Resources

- ["What are the weakest points of C++ in your opinion?" on Reddit](https://www.reddit.com/r/cpp/comments/7lvteh/what_are_the_weakest_points_of_c_in_your_opinion/?st=JBM8MFRN&sh=30098ea8)
- ["What would you change in C++ if backwards compatibility was not an issue?" on Reddit][1]
- ["Let's stop copying C" by Eevee][2]
- ["Hypothetically, which standard library warts would you like to see fixed in a "std2"?" on Reddit][3]
- ["If you had the power to completely overhaul C++, what would you change?" on Reddit][4]
- ["Considerations for programming language design: a rebuttal"][5]

[1]: https://www.reddit.com/r/cpp/comments/7639sf/what_would_you_change_in_c_if_backwards/?ref=share&ref_source=link
[2]: https://eev.ee/blog/2016/12/01/lets-stop-copying-c/
[3]: https://www.reddit.com/r/cpp/comments/4py6sl/hypothetically_which_standard_library_warts_would/?ref=share&ref_source=link
[4]: https://www.reddit.com/r/cpp/comments/2e52t4/if_you_had_the_power_to_completely_overhaul_c/?ref=share&ref_source=link
[5]: https://hackernoon.com/considerations-for-programming-language-design-a-rebuttal-5fb7ef2fd4ba
[6]: https://www.reddit.com/r/rust/comments/3kmrl8/new_to_rust_the_language_seems_really_verbose_is/?ref=share&ref_source=link
[7]: https://www.reddit.com/r/rust/comments/2zu3eo/what_is_rust_bad_at/cpmgbrx/
[8]: https://bitbashing.io/std-visit.html
