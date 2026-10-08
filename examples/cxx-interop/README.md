# cxx-interop

Calls C++ from cx and cx from C++.

- [`veclib.hpp`](veclib.hpp)/[`veclib.cpp`](veclib.cpp) — a small C++
  library. [`main.cx`](main.cx) imports the header and calls it directly.
- [`cxlib.cx`](cxlib.cx) — cx functions declared `extern "C++"`, giving
  them Itanium-mangled names. [`cxlib.hpp`](cxlib.hpp) declares them for
  [`main.cpp`](main.cpp), which calls them like any C++ function.

`build.cx` links `libcxx-interop-vec.a` (`cxx-interop-vec.lib` on
Windows) via `libraries` + `librarySearchPaths`.

## Calling C++ from cx

Compile the C++ side to a static library first, then `cx build` links it:

```sh
c++ -std=c++17 -O2 -c veclib.cpp -o veclib.o
ar rcs libcxx-interop-vec.a veclib.o
cx build
./cxx-interop
```

On Windows, from a Visual Studio developer prompt:

```sh
cl /nologo /std:c++17 /O2 /c veclib.cpp /Foveclib.obj
lib /nologo /OUT:cxx-interop-vec.lib veclib.obj
cx build
cxx-interop.exe
```

Expected output:

```
15
```

## Calling cx from C++

Compile the cx side to an object file (no `main`, so `-c` is implied, but
passing it is harmless), then link it into the C++ program:

```sh
cx cxlib.cx -c -o cxlib.o
c++ -std=c++17 main.cpp cxlib.o -o cxx-caller
./cxx-caller
```

Expected output:

```
1
0
```

This direction is not available on Windows: `extern "C++"` uses the
Itanium ABI, which is not supported on MSVC targets, so `cxlib.cx`
exports nothing there (see its `#if`).
