# Benchmarks

Internal performance tracking for the compiler and compiled programs.
CI appends one JSON record per `main` commit to the floating `bench-data`
tag (a tag, not a branch, so GitHub shows no "recent pushes" banner);
the website renders history graphs at `/bench`.

## Corpus

| Program | Workload | Output |
| --- | --- | --- |
| `sieve.cx` | Sieve of Eratosthenes below 1e8 | prime count |
| `mandelbrot.cx` | Mandelbrot grid, 1000 iterations per point | checksum |
| `fib.cx` | Naive recursive `fib(40)` | 102334155 |
| `wordcount.cx` | Word frequencies over 4M pseudo-random words | checksum |
| `mapfilter.cx` | map/filter chains plus capturing closure calls | checksum |
| `jsonparse.cx` | Parse a 5000-user JSON document 100 times | checksum |

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
