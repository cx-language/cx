# Benchmarks

Internal performance tracking for the compiler and compiled programs.
CI appends one JSON record per `main` commit to the floating `bench-data`
tag (a tag, not a branch, so GitHub shows no "recent pushes" banner);
the website renders history graphs at `/bench`.

## Corpus

| Program | Workload | Output |
| --- | --- | --- |
| `sieve/sieve.cx` | Sieve of Eratosthenes below 1e8 | prime count |
| `mandelbrot/mandelbrot.cx` | Mandelbrot grid, 1000 iterations per point | checksum |
| `fib/fib.cx` | Naive recursive `fib(40)` | 102334155 |
| `wordcount/wordcount.cx` | Word frequencies over 4M pseudo-random words | checksum |
| `mapfilter/mapfilter.cx` | map/filter chains plus capturing closure calls | checksum |
| `jsonparse/jsonparse.cx` | Parse a 5000-user JSON document 100 times | checksum |

Each program prints a single deterministic value. The bench script fails
if runs disagree, since unstable output means a meaningless benchmark.

## Running locally

```sh
python3 scripts/bench-corpus.py --cx build/cx --runs 2 --compile-runs 1
```

This writes `bench.json` with compile medians, run medians, and binary
sizes. `--build-seconds` / `--check-seconds` attach C++ build and test
suite times; CI passes those, local runs leave them null.

## Caveats

GitHub-hosted runners are noisy neighbors: single data points wobble,
trends and large step changes are the signal. Medians over repeated runs
dampen the wobble. A pinned self-hosted runner would tighten this
further if small regressions ever need bisecting.

## Language comparison

`bench` ports the corpus programs to C, C++, Rust, Go, Odin and Zig, written
idiomatically per language. Each program lives in its own directory with
all its ports, e.g. `bench/fib/fib.cx`, `fib.c`, `fib.cpp`, `fib.rs`,
`fib.go`, `fib.odin`, `fib.zig`. `jsonparse` is ported only to Go, Odin,
and Zig; C, C++, and Rust have no JSON parser in the standard library.
`mapfilter` is omitted for C, Go, Odin, and Zig, which have no capturing
lambdas.
`scripts/bench-langs.py` builds each port twice, runs it, and records
medians plus a self-contained HTML report with a chart per build. Debug
builds are also timed as rebuilds: one untimed compile warms the
standard-library cache, then each of `--compile-runs` samples compiles a
separate copy of the source. ccache is disabled. `--metrics compile` records
only those debug rebuilds and does not run the binaries; `--metrics run`
skips compile timing. The optimized build is the release configuration. The
unoptimized debug build is the development configuration: cx's default build
with `--no-leak-check`
(safety checks stay on, and both the LLVM IR pipeline and codegen
optimizations are skipped; the leak detector would otherwise exit these
programs for memory they leave to the OS), C and C++ at `-O0 -g`, Rust at
opt-level 0 with debug assertions and overflow checks, Go with
`-gcflags=all=-N -l`, Odin with `-debug` (`-o:none`), and Zig with
`-ODebug` (release is `-OReleaseFast`).

```sh
python3 scripts/bench-langs.py --cx build/cx
open bench/report.html
```

Missing toolchains are skipped with a warning. Ports must print the
same output as cx, except `mandelbrot`, whose checksum depends on
platform-defined float-to-int conversion and is only checked for
self-consistency.
