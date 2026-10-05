# Introduction

cx is a general-purpose programming language for writing fast software productively.
It compiles to efficient native code, gives you full low-level control when you need it,
and offers high-level conveniences the rest of the time, all without a garbage collector
or hidden runtime.

The language is simple and unopinionated, supporting imperative, generic, data-oriented,
functional, and object-oriented programming. Its syntax belongs to the C family, so if you
have used a language with curly braces and `int x = 1;` declarations, cx code should look
familiar from the start.

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

## Who cx is for

cx is a general-purpose language designed especially for areas where you want both
low-level control and high-level expressiveness in one language: gamedev, graphics, system
software, simulations, tools, and other performance-sensitive applications where debug
performance and quick prototyping matter.

It aims to combine the control and performance of C, C++, Zig, or Odin with the
expressiveness of a higher-level language: compile-time null safety, method call syntax,
closures, generics with interface bounds, operator overloading, tagged unions, and a
capable standard library, while keeping the language small and approachable.
[Design principles](./design-principles) explains the goals behind this and what cx
deliberately does not try to be, and [Comparison with related
languages](./comparison) covers how it differs from each of those languages.

## Project status

cx is approaching its 0.1 release. The language is already usable and being used for
writing real software, but some planned features are not implemented yet, some existing
features may still change, and there are still some bugs (all of which have been easily
worked around). It is a good fit for projects that don't require a 100% stable foundation,
or backwards compatibility, or a mature ecosystem. If you use it, questions are warmly
welcome and feedback is greatly appreciated on
[GitHub](https://github.com/cx-language/cx/issues){target="_blank"} and the
[Discord server](https://discord.gg/hsDbW9p){target="_blank"}.

## Frequently asked questions

### Do I need to import the standard library or other files in my project?

No. The standard library is visible to all code automatically, and all `.cx`
files in your project (except `build.cx` and vendored libraries under `vendor/`)
compile as one module, so neither needs `import`. Only external code needs
importing: dependencies, vendored libraries, and other directories outside your
project. See [Modules and imports](./modules).

### Is cx memory-safe?

Safe by default in the common cases, but without compile-time guarantees like Rust's.
See [Safety by default](./design-principles#safety-by-default) and [cx vs
Rust](./comparison#cx-vs-rust).

### How do I build a project and add dependencies?

Run `cx run` in the project directory; it works with no configuration. Dependencies can
either be Git repositories declared in `build.cx`, or vendored code alongside your code.
You import both by package name. Run tests with `cx test`. See [Build
system](./build-system).

### How do I use existing C or C++ code?

Import C/C++ headers directly (`import "MyHeader.h"`) and call their functions like
ordinary cx functions; individual C functions can also be declared with `extern`. C++
interop with `extern "C++"` only supports basic C-like language constructs for now; a more
full-featured C++ interop is a longer-term goal. See [Using C
libraries](./low-level-programming#using-c-libraries).

### Which build mode should I use?

The default debug mode compiles fast with safety checks enabled. Use `--release`
for fully optimized builds with checks disabled (overflow wraps), or
`--release-safe` for optimized builds that keep the checks. See [Build modes](./build-system#build-modes).

### There is no preprocessor or macros - how do I conditionally compile code?

Use `#if` with `-D` defines. Predefined platform-conditions use `#if Windows` for example,
both in source files and in `build.cx`. See [Conditional
compilation](./build-system#conditional-compilation).

## Not to be confused with

cx shares its name with several unrelated languages:

- [CX](https://github.com/skycoin/cx), Skycoin's general-purpose language with Go-like syntax.
- [Cx](https://github.com/fernandothedev/cx), a language that transpiles to C99.
- [Cx](https://github.com/COMMENTERTHE9/Cx_lang), a GC-free systems language for game engines.
- [C++/CX](https://learn.microsoft.com/en-us/cpp/cppcx/type-system-c-cx?view=msvc-170&redirectedfrom=MSDN), Microsoft's C++ extensions for the Windows Runtime.

## Where to go next

- [Getting started](./hello-world): install the compiler and write your first program.
- [Language overview](./language-overview): a quick tour of the language, with links to the details.
- [Design principles](./design-principles): the goals behind the language design, and its non-goals.
- [Comparison with related languages](./comparison): how cx relates to C, C++, Rust, Zig, Odin, and Jai.
