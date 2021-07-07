# Introduction

C* (pronounced "C star") is a C-based hybrid low-level/high-level general-purpose programming language.
It is designed for applications where runtime performance and programmer productivity matter.
The language is simple and unopinionated, supporting imperative, generic, data-oriented, functional, and object-oriented programming.
The language also aims to support interoperability and side-by-side usage with existing C/C++ code to allow gradual adoption.

## Goals

The primary goals of C* are:

- Improve productivity and expressivity compared to C/C++.
- Improve performance of standard library and common programming patterns (such as runtime polymorphism) compared to C/C++.
- Fix various design flaws and footguns present in C/C++.
- Don't change too much from C/C++, especially in terms of syntax, so that C/C++ programmers are immediately productive in C*.
- Don't introduce any "big new ideas" or dogma. Instead, focus on what has been proven to work and implement those in the best way possible.

## Why C*?

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
> — _Bjarne Stroustrup_

C* is trying to be that language.

Let's take a look at another popular systems programming language, Rust,
which also solves the majority of the above problems. Rust differs from C* in the following aspects:

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
  functions, or non-camel-case names for types. C*, like C++, should not
  force programmers to use a specific style.
- Forces the programmer to write very explicit code (e.g. [console I/O][6]).
- Numeric literals don't work well in generic code ([example][7]).

So in summary, C* is intended to be used over Rust for non-safety-critical
applications where programmer productivity, ergonomics, and performance are more
important than Rust's safety and explicitness.

## Simple and expressive language

- Only structs instead of having both struct and class.
- Only pointers instead of having both pointers and references. Note that C* pointers are not identical to C/C++ pointers.
- Member access always uses `.`, never `->`.
- Switch statement cases break automatically, avoids bugs caused by missing `break` in C and C++. 
  Fallthrough is opt-in. Variables can also be declared inside switch cases without requiring a block.
