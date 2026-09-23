# cpp-interop

Calls C++ from cx through `extern "C"` wrappers. cx imports C headers
directly, so a C++ library is used via a C ABI boundary: the C++ class
becomes an opaque handle with explicit create/destroy functions, and free
functions are wrapped one-to-one.

- [`mathlib.h`](mathlib.h)/[`mathlib.cpp`](mathlib.cpp) — the C++ library:
  a `Calculator` class plus a `factorial` free function.
- [`wrapper.h`](wrapper.h)/[`wrapper.cpp`](wrapper.cpp) — `extern "C"`
  wrappers exposing the class as `CalculatorHandle*` and the free function
  as `math_factorial`.
- [`main.cx`](main.cx) — imports `wrapper.h` and uses both. `defer`
  destroys the handle even on early return.

## Building

Compile the C++ side to a static library first, then `cx build` links it:

```sh
c++ -std=c++20 -O2 -c mathlib.cpp -o mathlib.o
c++ -std=c++20 -O2 -c wrapper.cpp -o wrapper.o
ar rcs libcpp-interop-math.a mathlib.o wrapper.o
cx build
./cpp-interop
```

Expected output:

```
30
120
```

`build.cx` links `libcpp-interop-math.a` via `libraries` +
`librarySearchPaths`. To call cx from C++ instead (embed the runtime),
see [`../embedding/`](../embedding/).
