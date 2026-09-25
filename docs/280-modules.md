# Modules and imports

`cx build` compiles every `.cx` file in the project directory, recursively,
as one module, so files in the same project never need to import each other.
See [Build system](build-system).

The standard library needs no import either:
every non-private declaration in `std/` is visible to all code automatically.
Only external code needs `import`: other directories outside your project,
and vendored libraries under `vendor/`, which compile as separate modules.

## Importing a module

An `import` declaration pulls in a directory of `.cx` files from outside the project,
making its declarations visible to the importing file.
Every `.cx` file in the named directory, recursively, becomes part of the module:

```cx
import shapes;
```

Dependencies declared in `build.cx` are imported by their package name
(see [Build system](build-system)).
Vendored dependencies are imported the same way:
a `vendor/<package>/` directory is found without extra flags.
Other directories are made importable with `-I`:

```sh
work/
├── myproject/
│   ├── main.cx         # contains: import shapes;
│   └── build.cx
└── shared/
    └── shapes/
        └── shape.cx
```

```sh
cx build -I ../shared
```

Only code outside the project can be imported:
importing a subdirectory of your own project is an error,
since `cx build` already compiles it.
An import is scoped to the file it's written in:
if another file also needs the module, it must import it too.

## Importing C headers

An import target ending in `.h` imports a C header instead of a cx module.
See [Using C libraries](low-level-programming#using-c-libraries).

## Vendored C libraries

The compiler ships importable modules for some popular C libraries,
provided as `extern` declarations with precise nullability,
so using them needs no C header import (which is slower to compile):

```cs {.noRun}
import glfw;
import gl;
import sdl3;
import raylib;
```

Vendored modules are imported explicitly like any other module, but need
no entry in `dependencies` and no link settings: each module carries its
own `build.cx`, and the build applies its link requirements automatically
when the module is imported. The C library itself must be installed on
the system; if it cannot be found, the build fails with an error naming it.

Currently vendored: `glfw` (GLFW 3.x, without the Vulkan functions),
`gl` (OpenGL 3.3 core profile), `sdl3` (SDL 3.x; see the header of
`vendor/sdl3/sdl3.cx` for omissions), and `raylib` (raylib 6.x, without
raymath.h; see the header of `vendor/raylib/raylib.cx` for omissions).
A `vendor/<package>/` directory in your own project shadows the shipped
module of the same name, so you can
override or extend a binding; the shadowing package's link settings
apply under `cx build` like any other vendored dependency. Vendored
modules need the native library at link time, so they are unavailable
in the web playground.

See [Using C libraries](low-level-programming#using-c-libraries).

## Compiling files directly

For quick one-off scripts and temporary tests, `.cx` files can be passed
to the compiler directly instead of using `cx build`:

```sh
cx main.cx helper.cx -o myapp
cx run main.cx
```

Files passed together form one module, with no imports needed between them.
A directory that isn't being compiled must be imported explicitly;
modules are looked up next to the source files, so with this layout:

```sh
myproject/
├── main.cx         # contains: import shapes;
└── shapes/
    ├── shape.cx
    └── circle.cx
```

```sh
cx run main.cx
```

`import shapes;` finds the `shapes/` directory sitting beside `main.cx`.

## Lazy checking of imports

Imported modules are parsed in full, but only the declarations your code
actually uses are typechecked. This keeps builds fast: a large dependency
costs almost nothing if you only use a small part of it. Your own module is
always checked in full, so errors surface even in unused code.

As a consequence, broken code in a dependency that nothing uses produces no
error. To validate a dependency completely, for example in its own test
suite, pass `--check-all`:

```sh
cx build --check-all
```

This typechecks every declaration in every imported module.
