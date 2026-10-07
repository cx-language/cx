# Comparison with related languages

Many languages compete in the space of fast, natively compiled software, and each makes
different trade-offs:

- **C** is minimal, universal, and time-tested, but leaves much of the work, and many of
  the mistakes, to the programmer.
- **C++** is powerful and mature, but large and complex after decades of backwards
  compatibility and design-by-committee baggage.
- **Rust** guarantees memory and thread safety at compile time, at the cost of a more
  complex language and the work of satisfying the borrow checker, making it a poor fit for
  use cases where compilation speed and prototyping flexibility are more important.
- **Zig** favors explicit low-level code above all and is well-suited for that, but
  suffers from slow compile times, and is not ideal for more concise, high-level code.
- **Odin** and **Jai** are newer languages that rethink low-level programming around
  simplicity and direct control. They choose to forgo features such as compile-time null
  safety, method call syntax, and closures.

cx aims to balance the control and performance of a low-level language together with the
expressiveness of a higher-level one (compile-time null safety, method call syntax,
closures, generics with interface bounds, operator overloading, tagged unions, and a
capable standard library), while keeping the language small and approachable.

|           | cx                                                        | C++                                                  | Rust                           |
|-----------|-----------------------------------------------------------|------------------------------------------------------|--------------------------------|
| Null      | `T?` types with auto-narrowing                            | Pointers always nullable, `std::optional`, no safety | `Option`, explicit `Some(...)` |
| Sum types | Tagged unions via `enum`                                  | `std::variant` + visitor helper                      | Tagged unions via `enum`       |
| Errors    | `Result`                                                  | Exceptions, `std::expected`                          | `Result`                       |
| C interop | `import "foo.h"`, no bindings                             | Native                                               | Manual `extern` declarations   |
| Build     | `cx build`, implicit module tree                          | Headers + external build system                      | Cargo, explicit module tree    |
| Safety    | Debug checks + leak detector built in, sanitizers planned | Opt-in sanitizers                                    | Proven by the borrow checker   |

## cx vs C

cx is heavily based on C and is in many ways compatible with it:
C headers can be imported directly and cx functions can be declared `extern "C"` to be callable from C,
so cx can be adopted gradually alongside existing C code.

What cx adds over C:

- Memory-safety conveniences without garbage collection: array bounds checks,
  integer overflow checks, and non-null asserts. Enabled by default in development builds,
  disabled by default in release builds.
- Improved type safety: nullable types checked at compile time, type-safe tagged unions.
- Higher-level features: generics with interface bounds, function overloading,
  named arguments, closures, method call syntax, operator overloading,
  anonymous structs for returning multiple values, and `defer`.
- A richer standard library: real string types, array and string slices,
  resizable arrays, maps, sets, range-based algorithms, file system access,
  and Unicode support.
- A modern compilation model: no header files or forward declarations required,
  whole-program compilation, and a built-in build system (`cx build`) that
  works with no configuration and fetches Git dependencies automatically.
- No preprocessor, but conditional compilation using `#if` is supported.

**When to pick C:** the tradeoff is language/ecosystem maturity vs modern
expressiveness/developer ergonomics. C is standardized, time-tested, and available
everywhere, while cx is a still-evolving language with a richer feature set designed for
day-to-day productivity and joy of programming.

## cx vs C++

> "Within C++, there is a much smaller and cleaner language struggling to get out."
> - _Bjarne Stroustrup_

cx is trying to be that language.
It keeps what C++ gets right: deterministic destruction, generics, function
overloading, close-to-the-metal performance with no garbage collector, and
the principle of paying only for what you use. It changes the parts of C++
that cost productivity:

- No header files, forward declarations, or per-file compilation. cx compiles
  the whole program at once, which also removes slow linking of many object
  files.
- One initialization syntax instead of several, one member access operator
  (`.`, never `->`), and pointers only instead of the pointer/reference split.
- `switch` cases don't fall through, so there is no `break` to forget; an explicit
  `fallthrough` keyword is planned for the rare cases that need it.
- Types are non-nullable by default; nullable types are spelled `T?` and
  checked at compile time.
- No exceptions. Fallible functions return `Result`, and callers handle each
  case with `switch`.
- No implicit narrowing conversions: converting e.g. `int64` to `int` requires
  an explicit `int(...)`. Safe implicit conversions are allowed, and options to turn
  them into warnings or errors per project are planned.
