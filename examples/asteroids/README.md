This directory contains a bare-bones clone of the Asteroids game. It uses the [SDL](https://www.libsdl.org) C library for graphics.
To build the project, install pkg-config and SDL3, then run `cx build`. This creates an `asteroids` executable.

On Ubuntu 25.04 and newer, install the prerequisites with `sudo apt-get install -y pkg-config libsdl3-dev`.

On older Ubuntu releases, SDL3 has no apt package, so install pkg-config
(`sudo apt-get install -y pkg-config`) and build SDL3 from source
(see https://wiki.libsdl.org/SDL3/README-linux for prerequisites):

```sh
git clone --depth 1 --branch release-3.4.16 https://github.com/libsdl-org/SDL.git /tmp/SDL
cmake -S /tmp/SDL -B /tmp/SDL/build -DCMAKE_BUILD_TYPE=Release
cmake --build /tmp/SDL/build --parallel
sudo cmake --install /tmp/SDL/build
```

On macOS, install the prerequisites with `brew install pkg-config sdl3`.

On Windows, pkg-config is not used; the build links the SDL3 import library
instead. Install SDL3 with vcpkg (`vcpkg install sdl3:x64-windows`), then run
`cx build -L%VCPKG_INSTALLATION_ROOT%\installed\x64-windows\lib`. This creates
an `asteroids.exe` executable. Alternatively, download the SDL3 Visual C++
development libraries from https://github.com/libsdl-org/SDL/releases, set
the environment variable SDLDIR to the extracted directory, and run
`cx build -L%SDLDIR%\lib\x64`. To run the game, copy `SDL3.dll` next to the
executable first.

__Controls:__ arrow keys to move, space to shoot, esc to quit.

<img width="726px" src="https://cloud.githubusercontent.com/assets/7543552/24635118/3f52e926-18da-11e7-962f-92216de6df8e.gif">
