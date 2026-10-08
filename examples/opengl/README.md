This directory contains a simple OpenGL example.
It uses the [GLFW](https://www.glfw.org) C library for window management.
To build the project, install pkg-config and GLFW3, then run `cx build`.
This creates an `opengl` executable.

On Ubuntu, install the prerequisites with `sudo apt-get install -y pkg-config libglfw3-dev libgl1-mesa-dev`.

On macOS, install the prerequisites with `brew install pkg-config glfw`.

On Windows, pkg-config is not used; the build links the GLFW DLL import
library instead (modern OpenGL entry points load at runtime via
gl-loader.cx). Install GLFW with vcpkg (`vcpkg install glfw3:x64-windows`),
then build with `cx build -L%VCPKG_INSTALLATION_ROOT%\installed\x64-windows\lib`.
This creates an `opengl.exe` executable; to run it, copy `glfw3.dll` from
`installed\x64-windows\bin` next to it. Alternatively, download the Windows
binaries from https://www.glfw.org/download.html and pass `-L` pointing at
their `lib-vc2022` directory (matching your compiler version).
