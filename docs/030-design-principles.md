# Design principles

These are the goals that shape cx's design, followed by the things cx deliberately does not
try to be.

## Performance

Code compiled in release mode should be as fast as hand-written low-level code on modern
hardware. cx compiles to native code through LLVM (or the C backend), has no garbage
collector, and does not perform expensive operations behind your back: there are no
implicit deep copies through copy constructors, and moves are destructive, so moved-from
values need no resetting and their destructors don't run (see [Structs](./structs)).
Higher-level features such as generics, tagged unions, and iterator chains are designed to
compile down to the code you would have written by hand, and you only pay for the features
you use.

Planned features open up more optimization opportunities than C++ has: integer overflow
that is undefined behavior in release builds (see [Builtin types](./builtin-types#planned-features)),
pointers declared non-aliasing (see [Pointers](./pointers#planned-features)), and struct
fields the compiler may reorder for a better layout (see [Structs](./structs#planned-features)).

## Fast iteration

Debug builds should compile and start as fast as possible to keep the edit-compile-run
cycle short. `cx run` executes through an in-process JIT, skipping the system linker (see
[Build modes](./build-system#build-modes)).

The language is also designed to let you write quick, hacky code to reach a design
decision faster, then make the surviving code correct afterwards. One example of this is
how cx prefers to emit warnings where other programming languages would stop the
compilation with hard errors: an unused variable or a missed null check doesn't block you
from running the program to try out an idea.

## Progressive disclosure of complexity

Following the principle of [progressive
disclosure](https://wiki.c2.com/?ProgressiveDisclosure){target="_blank"}, it should be easy to write most of the code, the parts that aren't performance
bottlenecks, so that it performs reasonably efficiently, and the language should provide
full control to optimize the bottlenecks without switching languages. More advanced
features are there when you need them, but you don't have to learn them up front.

## Simplicity

The language stays small, and most of its perceived simplicity comes from progressive
disclosure. For simple things, "there should be one, and preferably only one, obvious way
to do it": there is one composite value type (`struct`) instead of separate struct and
class concepts, one member access operator (`.`, never `->`), and in most code you get by
with just references. Fewer overlapping concepts means less to learn and less that can
surprise you, and a straightforward grammar makes tooling such as syntax highlighting and
formatting easier to write and keep correct.

## Safety by default

Debug builds check array bounds, integer overflow, and null dereferences, and report
memory leaks and double-frees at exit, so bugs that C and C++ leave to testing and luck
fail loudly at the line that caused them. The type system tracks which values may be null
and catches missing null checks at compile time (see [Nullable types](./nullable-types)).
Checks can be disabled individually or globally where measured performance requires it.
Built-in sanitizers such as AddressSanitizer are planned for catching the memory bugs these
checks don't (see [Build system](./build-system#planned-features)).

References and views (`T&`, `T[]`, `string`, iterators) are checked at compile time
without lifetime annotations: a borrow of a local or temporary cannot escape its
function, and an owner cannot be reassigned, moved, or mutated while a view of it
is alive (see [Pointers](./pointers#borrowed-parameters)). Raw pointers (`T*`) are
the explicit unsafe hatch and are not checked. cx still does not guarantee memory
or thread safety the way Rust does; see [Non-goals](#non-goals).

## Freedom of style

cx does not enforce a naming convention, coding style, or programming paradigm:
imperative, generic, data-oriented, functional, and object-oriented code are all
supported. Warnings can be tuned to each project's needs, for example enabling
`--Wunused-result` or disabling unused-entity warnings with `--Wno-unused`. Safe
implicit conversions turn into warnings or errors with `-Wconversion` and
`-Werror` (see [Casting](./casting)).

## Familiarity

cx should be familiar and attractive to C and C++ developers, so it tries not to change
too much: it only changes things that are an objective improvement over C++ or otherwise
well justified. Code with curly braces and `int x = 1;` declarations should look familiar
from the start.

## Working with existing code

C headers can be imported directly, with no bindings to write, so existing C libraries and
operating system APIs are usable from day one, and cx can be adopted gradually next to
existing code (see [Using C libraries](./low-level-programming#using-c-libraries)).
Interoperating with C++ code to a certain degree, to avoid writing a C wrapper for
everything, is a longer-term goal, but it should not restrict the design of the language.

## Scaling to large projects

Build times should stay fast as a project grows, and definitions of functions, types, and
variables should be easy to find with a plain text search. There are no header files or
forward declarations, and libraries are imported as a whole, so library authors can
reorganize their files without breaking users (see [Modules and imports](./modules)).

## Batteries-included tooling

`cx build` and `cx run` work without any configuration, dependencies are fetched straight
from Git repositories, `cx test` runs unit tests, and the `cx-lsp` language server
provides editor support (see [Build system](./build-system) and [Language
server](./lsp)).

## Proven ideas over new dogma

cx doesn't try to introduce big new ideas. Instead it takes features that have proven
themselves across many languages and aims to implement them as well as possible.

## Non-goals

cx explicitly does not aim for:

- **Rust-style compile-time guarantees at the cost of iteration speed and programmer
  productivity.** cx checks references and views with cheap function-local rules and no
  lifetime annotations, keeping the language simple and builds fast; it does not aim for
  Rust's full static guarantees. The remaining memory bugs are caught with debug-mode
  checks and tooling such as AddressSanitizer instead (see [cx vs
  Rust](./comparison#cx-vs-rust)).
- **A managed runtime or mandatory garbage collector.** There is no hidden runtime (aside
  from the optional safety checks in debug builds) and you only pay for what you use: the
  program you write is the program that runs.
- **Source compatibility with C or C++.** cx interoperates with existing C code
  (C++ interop is a longer-term goal) so projects can adopt it gradually,
  but it does not keep their syntax or semantics where they are flawed.
- **A stable, mature ecosystem today.** cx is still evolving toward its 1.0 release, so
  projects that need long-term stability guarantees are better served elsewhere for now
  (see [Project status](./introduction#project-status)).
