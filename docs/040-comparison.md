# Comparison with related languages

## cx vs. C

cx is heavily based on C and is in many ways compatible with it:
C headers can be imported directly and cx functions can be declared `extern "C"` to be callable from C,
so cx can be adopted gradually alongside existing C code.

What cx adds over C:

- Memory-safety conveniences without garbage collection: array bounds checks,
  integer overflow checks, and non-null asserts. Enabled by default in development builds,
  disabled by default in release builds.
- Improved type safety: nullable types checked at compile time, type-safe tagged unions.
- Higher-level features: generics with interface bounds, function overloading,
  named arguments, closures, method call syntax, operator overloading, sum types,
  tuples, multiple return values, and `defer`.
- A richer standard library: real string types, array and string slices,
  resizable arrays, maps, sets, range-based algorithms, file system access,
  and Unicode support.
- A modern compilation model: no header files or forward declarations required,
  whole-program compilation, and a built-in build system (`cx build`) that
  works with no configuration and fetches Git dependencies automatically.
- No preprocessor, but conditional compilation using `#if` is supported.

The tradeoff is language/ecosystem maturity vs modern expressiveness/developer ergonomics:
C is standardized, time-tested, and available everywhere, while cx is a still-evolving language
with a richer feature set designed for day-to-day productivity and joy of programming.

## Difference between cx and C++

cx keeps what C++ gets right: deterministic destruction, generics, function
overloading, close-to-the-metal performance with no garbage collector, and
the principle of paying only for what you use. It changes the parts of C++
that cost productivity:

- No header files, forward declarations, or per-file compilation. cx compiles
  the whole program at once, which also removes slow linking of many object
  files.
- One initialization syntax instead of several, one member access operator
  (`.`, never `->`), and pointers only instead of the pointer/reference split.
- `switch` cases break by default with opt-in fallthrough.
- Types are non-nullable by default; nullable types are spelled `T?` and
  checked at compile time.
- No exceptions. Fallible code currently uses nullable returns and status
  codes; a `Result` type is planned.
- No bug-prone implicit narrowing conversions. Safe implicit conversions are
  allowed but can be turned into warnings or errors per project.
- Moves are destructive, so moved-from objects need no resetting and their
  destructors don't run, unlike C++ move semantics.
- The compiler may reorder struct fields for better layout, unless prevented
  with an attribute or flag.
- Interfaces serve as generic bounds (similar to C++ concepts) with readable
  errors on mismatch, and can additionally provide default implementations
  and fields.
- A standard build system and dependency manager are built in, instead of the
  fragmented CMake/conan/vcpkg ecosystem. Dependencies are plain Git
  repositories; there is no central registry.

One caveat: cx imports C headers directly, but C++ API interop is a
longer-term goal, so C++ code currently needs C wrappers to be callable from
cx. And where C++ has decades of standard library, tooling, and compiler
maturity, cx is still young.

|           | cx                                 | C++                                                  | Rust                           |
|-----------|------------------------------------|------------------------------------------------------|--------------------------------|
| Null      | `T?` types with auto-narrowing     | Pointers always nullable, `std::optional`, no safety | `Option`, explicit `Some(...)` |
| Sum types | Tagged unions via `enum`           | `std::variant` + visitor helper                      | Tagged unions via `enum`       |
| Errors    | `Result`                           | Exceptions, `std::expected`                          | `Result`                       |
| C interop | `import "foo.h"`, no bindings      | Native                                               | Manual `extern` declarations   |
| Build     | `cx build`, implicit module tree   | Headers + external build system                      | Cargo, explicit module tree    |
| Safety    | Debug checks + sanitizers built in | Opt-in sanitizers                                    | Proven by the borrow checker   |

## cx vs Rust

- If you need guaranteed memory safety, pick Rust.

