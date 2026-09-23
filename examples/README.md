# Examples

This directory contains example cx programs, from single-file demos to full
applications. Each subdirectory is a `cx build` project (see
[Build system](../docs/build-system.md)); single `.cx` files compile with
`cx <file>`.

## Single-file programs

- [`hello.cx`](hello.cx) — minimal program printing `Hello world`.
- [`fibonacci.cx`](fibonacci.cx) — Fibonacci numbers with a `for` loop.
- [`fizzbuzz.cx`](fizzbuzz.cx) — FizzBuzz with `if`/`else if` chains.
- [`filter-map.cx`](filter-map.cx) — `List.filter`/`map` pipeline.
- [`ranges.cx`](ranges.cx) — ranges, loops, and list initialization.
- [`sieve.cx`](sieve.cx) — prime number sieve using a `List`.
- [`structs.cx`](structs.cx) — structs with default values, named arguments, and methods.
- [`operator-overloading.cx`](operator-overloading.cx) — complex numbers with `operator+`/`operator*`.
- [`printable.cx`](printable.cx) — implement `Printable::print`, get `toString()` for free.
- [`interpolation.cx`](interpolation.cx) — string interpolation (`$name`, `${expr}`).
- [`null-safety.cx`](null-safety.cx) — nullable types (`int?`) with compile-time null-safety.
- [`result.cx`](result.cx) — fallible functions returning `Result` instead of throwing.
- [`tagged-union.cx`](tagged-union.cx) — tagged union (`enum` with payloads) and `switch` dispatch.
- [`mandelbrot.cx`](mandelbrot.cx) — Mandelbrot set visualizer with a custom `Complex` type.
- [`brainfuck.cx`](brainfuck.cx) — Brainfuck interpreter.
- [`tree.cx`](tree.cx) — recursive directory listing using C APIs (`dirent.h`).

## Projects

- [`argparser/`](argparser/) — CLI argument parser library (`argparser.cx`,
  `help.cx`) and a demo task manager (`main.cx`). Supports boolean flags,
  string/integer options, positional arguments, git-style subcommands, and
  generated help text. Build with `cx build` to get the `todo` executable.
- [`embedding/`](embedding/) — embed cx in a C++ host: `embedding.cpp` loads
  `script1.cx`/`script2.cx` via `cx.h` (`cxCreateModule`,
  `cxLoadScriptFromFile`, `cxCompileModule`, `cxGetFunction`) and calls into
  them. See [`cpp-interop/`](cpp-interop/) for the opposite direction
  (calling C++ from cx).
- [`cpp-interop/`](cpp-interop/) — call C++ from cx through `extern "C"`
  wrappers: a small C++ library (`mathlib.cpp`/`mathlib.h`) with a class and
  free functions, wrapped as C ABI (`wrapper.cpp`/`wrapper.h`), imported from
  cx (`main.cx`) via `import` and `extern` declarations.
- [`opengl/`](opengl/) — minimal OpenGL triangle using GLFW for window
  management. Needs `pkg-config` and GLFW3 (`apt install pkg-config
  libglfw3-dev libgl1-mesa-dev`, `brew install pkg-config glfw3`). Build with
  `cx build`.
- [`asteroids/`](asteroids/) — bare-bones Asteroids clone using SDL3.
  See [`asteroids/README.md`](asteroids/README.md) for platform setup.
  Controls: arrow keys to move, space to shoot, esc to quit.

  ![Asteroids gameplay](https://cloud.githubusercontent.com/assets/7543552/24635118/3f52e926-18da-11e7-962f-92216de6df8e.gif)
- [`voxel-game/`](voxel-game/) — Minecraft-style voxel game with procedurally
  generated textured terrain, first-person walking/jumping, block
  breaking/placing, rendered with OpenGL 3.3 via GLFW. See
  [`voxel-game/README.md`](voxel-game/README.md) for building, `--self-test`,
  and `--screenshot`. Controls: mouse to look, W/A/S/D to walk, Shift to
  sprint, Space to jump, left/right click to break/place, 1–6 to select block,
  Esc to quit.

  ![Voxel game screenshot](https://github.com/user-attachments/assets/c21f64b3-3d0c-44db-a742-b896d4ab1c17)

## Building all examples

```sh
python3 build_examples.py --cx /path/to/cx
```

compiles every single-file program (`cx <file> -Werror`) and every project
directory (`cx build -Werror`), skipping `inputs/` fixtures. `embedding/` is
built with `-Wno-unused` since its entry point is called from the C++ host.
`tree.cx`, `asteroids`, `opengl`, and `voxel-game` are skipped on Windows.
