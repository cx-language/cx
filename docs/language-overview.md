# Language overview

## Simple and expressive language

- Only structs instead of having both struct and class.
- Only pointers instead of having both pointers and references. Note that cx pointers are not identical to C/C++ pointers.
- Member access always uses `.`, never `->`.
- Switch statement cases break automatically, avoids bugs caused by missing `break` in C and C++. 
  Fallthrough is opt-in. Variables can also be declared inside switch cases without requiring a block.
- Simpler to parse, making it easier to implement syntax highlighting, auto-formatting, linting, etc.
- Single-parameter constructors are not implicit by default.
- Only one initialization syntax instead of [three](https://en.cppreference.com/w/cpp/language/initialization).
- The `static` keyword is not overloaded with 3 different meanings. cx uses it only for non-instance members.
- No built-in preprocessor, at least not in the same form as C and C++.
  cx only has the conditional compilation directive `#ifdef` and its friends, which can be configured only from the
  command-line, not from source code as can be done in C and C++ with `#define`. Although there is no preprocessor, cx
  may get macros in some form if it turns out the language needs them.
- No `const` (except for C interop).

Semantics are more in line with usual programmer expectations:

- Read-only use of `operator[]` on maps doesn't insert a default-constructed element.
- Inline functions are actually inlined, at least in debug mode.
- The integer types `int` and `uint` are at least 32 bits.
- String literals have a dedicated type, so they know their own size and have all the string manipulation member functions.
- Array parameters aren't silently converted into pointers.

## Safer by default

cx inserts at least the following safety checks by default:

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
- Using `interface`s (cx's equivalent of C++ concepts) allows defining what kind of types are valid for a given template
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

- cx's syntax is clean, consistent, and similar to the C family of languages.
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

## Standard build system and package manager

Building a cx project is done with a single command, `cx build`, which works out of the box without any build
configuration, and dependencies are fetched from Git repositories. See the [Build system](./build-system) page for details.

Long-term goal: include a built-in linting tool that runs during compilation to enforce a specific coding style, or to
disallow uses of certain language features.

## Faster than C++

More optimization opportunities:

- Moves are destructive, removing the need to reset moved-from objects and call their destructors, as is required in
  C++.
- Pointers can be declared non-aliasing either individually or globally with a compiler flag.
- The language provides a type-safe reallocation function for arrays, instead of just allocation and deallocation
  functions like C++. For example, the cx equivalent of C++'s `std::vector` makes use of this function.
- Unsigned integer overflow is undefined. There are still functions to do wrapping arithmetic on both signed and
  unsigned integers, when needed.
- Compiler is allowed to reorder struct fields. This can be prevented using an attribute or compiler flag.

cx uses the open-source LLVM library as a code generation back-end, benefiting from all current and future optimizations
implemented in LLVM.

No hidden expensive operations, such as implicit calls to copy constructors and copy assignment operators like in C++.

## Transparent interoperation with existing C APIs

C headers can be imported directly from cx code. The Clang API is used to parse the C headers and allow the contained
declarations to be accessed from cx code. cx functions can be declared `extern "C"` to enable calling them from C.

Support for some level of interoperability with C++ APIs is a longer-term goal.

## Additional language features

__Destructuring__: While iterating a map, the key and value from the key-value pair can be destructured into separate variables.
Functions can return multiple return values, which can then be destructured into separate variables at the call site.

__Compile-time reflection__ to cover common use cases such as iteration over enum cases, getting enum case string representations, printing the
active type of a union, etc. Should follow the "pay only for what you use" principle.

__Named arguments__ as an optional way to avoid cryptic call sites like `foo(true, false)` by labeling the arguments with the corresponding
parameter name, with the compiler checking that the argument labels match the parameter names, e.g.
`foo(verbose = true, ignoreErrors = false)`. Consequently, named arguments don't have to be in same order as the parameters. This also
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