- Compile-time memory safety is useful for certain types of projects,
  but it comes at a cost in language complexity (e.g. lifetime annotations)
  and productivity (e.g. having to spend extra development time to satisfy the borrow checker).
  This is not desirable in all projects, and for them cx might be more appropriate.
  cx's long-term goal for ensuring memory safety is to support proven runtime verification tools such as AddressSanitizer.
  These tools could be built in to the compiler to make it easy to enable them e.g. for debug builds.

  Memory safety violations caught by AddressSanitizer are probably also easier to understand than the sometimes very cryptic
  compile errors produced by Rust's borrow checker, since you can see the exact runtime conditions that caused the
  violation in your debugger.

- Rust tries to make you write the correct code the first time.
  If that code turns out to be the wrong approach and gets deleted,
  all the time spent making it correct was wasted.
  cx lets you write quick, hacky, even incorrect code to reach the design decision faster:
  if you discard it, you haven't sunk much effort into making it correct.
  And nothing stops you from making the surviving code correct afterwards.

- Rust code is by design very explicit.
  While this is useful for code where you care about every little detail, every instance of possible runtime overhead, and every error condition,
  it is counterproductive for code where you don't care about such things.
  To solve this divide, cx allows the programmer to configure individual compiler warnings.
  For example, warnings for safe implicit conversions can be enabled or disabled based on the project's requirements.
  This is a common theme in cx: the compiler is adaptable to the user's needs.

- To call C functions from Rust, the functions have to be declared in the Rust code.
  cx allows importing C headers directly.
  The compiler delegates to the Clang API to parse the C declarations from those headers.

- Rust's module system has been described as confusing[^rust-modules].
  cx's module system is designed to be simple, easy to understand, and straightforward to use:
  the compiler infers the module structure from the project's directory structure.
  Source files from the same module don't need to explicitly imported.

- cx syntax is closer to the C/C++ syntax than Rust is.
  For C/C++ developers, this makes the learning curve of cx less steep and helps them get comfortable and productive more quickly.

- The naming conventions of the Rust language and standard libraries favor very short abbreviated names such as `Vec`, `str`, `i32`.
  cx uses less-abbreviated names such as `List`, `string`, `int32`.

- cx has potential for faster compile times than Rust, due to not having to do things like borrow checking.

- Rust doesn't allow function overloading. cx does.
- Rust variables are immutable by default and need `mut` to be mutable.
  cx uses `var` for mutable variables and `const` for constants, with type
  inference in both cases.

- Rust forces every nullable value to be wrapped in `Some(...)`.
  cx implicitly converts `T` to `T?`, so plain values flow into nullable
  positions without wrapping.

- Rust traits and cx interfaces both bound generic parameters, but cx
  interfaces can also declare fields with defaults, so unrelated types can
  share state as well as behavior. Rust traits cannot carry state.

- Rust has a powerful macro system (`macro_rules!` and procedural macros).
  cx has no macros, only `#ifdef` conditions set from the command line.

- Rust checks thread safety at compile time through `Send`/`Sync`.
  cx has no equivalent checking.

- Both languages ship a standard build tool, but with different dependency
  models: Cargo builds on the central crates.io registry, while `cx build`
  fetches dependencies from plain Git URLs with no registry or publishing step.

- Rust doesn't allow function overloading. cx does, on both parameter types
  and parameter names, and additionally supports named arguments.

## cx vs Zig

- Zig doesn't have operator overloading due to its "no hidden control flow" principle.
  cx allows operator overloading.

- Zig doesn't have a dedicated string type or string concatenation operator.

- Zig doesn't have a method call syntax.

- Zig is very explicit about errors, allocation, `pub`ness, `const`ness.

- In Zig, types are values that are passed as `comptime` arguments, returned from `comptime` functions, etc.
  In other words, syntactically they are treated the same as runtime values.
  cx keeps types and runtime values separate, opting for a more familiar C++-like syntax.

- Zig is a more low-level-focused language, competing primarily with C.
  cx is more of a hybrid low-level/high-level language, competing primarily with C++.

