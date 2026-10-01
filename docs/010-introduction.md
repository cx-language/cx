# Introduction

cx is a general-purpose programming language for writing fast software productively.
It compiles to efficient native code, gives you full low-level control when you need it,
and offers high-level conveniences the rest of the time, all without a garbage collector
or hidden runtime.

The language is simple and unopinionated, supporting imperative, generic, data-oriented,
functional, and object-oriented programming. Its syntax belongs to the C family, so if you
have used a language with curly braces and `int x = 1;` declarations, cx code should look
familiar from the start.

## A first look

```cs
struct Image {
    int width;
    int height;
}

Result<Image, string> makeImage(int width, int height) {
    if width <= 0 || height <= 0 {
        return Err("dimensions must be positive");
    }
    return Ok(Image(width = width, height = height));
}

void main() {
    for size in [640, -1, 1920] {
        switch makeImage(size, size) {
            case Ok img: println(img.width, "x", img.height);
            case Err e: println("rejected: ", e);
        }
    }

    var evenSquares = List([1, 2, 3, 4, 5, 6]).filter(n => n % 2 == 0).map(n => n * n).toList();
    println(evenSquares);
}
```

This small program already shows several things cx is built around: plain structs,
errors returned as values and handled with `switch`, named arguments, type inference with
`var`, and lazy iterator chains with lambdas. Code examples on the website can be edited
and run directly in the browser, or you can install the compiler as described in
[Getting started](./hello-world).

## What cx is about

- **Performance.** Code compiled in release mode should be as fast as hand-written
  low-level code. cx compiles to native code through LLVM (or the C backend), has no garbage
  collector, and does not perform expensive operations behind your back. Higher-level
  features such as generics, tagged unions, and iterator chains are designed to compile
  down to the code you would have written by hand.
- **Productivity.** Writing most code should be easy, and optimizing the hot spots should
  be possible without switching languages. A rich standard library, concise syntax,
  and a fast edit-compile-run cycle keep the focus on the problem being solved.
- **Simplicity.** The language stays small, with one obvious way to do simple things.
  More advanced features are there when you need them, but you don't have to learn them
  up front.
- **Safety by default.** Debug builds check array bounds, integer overflow, and null
  dereferences, and report memory leaks and double-frees at exit. The type system tracks which values may
  be null and catches missing null checks at compile time. Checks can be disabled
  individually or globally where performance requires it.
- **Freedom of style.** cx does not enforce a naming convention, coding style, or
  programming paradigm. Warnings for things like implicit conversions can be tuned to each
  project's needs.
- **Batteries-included tooling.** `cx build` and `cx run` work without any configuration,
  dependencies are fetched straight from Git repositories, `cx test` runs unit tests,
  and the `cx-lsp` language server provides editor support.
- **Working with existing code.** C headers can be imported directly, with no bindings to
  write, so existing C libraries and operating system APIs are usable from day one and
  cx can be adopted gradually next to existing code.
- **Proven ideas over new dogma.** cx doesn't try to introduce big new ideas. Instead it
  takes features that have proven themselves across many languages and aims to implement
  them as well as possible.

The [Language overview](./language-overview) walks through these features in more detail,
and [Design principles](./design-principles) describes the reasoning behind them.

## Who cx is for

cx is a general-purpose language, but it is designed especially for game development and
other areas where you want both low-level control and high-level expressiveness in one
language: graphics, simulations, tools, and other performance-sensitive applications where
debug performance and quick prototyping matter.

Debug builds compile fast for a short edit-compile-run cycle (see
[Build modes](./build-system#build-modes)), and nothing stops you from writing quick, hacky
code to reach a design decision faster, then making the surviving code correct afterwards.
For a sense of what this looks like in practice, see the
[example projects](https://github.com/cx-language/cx/tree/main/examples){target="_blank"},
which include a voxel game, a ray tracer, and a flocking simulation.

## Where cx fits

Many languages compete in the space of fast, natively compiled software, and each makes
different trade-offs:

- **C** is minimal, universal, and time-tested, but leaves much of the work, and many of
  the mistakes, to the programmer.
- **C++** is powerful and mature, but large and complex after decades of backwards
  compatibility.
- **Rust** guarantees memory and thread safety at compile time, at the cost of a more
  complex language and the work of satisfying the borrow checker.
- **Zig**, **Odin**, and **Jai** are newer languages that rethink low-level programming
  around simplicity and direct control.

cx aims for a different balance: the control and performance of a low-level language
together with the expressiveness of a higher-level one (generics with interface bounds,
methods, operator overloading, closures, null safety, tagged unions, and a capable standard
library), while keeping the language small and approachable. For detailed comparisons with
each of these languages, see [Comparison with related languages](./comparison).

## Non-goals

cx explicitly does not aim for:

- **Guaranteed compile-time memory or thread safety.** cx prefers a simpler language and
  faster iteration, catching memory bugs with debug-mode checks and tooling such as
  AddressSanitizer instead (see [cx vs Rust](./comparison#cx-vs-rust)).
- **A managed runtime or garbage collector.** There is no hidden runtime:
  the program you write is the program that runs.
- **Source compatibility with C or C++.** cx interoperates with existing C code
  (C++ interop is a longer-term goal) so projects can adopt it gradually,
  but it does not keep their syntax or semantics where they are flawed.

## Project status

cx is approaching its 0.1 release. Most of the language is usable today, but some planned
features are not implemented yet, existing features may still change, and there are bugs.
It is a good fit for experimenting, hobby projects, and giving feedback, but not yet for
projects that need a stable, mature ecosystem. Questions and feedback are welcome on
[GitHub](https://github.com/cx-language/cx/issues){target="_blank"} and the
[Discord server](https://discord.gg/hsDbW9p){target="_blank"}.

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

cx is safe by default in the common cases: array accesses are bounds-checked, integer
arithmetic is overflow-checked, null dereferences are checked, and the type system tracks
nullability at compile time. But cx does not guarantee memory or thread safety at compile
time the way Rust does; for that, use debug-mode checks and tooling such as
AddressSanitizer. See [Safer by default](./language-overview#safer-by-default)
and [cx vs Rust](./comparison#cx-vs-rust).

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

## Where to go next

- [Getting started](./hello-world): install the compiler and write your first program.
- [Language overview](./language-overview): a tour of the main features.
- [Design principles](./design-principles): the reasoning behind the language design.
- [Comparison with related languages](./comparison): how cx relates to C, C++, Rust, Zig, Odin, and Jai.
