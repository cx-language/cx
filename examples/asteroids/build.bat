cx -c *.cx -I%SDLDIR%\include -fms-extensions
lld-link output.obj -LIBPATH:%SDLDIR%\lib\x64 SDL3.lib legacy_stdio_definitions.lib ucrt.lib msvcrt.lib -DEBUG