- Moves are destructive, so moved-from objects need no resetting and their
  destructors don't run, unlike C++ move semantics.
- More optimization opportunities are planned: integer overflow that is undefined
  behavior in release builds (C++ only does this for signed integers), pointers declared
  non-aliasing individually or with a compiler flag, struct fields the compiler may
  reorder for a better layout unless prevented with an attribute or flag, and a type-safe
  reallocation function that `List` can grow with, which C++'s allocators lack.
- Interfaces serve as generic bounds (similar to C++ concepts) with readable
  errors on mismatch, and can additionally provide default implementations
  and fields.
- A standard build system and dependency manager are built in, instead of the
  fragmented CMake/conan/vcpkg ecosystem. Dependencies are plain Git
  repositories; there is no central registry.
- Tagged unions are a language feature (`enum` with payloads, matched with
  `switch`) instead of a library type like `std::variant`, which needs
  [tedious visitor boilerplate][std-visit] to use.
- No implicit deep copies through copy constructors and copy assignment
  operators; expensive operations are visible in the code.
- Debug builds check array bounds, integer overflow, and null dereferences by
  default, instead of leaving them to opt-in sanitizers.
- Less verbose syntax for common things: lambdas infer their parameter types
  (`n => n * 2`), named arguments label call sites, and there is no C-style
  cast syntax.
- The standard library covers more everyday needs: string and array slices,
  Unicode-aware strings, helpers such as `split` and `join`, and algorithms
  that take ranges instead of iterator pairs.

**When to pick C++:** cx imports C headers directly, but C++ API interop is a
longer-term goal, so C++ code currently needs C wrappers to be callable from
cx. And where C++ has decades of standard library, tooling, and compiler
maturity, cx is still young.

## cx vs Rust

Rust and cx share many features (tagged unions via `enum`, `Result`-based error handling,
generics with bounds, a standard build tool), but make opposite choices about how much
correctness to demand up front:

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
  cx instead reports many issues as warnings rather than errors, and lets each project
  tune which warnings it wants, e.g. with `--Wunused-result` or `--Wno-unused`.
  Options for warning about safe implicit conversions are planned too.

- Rust checks thread safety at compile time through `Send`/`Sync`.
  cx has no equivalent checking.

- To call C functions from Rust, the functions have to be declared in the Rust code.
  cx allows importing C headers directly.
  The compiler delegates to the Clang API to parse the C declarations from those headers.

- Rust's module system has been described as confusing[^rust-modules].
  cx's module system is designed to be simple, easy to understand, and straightforward to use:
  the compiler infers the module structure from the project's directory structure.
  Source files from the same module don't need to be explicitly imported.

- cx syntax is closer to the C/C++ syntax than Rust is.
  For C/C++ developers, this makes the learning curve of cx less steep and helps them get comfortable and productive more quickly.

- The naming conventions of the Rust language and standard libraries favor very short abbreviated names such as `Vec`, `str`, `i32`.
  cx uses less-abbreviated names such as `List`, `string`, `int32`.

- The Rust compiler warns about names that don't follow its snake_case and
  CamelCase conventions. cx doesn't enforce any naming convention or coding style.

- cx has potential for faster compile times than Rust, due to not having to do things like borrow checking.

- Rust doesn't allow function overloading. cx does, on both parameter types
  and parameter names, and additionally supports named arguments.

- Rust variables are immutable by default and need `mut` to be mutable.
  cx uses `var` for mutable variables and `const` for constants, with type
  inference in both cases.

- Rust forces every nullable value to be wrapped in `Some(...)`.
  cx implicitly converts `T` to `T?`, so plain values flow into nullable
  positions without wrapping.

- Rust traits and cx interfaces both bound generic parameters, and both
  support default method implementations.

- Rust has a powerful macro system (`macro_rules!` and procedural macros).
  cx has no macros, only `#if` conditions set from the command line.

- Both languages ship a standard build tool, but with different dependency
  models: Cargo builds on the central crates.io registry, while `cx build`
  fetches dependencies from plain Git URLs with no registry or publishing step.

**When to pick Rust:** if you need guaranteed memory or thread safety, pick Rust.

## cx vs Zig

Zig is a more low-level-focused language, competing primarily with C.
cx is more of a hybrid low-level/high-level language, competing primarily with C++.

