# Modules and imports

`cx build` compiles every `.cx` file in the project directory, recursively,
as one module, so files in the same project never need to import each other.
See [Build system](build-system).

## Importing a module

An `import` declaration pulls in a directory of `.cx` files from outside the project,
making its declarations visible to the importing file.
Every `.cx` file in the named directory, recursively, becomes part of the module:

```cx
import shapes;
```

Dependencies declared in `build.cx` are imported by their package name
(see [Build system](build-system)).
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

An import target ending in `.h` imports a C header instead of a C* module.
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