- Zig has no automatic type narrowing for accessing a nullable value inside a matching null check,
  instead opting for an additional syntax: `if (optional_foo) |foo| { ... }`
- In Zig, types are values that are passed as `comptime` arguments, returned from `comptime` functions, etc.
  In other words, syntactically they are treated the same as runtime values.
  cx keeps types and runtime values separate, opting for a more familiar C++-like syntax.

- Zig's generics are duck-typed `comptime` parameters with no way to declare
  constraints. cx generics use angle brackets and can be constrained with
  interfaces, giving checked bounds and readable errors on mismatch.

- Both languages import C declarations directly: Zig through `@cImport` and
  `translate-c`, cx through `import` of the header, parsed with the Clang API.

- Zig configures builds with a `build.zig` script. `cx build` needs no build
  file at all; optional settings live in `build.cx`, and dependencies are Git
  URLs.

## cx vs Jai

- cx has compile-time null-safety and nullable types.

- cx has method call syntax.

- cx has interfaces, which can be used e.g. as type parameter bounds in generic code for better error messages.

- cx has automatic importing of C headers, implemented using the Clang API.

- cx doesn't require importing standard library modules explicitly.

- cx doesn't require importing files from the same project.
  All cx files in the project source directory are by default assumed to belong to the project as a "convention over configuration".

- cx has a syntax that is more familiar to C/C++ programmers, allowing them to adapt to the language more effortlessly.
- Jai is developed behind closed doors and distributed only through its
  beta program. cx is open source.

- Jai's polymorphic procedures take `$T` type parameters with no way to
  constrain them. cx generics can be bounded by interfaces, which documents
  requirements and produces readable errors on mismatch.

- Jai threads an implicit context parameter (allocators and such) through
  calls. cx has no implicit context; containers allocate on their own.

- Jai can run arbitrary code at compile time. cx has a narrower compile-time
  story: generics, compile-time-evaluable functions, and planned reflection
  for cases like iterating enum values.

## cx vs Odin

- cx has compile-time null-safety and nullable types with automatic narrowing.
  Odin uses `Maybe(T)` with explicit handling.

- cx has method call syntax.
  Odin has no methods; procedures take the receiver as an explicit first argument.

- cx has automatic importing of C headers, implemented using the Clang API.

- cx has a syntax that is more familiar to C/C++ programmers, allowing them to adapt to the language more effortlessly.

## Difference between cx and Odin

Odin is the closest in spirit of the newer systems languages: pragmatic,
C-like, manually managed memory with no garbage collector or exceptions,
`defer`, multiple return values, and slices, maps, and dynamic arrays built
in. The differences:

- Odin threads an implicit `context` (allocator, temporary allocator, logger)
  through every procedure call. cx has no implicit context; standard
  containers allocate on their own.
- Odin spells nullable types `Maybe(T)` and handles them with `or_return` /
  `or_else` helpers and `switch` matching. cx spells them `T?`, narrows them
  with ordinary null checks, and offers the `!` operator for asserting non-null.
- Odin interoperates with C through `foreign` blocks that redeclare each
  signature. cx imports C headers directly, parsed with the Clang API.
- Odin generics are `$T` parameters with optional `where` clauses. cx
  generics use angle brackets with interface bounds, plus function
  overloading, which Odin replaces with explicit procedure groups.
- Odin has neither methods nor operator overloading. cx has both, plus named
  arguments.
- Both languages organize code by directory, but cx goes further: the whole
  project is one module, so files from the same project are never imported.
  Only external directories need `import`.
- Odin has no dependency fetching; libraries are vendored or added as
  submodules. `cx build` fetches Git dependencies automatically, with no
  central registry.
- Odin is further along: it is stable, documented, and used in production.
  cx is younger and still evolving.

[^rust-modules]: <https://boats.gitlab.io/blog/post/2017-01-04-the-rust-module-system-is-too-confusing/>{target="_blank"}
