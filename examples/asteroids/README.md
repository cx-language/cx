This directory contains a bare-bones clone of the Asteroids game. It uses the [SDL](https://www.libsdl.org) C library for graphics.
To build the project, install pkg-config and SDL3, then run `cx build`. This creates a `bin/asteroids` executable.

On macOS, install the prerequisites with `brew install pkg-config sdl3`.

On Windows, `cx build` needs a `pkg-config` that can find SDL3. Alternatively, build manually:
1. Download the SDL3 Visual C++ development libraries from https://github.com/libsdl-org/SDL/releases.
2. Set the environment variable SDLDIR to point to the extracted SDL3 directory.
3. Install LLVM/Clang from https://releases.llvm.org/download.html. The build uses the LLVM linker.
4. Compile with `cx -c *.cx -I%SDLDIR%\include` and link with `lld-link output.obj -LIBPATH:%SDLDIR%\lib\x64 SDL3.lib legacy_stdio_definitions.lib ucrt.lib msvcrt.lib -DEBUG`. This should create an `output.exe` file.
5. Copy SDL3.dll to the same directory as the executable.
6. Run `output.exe`.

__Controls:__ arrow keys to move, space to shoot, esc to quit.

<img width="726px" src="https://cloud.githubusercontent.com/assets/7543552/24635118/3f52e926-18da-11e7-962f-92216de6df8e.gif">
