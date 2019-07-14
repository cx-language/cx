This directory contains a bare-bones clone of the Asteroids game. It uses the [SDL](https://www.libsdl.org) C library for graphics.
To build the project, run `make`.

On Windows:
1. Download the SDL2 Visual C++ development libraries from https://www.libsdl.org/download-2.0.php.
2. Set the environment variable SDLDIR to point to the extracted SDL2 directory.
3. Install LLVM/Clang from https://releases.llvm.org/download.html. The build uses the LLVM linker.
4. Run `build.bat`. This should create an `output.exe` file.
5. Copy SDL2.dll to the same directory as the executable.
6. Run `output.exe`.

__Controls:__ arrow keys to move, space to shoot, esc to quit.

<img width="726px" src="https://cloud.githubusercontent.com/assets/7543552/24635118/3f52e926-18da-11e7-962f-92216de6df8e.gif">
