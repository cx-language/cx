# Comparison with related projects

## Difference between cx and Rust

- Compile-time memory safety is useful for certain types of projects,
  but it comes at a cost in language complexity (e.g. lifetime annotations)
  and productivity (e.g. having to spend extra development time to satisfy the borrow checker).
  This is not desirable in all projects, and for them cx might be more appropriate.
  cx's long-term goal for ensuring memory safety is to support proven runtime verification tools such as AddressSanitizer.
  These tools could be built in to the compiler to make it easy to enable them e.g. for debug builds.

  Memory safety violations caught by AddressSanitizer are probably also easier to understand than the sometimes very cryptic
  compile errors produced by Rust's borrow checker, since you can see the exact runtime conditions that caused the
  violation in your debugger.

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

## Difference between cx and Zig

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

## Difference between cx and Jai

- cx has compile-time null-safety and nullable types.

- cx has method call syntax.

- cx has interfaces, which can be used e.g. as type parameter bounds in generic code for better error messages.

- cx has automatic importing of C headers, implemented using the Clang API.

- cx doesn't require importing standard library modules explicitly.

- cx doesn't require importing files from the same project.
  All cx files in the project source directory are by default assumed to belong to the project as a "convention over configuration".

- cx has a more familiar C-style syntax, which:
  - allows C/C++ programmers to be immediately productive with it
  - allows existing syntax tools such as syntax highlighters and code formatters to be used with little to no changes.

[^rust-modules]: <https://boats.gitlab.io/blog/post/2017-01-04-the-rust-module-system-is-too-confusing/>{target="_blank"}
