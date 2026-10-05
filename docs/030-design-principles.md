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

cx does not guarantee memory or thread safety at compile time the way Rust does; see
[Non-goals](#non-goals).

## Freedom of style

cx does not enforce a naming convention, coding style, or programming paradigm:
imperative, generic, data-oriented, functional, and object-oriented code are all
supported. Warnings can be tuned to each project's needs, for example enabling
`--Wunused-result` or disabling unused-entity warnings with `--Wno-unused`.

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

- **Guaranteed compile-time memory safety at the cost of iteration speed and programmer
  productivity.** cx prefers a simpler language and faster iteration, catching memory bugs
  with debug-mode checks and tooling such as AddressSanitizer instead (see [cx vs
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

## Language design resources

Discussions that have informed the design of cx:

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