- Zig doesn't have operator overloading due to its "no hidden control flow" principle.
  cx allows operator overloading.

- Zig doesn't have a dedicated string type or string concatenation operator.

- Zig doesn't have a method call syntax.

- Zig is very explicit about errors, allocation, `pub`ness, `const`ness.

- In Zig, types are values that are passed as `comptime` arguments, returned from `comptime` functions, etc.
  In other words, syntactically they are treated the same as runtime values.
  cx keeps types and runtime values separate, opting for a more familiar C++-like syntax.

- Zig's generics are duck-typed `comptime` parameters with no way to declare
  constraints. cx generics use angle brackets and can be constrained with
  interfaces, giving checked bounds and readable errors on mismatch.

- Zig has no automatic type narrowing for accessing a nullable value inside a matching null check,
  instead opting for an additional syntax: `if (optional_foo) |foo| { ... }`.

- Both languages import C declarations directly: Zig through `@cImport` and
  `translate-c`, cx through `import` of the header, parsed with the Clang API.

- Zig configures builds with a `build.zig` script. `cx build` needs no build
  file at all; optional settings live in `build.cx`, and dependencies are Git
  URLs.

**When to pick Zig:** if you want every allocation, error, and control-flow path spelled
out explicitly, or need Zig's cross-compilation toolchain, Zig is the better fit.

## cx vs Odin

Odin is the closest in spirit of the newer systems languages: pragmatic,
C-like, manually managed memory with no garbage collector or exceptions,
`defer`, multiple return values, and slices, maps, and dynamic arrays built
in. The differences:

- Odin spells nullable types `Maybe(T)` and handles them with `or_return` /
  `or_else` helpers and `switch` matching. cx spells them `T?`, checks them at compile
  time, narrows them with ordinary null checks, and offers the `!` operator for asserting
  non-null.
- Odin has neither methods nor operator overloading; procedures take the receiver as an
  explicit first argument. cx has both, plus named arguments.
- Odin generics are `$T` parameters with optional `where` clauses. cx
  generics use angle brackets with interface bounds, plus function
  overloading, which Odin replaces with explicit procedure groups.
- Odin interoperates with C through `foreign` blocks that redeclare each
  signature. cx imports C headers directly, parsed with the Clang API.
- Both have an implicit context. Odin's `context` carries an allocator, temporary
  allocator, and logger; cx's ambient context currently holds only the allocator, swapped
  with `withAllocator` (see [The ambient
  context](./low-level-programming#the-ambient-context)), and a fuller context like
  Odin's is planned.
- Both languages organize code by directory, but cx goes further: the whole
  project is one module, so files from the same project are never imported.
  Only external directories need `import`.
- Odin has no dependency fetching; libraries are vendored or added as
  submodules. `cx build` fetches Git dependencies automatically, with no
  central registry.
- cx has a syntax that is more familiar to C/C++ programmers, allowing them to adapt to
  the language more effortlessly.

**When to pick Odin:** Odin is further along: it is stable, documented, and used in
production. cx is younger and still evolving.

## cx vs Jai

- Jai is developed behind closed doors and distributed only through its
  beta program. cx is open source.

- cx has compile-time null-safety and nullable types.

- cx has method call syntax.

- Jai's polymorphic procedures take `$T` type parameters with no way to
  constrain them. cx generics can be bounded by interfaces, which documents
  requirements and produces readable errors on mismatch.

- cx has automatic importing of C headers, implemented using the Clang API.

- cx doesn't require importing standard library modules explicitly, or
  importing files from the same project: all cx files in the project source
  directory belong to the project by convention.

- Jai threads an implicit context parameter (allocators and such) through
  calls. cx's ambient context currently holds only the allocator (see [The
  ambient context](./low-level-programming#the-ambient-context)); a fuller context like
  Jai's is planned.

- Jai can run arbitrary code at compile time. cx has a narrower compile-time
  story: generics (including integer parameters) and constant expressions, with
  compile-time-evaluable functions and reflection for cases like iterating enum values
  planned.

- cx has a syntax that is more familiar to C/C++ programmers, allowing them to adapt to the language more effortlessly.

[std-visit]: https://bitbashing.io/std-visit.html

[^rust-modules]: <https://boats.gitlab.io/blog/post/2017-01-04-the-rust-module-system-is-too-confusing/>{target="_blank"}
