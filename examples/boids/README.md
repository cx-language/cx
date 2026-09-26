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

## Running

```sh
./boids
```

## Testing

A deterministic screenshot (binary PPM) with scene-framing checks.
It runs the simulation headless, without opening a window:

```sh
./boids --screenshot shot.ppm
```

## Controls

- Hold the left mouse button: attract the flock to the cursor.
- Esc: quit.
