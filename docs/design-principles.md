# Design principles

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
  principle. For simple things: "There should be one - and preferably only one
  - obvious way to do it."
- Should scale well, e.g. fast build times, easy greppability of function/type/variable definitions.
- Should be familiar and attractive to C++ and C developers. I.e. try not to
  change too much. Only change things that are an objective improvement from C++
  or otherwise well justified.

## Resources for further cx language design

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
