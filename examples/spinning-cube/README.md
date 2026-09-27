# spinning-cube

![Spinning cube output](screenshot.png)

A software rasterizer written in cx: a spinning cube with flat
shading and a depth buffer, projected and rasterized one triangle at
a time. No dependencies; renders directly to a binary PPM file.

## Building

Run `cx build`. This creates a `spinning-cube` executable.

## Running

```sh
./spinning-cube [output.ppm] [width height] [angle]
```

Defaults to `cube.ppm` at 800x600 with the cube spun 25 degrees. An
800x600 render takes well under a tenth of a second with a debug
build. Pass different angles to spin the cube.

## Testing

```sh
./spinning-cube --self-test
```

checks the rotation math, the projection, the face and background
pixels of a tiny render, and that two angles render different frames,
printing `self-test passed` on success.
