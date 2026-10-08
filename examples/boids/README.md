# boids

![Boids flocking](screenshot.png)

A boids flocking simulation written in cx: 1500 agents with
separation, alignment, and cohesion over a uniform grid, in a
wrapping toroidal world. Boids are colored by speed. It uses the
[SDL](https://www.libsdl.org) C library for graphics.

## Building

Install pkg-config and SDL3, then run `cx build`. This creates a
`boids` executable.

On Ubuntu 25.04 and newer, install the prerequisites with
`sudo apt-get install -y pkg-config libsdl3-dev`. On older releases,
build SDL3 from source (see `../asteroids/README.md`).

On macOS, install the prerequisites with
`brew install pkg-config sdl3`.

On Windows, pkg-config is not used; the build links the SDL3 import
library instead. Install SDL3 with vcpkg (`vcpkg install sdl3:x64-windows`),
then build with `cx build -L%VCPKG_INSTALLATION_ROOT%\installed\x64-windows\lib`.
This creates a `boids.exe` executable. Alternatively, download the SDL3
Visual C++ development libraries from https://github.com/libsdl-org/SDL/releases,
set SDLDIR to the extracted directory, and build with `cx build -L%SDLDIR%\lib\x64`.

## Running

```sh
./boids
```

On Windows, copy `SDL3.dll` from `installed\x64-windows\bin` (or `%SDLDIR%\lib\x64`)
next to `boids.exe` first, then run `boids.exe`.

## Testing

A deterministic screenshot (binary PPM) with scene-framing checks.
It runs the simulation headless, without opening a window:

```sh
./boids --screenshot shot.ppm
```

## Controls

- Hold the left mouse button: attract the flock to the cursor.
- Esc: quit.
