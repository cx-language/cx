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

## Non-goals

cx explicitly does not aim for:

- Guaranteed compile-time memory or thread safety. That is Rust's trade-off:
  cx prefers a simpler language and faster iteration, catching memory bugs with
  debug-mode checks and tooling such as AddressSanitizer instead
  (see [Difference between cx and Rust](./comparison#difference-between-cx-and-rust)).
- A managed runtime or garbage collector. There is no hidden runtime;
  the program you write is the program that runs.
- Source compatibility with C or C++. cx interoperates with existing C code
  (C++ interop is a longer-term goal) so projects can adopt it gradually,
  but it does not keep C/C++ syntax or semantics where they are flawed.

## Who cx is for

cx is a general-purpose language, but it is designed especially for game
development and other areas where you want both low-level control and
high-level expressiveness in one language - domains where debug performance and
quick prototyping matter. Debug builds compile fast for a short
edit-compile-run cycle (see [Build modes](./build-system#build-modes)), and
nothing stops you from writing quick, hacky code to reach a design decision
faster and making the surviving code correct afterwards
(see [Difference between cx and Rust](./comparison#difference-between-cx-and-rust)).

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

## Frequently asked questions

### Do I need to import the standard library or other files in my project?

No. The standard library is visible to all code automatically, and all `.cx`
files in your project (except `build.cx` and vendored libraries under `vendor/`)
compile as one module, so neither needs `import`. Only external code needs
importing: dependencies, vendored libraries, and other directories outside your
project. See [Modules and imports](./modules).

### What does "no hidden allocation or runtime" mean? Do containers allocate?

There is no garbage collector or language runtime allocating behind your back.
Allocating is confined to the container types you choose (`List`, `Map`,
`StringBuf`, ...), which manage their own memory without allocator plumbing.
For full control over allocation, use fixed-size arrays, slices, `malloc`/`free`,
or an `Arena`. See [Low-level programming](./low-level-programming).

### Is cx memory-safe?

cx is safer by default than C/C++: array accesses are bounds-checked, integer
arithmetic is overflow-checked, null dereferences are checked, and the type
system tracks nullability at compile time. But unlike Rust, cx does not
guarantee memory or thread safety at compile time; for that, use debug-mode
checks and tooling such as AddressSanitizer. See [Safer by default](./language-overview#safer-by-default)
and [Difference between cx and Rust](./comparison#difference-between-cx-and-rust).

### How do I build a project and add dependencies?

Run `cx build` in the project directory; it works with no configuration.
Dependencies are Git repositories declared in `build.cx` and imported by package
name, with no central registry. Run tests with `cx test`. See [Build system](./build-system).

### How do I use existing C or C++ code?

Import C headers directly (`import "stdio.h";`) and call their functions like
ordinary cx functions; individual C functions can also be declared with bare
`extern`. C++ APIs need C wrappers for now; direct C++ interop is a longer-term goal.
See [Using C libraries](./low-level-programming#using-c-libraries).

### Which build mode should I use?

The default debug mode compiles fast with safety checks enabled. Use `--release`
for fully optimized builds with checks disabled (overflow wraps), or
`--release-safe` for optimized builds that keep the checks. See [Build modes](./build-system#build-modes).

### There is no preprocessor or macros - how do I conditionally compile code?

Use `#if` with `-D` defines and the predefined `Windows`/`macOS` conditions,
both in source files and in `build.cx`. See [Conditional compilation](./build-system#conditional-compilation).

### What does "simple and unopinionated" mean in practice?

The language stays small (one struct concept, one member-access operator, no
pointer/reference split), there is one obvious way to do simple things, and the
compiler never forces a naming convention or coding style on you. Multiple
paradigms are supported equally. See [Design principles](./design-principles).

### What is cx not for?

Safety-critical code where compile-time guarantees matter more than iteration
speed, and projects that need a stable, mature ecosystem today:
cx is still evolving toward its 0.1 release. See [Non-goals](#non-goals).

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
