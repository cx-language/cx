# c-interop

Calls C from cx and cx from C.

- [`mathlib.h`](mathlib.h)/[`mathlib.c`](mathlib.c) — a small C library.
  [`main.cx`](main.cx) imports the header and calls it directly.
- [`cxlib.cx`](cxlib.cx) — cx functions declared `extern`, giving them C
  linkage under their plain names. [`cxlib.h`](cxlib.h) declares them for
  [`main.c`](main.c), which calls them like any C function.

`build.cx` links `libc-interop-math.a` via `libraries` +
`librarySearchPaths`. To call cx through the JIT runtime instead of static
linking, see [`../embedding/`](../embedding/).

## Calling C from cx

Compile the C side to a static library first, then `cx build` links it:

```sh
cc -O2 -c mathlib.c -o mathlib.o
ar rcs libc-interop-math.a mathlib.o
cx build
./c-interop
```

Expected output:

```
30
42
120
```

## Calling cx from C

Compile the cx side to an object file (no `main`, so `-c` is implied, but
passing it is harmless), then link it into the C program:

```sh
cx cxlib.cx -c -o cxlib.o
cc main.c cxlib.o -o c-caller -lm
./c-caller
```

Expected output:

```
1
5.000000
```