- Simpler to parse, making it easier to implement syntax highlighting, auto-formatting, linting, etc.
- Single-parameter constructors are not implicit by default.
- Only one initialization syntax instead of [three](https://en.cppreference.com/w/cpp/language/initialization).
- The `static` keyword is not overloaded with 3 different meanings. C* uses it only for non-instance members.
- No built-in preprocessor, at least not in the same form as C and C++.
  C* only has the conditional compilation directive `#ifdef` and its friends, which can be configured only from the
  command-line, not from source code as can be done in C and C++ with `#define`. Although there is no preprocessor, C*
  may get macros in some form if it turns out the language needs them.
- No `const` (except for C interop).

Semantics are more in line with usual programmer expectations:

- Read-only use of `operator[]` on maps doesn't insert a default-constructed element.
- Inline functions are actually inlined, at least in debug mode.
- The integer types `int` and `uint` are at least 32 bits.
- String literals have a dedicated type, so they know their own size and have all the string manipulation member functions.
- Array parameters aren't silently converted into pointers.

## Safer by default

C* inserts at least the following safety checks by default:

- Array bounds checks
- Integer overflow checks
- Null checks

These checks can be disabled individually or globally to make the code run faster.

The compiler warns if a variable might be read before being initialized, for example when passing it as an out parameter
to a C function. If you know the warning is safe to ignore, you can suppress it by assigning the keyword
`undefined` to the variable.

The compiler warns if a function's return value is ignored, by default. The function can be annotated to suppress this
behavior, if the return value is safe to ignore. The call site can also explicitly declare the return value as unused by
assigning it to `_`, instead of casting to void. `_` is a magic identifier which is also used in other contexts to
mean "ignore this value".

## Improved type system

Stronger typing to help avoid bugs and to make refactoring easier:

- No bug-prone implicit conversions between built-in types. Safe implicit conversions are still allowed, but may also be
  prevented with a compiler flag.
- All types are non-nullable by default. Nullable types are annotated with `?`.

Modern type system features for convenience and performance:

- Proper tagged union type to allow runtime polymorphism without dynamic allocation and virtual function calls.
  - C `union`: not type-safe, have to write a lot of wrapper code to call destructors and move/copy
    constructors/assignment operators.
  - C++17 `std::variant`: not easy or elegant to use (see
    https://bitbashing.io/std-visit.html).
- Using `interface`s (C*'s equivalent of C++ concepts) allows defining what kind of types are valid for a given template
  parameter when writing a generic type or function. This leads to much more readable error messages when instantiating
  generic types or functions with an incompatible type as the generic argument. C++-style unconstrained generic
  parameters are still allowed for flexibility.
- Arrays are first-class values that can be returned and passed by value. Arrays know their own size, either statically
  or at runtime.
- First-class tuples to allow e.g. returning multiple values or storing pairs of values in a container using a
  lightweight syntax.

## Standard library covers common use cases better

- More string manipulation functions, e.g. case-insensitive comparison, `split`, `join`, `startsWith`, `endsWith`.
- String and array slice types: `StringRef` and `ArrayRef`.
- Unicode-correct string type in addition to a raw byte string type.
- Algorithms take range objects instead of iterator pairs for ease of use.
- Iterable types in the standard library provide `map`, `filter`, `reduce`, etc. as member functions.
- File system library.
- Process library.
- Better math support:
  - Math functions can be evaluated at compile-time.
  - Standard library provides math constants such as Pi.

## Improved syntax

- C*'s syntax is clean, consistent, and similar to the C family of languages.
- No C-style cast syntax, only C++-style casts and built-in type constructors.
- Simpler lambda/closure syntax with argument type inference by default.
- More readable syntax for function pointer types.
- Array size is defined next to the array element type.
- Semicolons are optional.

## Better compilation model

- No header files, no forward declarations. API definitions can be generated from the source code if needed.
- Programs are compiled as a whole, instead of compiling each file separately and then linking them together. This
  should make compiling faster as it leaves more room for optimizing the build process, and linking is usually quite
  slow.
- Libraries are imported as a whole, instead of importing individual files from them, freeing the user of the library
  from having to figure out which file to import for a specific feature, and freeing the library author from keeping the
  library's file structure the same for backwards compatibility.
- Every library is automatically wrapped inside their own namespace, no need to explicitly declare the namespace in each
  source file.

All of the above should result in faster compilation times and increased programmer productivity compared to C and C++.

## Standard build system

Building a C* project is done with a single command: `cx build`, which works out of the box without a single line of
build configuration code. It finds all C* source files in the project directory (recursively) and compiles them into an
executable. Additional build information, such as dependencies, can be declared in a package configuration
file. `cx build` will then download or find these dependencies, and link them into the resulting executable.

No need to learn a new build system for each project, or manage project-specific build scripts, which is often the case
with C and C++ projects.

Long-term goal: include a built-in linting tool that runs during compilation to enforce a specific coding style, or to
disallow uses of certain language features.

## Standard package manager

Using and distributing third-party libraries should be made as easy as possible. For example, there should be no
platform-specific dependency or release management. Adding a dependency or publishing a library means running a single
command.

Initially supports fetching dependencies from Git repositories. Other version control systems, binary distributions,
system package managers, etc. will be supported later.

## Faster than C++

More optimization opportunities:

- Moves are destructive, removing the need to reset moved-from objects and call their destructors, as is required in
  C++.
- Pointers can be declared non-aliasing either individually or globally with a compiler flag.
- The language provides a type-safe reallocation function for arrays, instead of just allocation and deallocation
  functions like C++. For example, the C* equivalent of C++'s `std::vector` makes use of this function.
- Unsigned integer overflow is undefined. There are still functions to do wrapping arithmetic on both signed and
  unsigned integers, when needed.
- Compiler is allowed to reorder struct fields. This can be prevented using an attribute or compiler flag.

C* uses the open-source LLVM library as a code generation back-end, benefiting from all current and future optimizations
implemented in LLVM.

No hidden expensive operations, such as implicit calls to copy constructors and copy assignment operators like in C++.

## Transparent interoperation with existing C APIs

C headers can be imported directly from C* code. The Clang API is used to parse the C headers and allow the contained
declarations to be accessed from C* code. C* functions can be declared `extern "C"` to enable calling them from C.

Support for some level of interoperability with C++ APIs is a longer-term goal.

## Additional language features

__Destructuring__: While iterating a map, the key and value from the key-value pair can be destructured into separate variables.
Functions can return multiple return values, which can then be destructured into separate variables at the call site.

__Compile-time reflection__ to cover common use cases such as iteration over enum cases, getting enum case string representations, printing the
active type of a union, etc. Should follow the "pay only for what you use" principle.

__Named arguments__ as an optional way to avoid cryptic call sites like `foo(true, false)` by labeling the arguments with the corresponding
parameter name, with the compiler checking that the argument labels match the parameter names, e.g.
`foo(a: true, b: false)`. Consequently, named arguments don't have to be in same order as the parameters. This also
enables function overloading on parameter names.

__Defer statement__ to allow declaring arbitrary code, such as resource cleanup functions, to be executed when exiting the
current scope. In C++ one has to write an RAII wrapper class to achieve this.

__Simple type inference__ for local and global variables, to improve productivity and make the code more readable,
as well as more amenable to type changes when refactoring.
The strong type system ensures that type inference will not cause any unexpected typing issues.

## …and all the good parts from C and C++

- Performance and control.
- Pay only for what you use.
- No garbage collection.
- Don't force the programmer to use a specific style.
- Have some implicit conversions and other features for convenience. Allow the programmer to customize these to their preference via compiler warnings and/or linter options.
- Backwards compatibility eventually once the language design has stabilized.

## Design principles

- [Progressive disclosure](https://wiki.c2.com/?ProgressiveDisclosure)
  of complexity, i.e. it should be easy to write most of the code (the parts
  that aren't performance bottlenecks) to perform reasonably efficiently, and
  the language should provide full control to optimize the bottlenecks.
- Code compiled in release mode must be as fast as possible on modern hardware.
- Compilation in debug mode should be as fast as possible to speed up the
  edit-compile-run cycle during development.
- Be able to interoperate with C++ code to a certain degree, to avoid having to
  write a C wrapper for everything. However, this should not restrict the design
  of the language.
- Safe by default, but allow disabling safety checks (both individually and
  collectively) easily where necessary. E.g. array access bounds checking,
  integer overflow detection.
- The language should be relatively simple, but most of the perceived
  simplicity should come from the aforementioned progressive disclosure
  principle. For simple things: "There should be one — and preferably only one
  — obvious way to do it."
- Should scale well, e.g. fast build times, easy greppability of function/type/variable definitions.
- Should be familiar and attractive to C++ and C developers. I.e. try not to
  change too much. Only change things that are an objective improvement from C++
  or otherwise well justified.

## Comparison with related projects

### Difference between C* and Rust

- Compile-time memory safety is useful for certain types of projects,
  but it comes at a cost in language complexity (e.g. lifetime annotations)
  and productivity (e.g. having to spend extra development time to satisfy the borrow checker).
  This is not desirable in all projects, and for them C* might be more appropriate.
  C*'s long-term goal for ensuring memory safety is to support proven runtime verification tools such as AddressSanitizer.
  These tools could be built in to the compiler to make it easy to enable them e.g. for debug builds.

  Memory safety violations caught by AddressSanitizer are probably also easier to understand than the sometimes very cryptic
  compile errors produced by Rust's borrow checker, since you can see the exact runtime conditions that caused the
  violation in your debugger.

- Rust code is by design very explicit.
  While this is useful for code where you care about every little detail, every instance of possible runtime overhead, and every error condition,
  it is counterproductive for code where you don't care about such things.
  To solve this divide, C* allows the programmer to configure individual compiler warnings.
  For example, warnings for safe implicit conversions can be enabled or disabled based on the project's requirements.
  This is a common theme in C*: the compiler is adaptable to the user's needs.

- To call C functions from Rust, the functions have to be declared in the Rust code.
  C* allows importing C headers directly.
  The compiler delegates to the Clang API to parse the C declarations from those headers.

- Rust's module system has been described as confusing[^rust-modules].
  C*'s module system is designed to be simple, easy to understand, and straightforward to use:
  the compiler infers the module structure from the project's directory structure.
  Source files from the same module don't need to explicitly imported.

- C* syntax is closer to the C/C++ syntax than Rust is.
  For C/C++ developers, this makes the learning curve of C* less steep and helps them get comfortable and productive more quickly.

- The naming conventions of the Rust language and standard libraries favor very short abbreviated names such as `Vec`, `str`, `i32`.
  C* uses less-abbreviated names such as `List`, `string`, `int32`.

- C* has potential for faster compile times than Rust, due to not having to do things like borrow checking.

- Rust doesn't allow function overloading. C* does.

### Difference between C* and Zig

- Zig doesn't have operator overloading due to its "no hidden control flow" principle.
  C* allows operator overloading.

- Zig doesn't have a dedicated string type or string concatenation operator.

- Zig doesn't have a method call syntax.

- Zig is very explicit about errors, allocation, `pub`ness, `const`ness.

- In Zig, types are values that are passed as `comptime` arguments, returned from `comptime` functions, etc.
  In other words, syntactically they are treated the same as runtime values.
  C* keeps types and runtime values separate, opting for a more familiar C++-like syntax.

- Zig is a more low-level-focused language, competing primarily with C.
  C* is more of a hybrid low-level/high-level language, competing primarily with C++.

- Zig has no automatic type narrowing for accessing a nullable value inside a matching null check, 
  instead opting for an additional syntax: `if (optional_foo) |foo| { ... }`

### Difference between C* and Jai

- C* has compile-time null-safety and nullable types.

- C* has method call syntax.

- C* has interfaces, which can be used e.g. as type parameter bounds in generic code for better error messages.

- C* has automatic importing of C headers, implemented using the Clang API.

- C* doesn't require importing standard library modules explicitly.

- C* doesn't require importing files from the same project.
  All C* files in the project source directory are by default assumed to belong to the project as a "convention over configuration".

- C* has a more familiar C-style syntax, which:
  - allows C/C++ programmers to be immediately productive with it
  - allows existing syntax tools such as syntax highlighters and code formatters to be used with little to no changes.


## Resources for further C* language design

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

[^rust-modules]: <https://boats.gitlab.io/blog/post/2017-01-04-the-rust-module-system-is-too-confusing/>{target="_blank"}
