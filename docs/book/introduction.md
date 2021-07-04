# Introduction

C* (pronounced "C star") is a C-based hybrid low-level/high-level programming language focused on runtime performance and developer productivity, in this order of priority.
The language is simple and unopinionated, supporting imperative, generic, data-oriented, functional, and object-oriented programming.

## Design goals

The primary goals of C* are:
- Improve productivity and expressivity compared to C/C++.
- Improve performance of standard library and common programming patterns (such as runtime polymorphism) compared to C/C++.
- Fix various design flaws and footguns present in C/C++.
- Don't change too much from C/C++, especially in terms of syntax, so that C/C++ programmers are immediately productive in C*.
- Don't introduce any "big new ideas" or dogma. Instead, focus on what has been proven to work and implement those in the best way possible.

## Difference between C* and Rust

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
  C* uses less abbreviated names such as `List`, `string`, `int32`. 
  
- C* has potential for faster compile times than Rust, due to not having to do things like borrow checking.

- Rust doesn't allow function overloading. C* does.

## Difference between C* and Zig

- Zig doesn't have operator overloading due to its "no hidden control flow" principle.
  C* allows operator overloading.

- In Zig, types are values that are passed as `comptime` arguments, returned from `comptime` functions, etc.
  In other words, syntactically they are treated the same as runtime values.
  C* keeps types and runtime values separate, opting for a more familiar C++-like syntax.

- Zig is a more low-level-focused language, competing primarily with C.
  C* is more of a hybrid low-level/high-level language, competing primarily with C++.

## Difference between C* and Jai

- C* has compile-time null-safety and nullable types.
- C* has method call syntax.
- C* has interfaces, which can be used e.g. as type parameter bounds in generic code for better error messages.
- C* has automatic importing of C headers, implemented using the Clang API.
- C* doesn't require importing standard library modules explicitly.
- C* doesn't require importing files from the same project.
  All C* files in the project source directory are by default assumed to belong to the project as a "convention over configuration".
- C* is a simpler language.
- C* has a more familiar C-style syntax, which:
  - allows C/C++ programmers to be immediately productive with it
  - allows existing syntax tools such as syntax highlighters and code formatters to be used with little to no changes.





???:
- C* plans to allow building side-by-side with C/C++ code.




---

C* is a modern system programming language designed as an alternative to languages like C++, C, Rust, and Zig. The primary goals of C* are
programmer productivity and performance. Memory safety is also a priority, but while C* improves on what C++ and C have
to offer, it doesn't go as far as Rust at the detriment of ergonomics and productivity.

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

## Faster than C and C++

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

No hidden expensive operations, like implicit calls to copy constructors and copy assignment operators in C++.

## Improved syntax

C*'s syntax is clean, consistent, and similar to the C family of languages.

No C-style cast syntax, only C++-style casts and built-in type constructors.

### Examples of syntax

Syntax of a lambda returning the result of a single expression, with argument type inference:

```
// C++
a([](auto& b) { return b.c(); });

// C*
a(b -> b.c());
```

Function pointer declaration:

```
// C/C++
void (*f)(int);

// C*
void(int) f;
```

## Simple and expressive language

- Only structs instead of having both struct and class.
- Only pointers instead of having both pointers and references. Note that C*
  pointers are not identical to C/C++ pointers.
- Member access always uses `.`, never `->`.
- Switch statement cases break automatically, avoids bugs caused by missing
  `break` in C and C++. Fallthrough is opt-in. Variables can also be declared inside switch cases without requiring a
  block.
- Simpler to parse, making it easier to implement syntax highlighting, auto-formatting, linting, etc.
- Single-parameter constructors are not implicit by default.
- Only one initialization syntax instead of [three](https://en.cppreference.com/w/cpp/language/initialization).
- The `static` keyword is not overloaded with 3 different meanings. C* uses it only for non-instance members.
- No built-in preprocessor, at least not in the same form as C and C++. C*
  only has the conditional compilation directive `#ifdef` and its friends, which can be configured only from the
  command-line, not from source code as can be done in C and C++ with `#define`. Although there is no preprocessor, C*
  may get macros in some form if it turns out the language needs them.

Semantics are more in line with usual programmer expectations:

- Read-only use of `operator[]` on maps doesn't insert a default-constructed element.
- Inline functions are actually inlined, at least in debug mode.
- The integer types `int` and `uint` are at least 32 bits.
- String literals are of type `StringRef`, so they know their size and have all string manipulation member functions.
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

## Transparent interoperation with existing C APIs

C headers can be imported directly from C* code. The Clang API is used to parse the C headers and allow the contained
declarations to be accessed from C* code. C* functions can be declared `extern "C"` to enable calling them from C.

Support for the same level of interoperability with C++ APIs is a longer-term goal.

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

- More string manipulation functions, e.g. case-insensitive comparison, `split`,
  `join`, `startsWith`, `endsWith`.
- String and array slice types: `StringRef` and `ArrayRef`.
- Unicode-correct string type in addition to a raw byte string type.
- Algorithms take range objects instead of iterator pairs for ease of use.
- Iterable types in the standard library provide `map`, `filter`,
  `reduce`, etc. as member functions.
- File system library.
- Process library.
- Better math support:
    - Math functions can be evaluated at compile-time.
    - Standard library provides math constants such as Pi.

## Language features

### Destructuring

While iterating a map, the key and value from the key-value tuple can be destructured into separate variables:

```
for (var (key, value) in myMap) {
    ...
}
```

Destructure the return value of a function returning multiple values as a tuple:

```
(int, bool) foo() {
    var a = 42;
    var b = false;
    return (a, b);
}

void main() {
    var (a, b) = foo();
}
```

### Compile-time reflection

Should cover common use cases such as iteration over enum cases, getting enum case string representations, printing the
active type of a union, etc. Should follow the "pay only for what you use" principle.

### Named arguments

An optional way to avoid cryptic call sites like `foo(true, false)` by labeling the arguments with the corresponding
parameter name, with the compiler checking that the argument labels match the parameter names, e.g.
`foo(a: true, b: false)`. Consequently, named arguments don't have to be in same order as the parameters. This also
enables function overloading on parameter names.

### Defer statement

The `defer` keyword allows declaring arbitrary code, such as resource cleanup functions, to be executed when exiting the
current scope. In C++ one has to write an RAII wrapper class to achieve this.

### Type inference

Type inference for local and global variables, to improve productivity and make the code more amenable to type changes
when refactoring. The strong type system still has your back.

## …and all the good parts from C and C++

- Performance and control.
- Pay only for what you use.
- No garbage collection.
- Don't force the programmer to use a specific style.
- Have some implicit conversions and other features for convenience.
- Backwards compatibility eventually once the language design has stabilized.


[^rust-modules]: <https://boats.gitlab.io/blog/post/2017-01-04-the-rust-module-system-is-too-confusing/>{target="_blank"}
