# fractal

![Mandelbrot close-up](screenshot.jpg)

A GPU Mandelbrot explorer written in cx: the fragment shader
evaluates the set per pixel with smooth iteration-count coloring,
so zooming and panning stay fluid. It uses the
[GLFW](https://www.glfw.org) C library for window management.

## Building

Install pkg-config and GLFW3, then run `cx build`. This creates a
`fractal` executable.

On Ubuntu, install the prerequisites with
`sudo apt-get install -y pkg-config libglfw3-dev libgl1-mesa-dev`.

On macOS, install the prerequisites with
`brew install pkg-config glfw`.

On Windows, pkg-config is not used; the build links the GLFW DLL import
library instead (modern OpenGL entry points load at runtime via
gl-loader.cx). Install GLFW with vcpkg (`vcpkg install glfw3:x64-windows`),
then build with `cx build -L%VCPKG_INSTALLATION_ROOT%\installed\x64-windows\lib`.
This creates a `fractal.exe` executable. Alternatively, download the
Windows binaries from https://www.glfw.org/download.html and pass `-L`
pointing at their `lib-vc2022` directory (matching your compiler version).

## Running

Run from this directory so the program finds `assets/`:

```sh
./fractal
```

On Windows, copy `glfw3.dll` from `installed\x64-windows\bin` next to
`fractal.exe` first, then run `fractal.exe`.

## Testing

```sh
./fractal --screenshot shot.ppm
```

Note that `--screenshot` still initializes GLFW, so it needs a display
and will not run on a headless Linux server without X.

## Controls

- Left-drag: pan. Scroll: zoom at the cursor.
- Up/Down: double/halve the iteration limit. R: reset the view.
- Esc: quit.
