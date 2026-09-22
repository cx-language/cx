# Build system

`cx build` builds a cx project with no configuration:
run it in the project directory and it compiles every `.cx` file there, recursively,
into an executable named after the directory.
The only exception is the `build.cx` file in the project directory itself,
which holds build settings instead of source code (see below).

```sh
myproject/
├── main.cx
└── util.cx

$ cx build
$ ./myproject
```

Extra build settings go in a `build.cx` file in the project directory,
written in plain cx syntax. Each setting is a variable declaration:

```cx
var name = "myproject"
var pkgConfigDependencies = ["sdl3"]
var libraries = ["m"]
```

Supported settings:

- `name`: the executable name. Output goes in the project root directory by default.
- `outputDirectory`: where to place executables (project root by default).
- `multitarget`: if true, build each subdirectory of `src/` (or of the project directory, if there is no `src/`) as a separate executable.
- `defines`: preprocessor definitions, as if passed with `-D`.
- `headerSearchPaths`: directories to search for C headers, as if passed with `-I`.
- `librarySearchPaths`: directories to search for libraries, as if passed with `-L`.
- `libraries`: system libraries to link, as if passed with `-l`. A value naming an existing file (such as a vendored static archive) is linked directly instead of being searched for.
- `frameworks`: macOS frameworks to link, as if passed with `-framework`.
- `pkgConfigDependencies`: system libraries resolved via `pkg-config --cflags --libs`.
- `dependencies`: cx libraries to fetch from Git, as `(package = "...", url = "...", version = "...")` entries (see below).

Each setting can be declared only once,
but `#if` conditions select settings per platform or configuration:

```cx
var name = "myproject"

#if macOS
var frameworks = ["Cocoa"]
#else
var libraries = ["X11"]
#endif
```

The conditions `macOS` and `Windows` are predefined on those platforms;
custom conditions are enabled with `-D` flags: `cx build -DEXTRA`.
`-D`, `-I`, `-L`, `-l`, and `-framework` flags can also be passed directly
for one-off overrides.

## Build modes

`cx build` and `cx run` build in debug mode by default: unoptimized for fast
compilation, safety checks enabled. Two flags select optimized release builds:

- `--release`: fully optimized (`-O3`), safety checks disabled.
- `--release-safe`: fully optimized (`-O3`), safety checks enabled.

Safety checks abort the program with an error on integer overflow and similar
traps; `--release` drops them for maximum speed, so arithmetic overflow
wraps instead.

Builds embed debug info: aborts print a stack trace naming the cx functions
involved, and the binary loads in a debugger (`lldb`, `gdb`).
On macOS the debug info for `cx build` output is collected into a `.dSYM`
bundle next to the binary.

## Installing dependencies

Dependencies are cx libraries hosted in Git repositories.
Each entry gives the `package` name used by `import`,
the repository `url` (any URL `git clone` accepts),
and the `version` to check out (a tag, branch, or commit):

```cx
var dependencies = [(package = "shapes", url = "https://github.com/example/shapes.git", version = "v1.2.0")]
```

On the first build, `cx build` clones each repository into `~/.cx/dependencies`
and checks out the requested revision.
The package is then imported by its package name:

```cx
import shapes;
```

There is no package registry: dependencies are just Git repositories nominated by URL,
with no central index or publishing step.

### Transitive dependencies

Each dependency's own `build.cx` applies to that dependency: its `defines`,
`headerSearchPaths`, `librarySearchPaths`, `libraries`, `frameworks`, and
`pkgConfigDependencies` are used when compiling it, and its `dependencies`
are fetched and imported the same way, recursively. Paths in a dependency's
settings resolve against that dependency's directory. Only `name`,
`outputDirectory`, and `multitarget` stay with the main project.

A package required at two versions, or both vendored and fetched, is an
error at the importing `import`: remove or unify the duplicate source.
Command-line `-D`, `-I`, `-L`, `-l`, and `--cflags` flags apply to every
package; a dependency's `#if` conditions see those plus its own `defines`.

Passing files directly (`cx main.cx`) skips dependency build files: sources
still resolve, but their settings don't apply. Use `cx build` for projects
with dependencies.

### Vendoring

If you'd rather not fetch over the network, you can vendor dependencies instead:
copy each library into its own directory under `vendor/`,
with the directory name matching the `package` name you'd import,
and `import` it by that name:

```sh
myproject/
├── build.cx
├── main.cx         # contains: import shapes;
└── vendor/
    └── shapes/
        └── shape.cx
```

```cx
import shapes;
```

`cx build` runs at the project root and only reads the `build.cx` there.
The `build.cx` at an imported package's root applies to that package as above.
A `build.cx` anywhere else is an ordinary source file and compiles as usual.
Everything else under `vendor/` is only reachable via `import`.
Because vendored code compiles as an imported module rather than as part of
your project, unused functions in it don't produce warnings.
Sources kept outside the project can likewise be used with `-I` plus `import`.

Vendored C libraries work the same way: point `headerSearchPaths` at their headers
and `librarySearchPaths` at their binaries, then link by name:

```cx
var headerSearchPaths = ["vendor/mylib/include"]
var librarySearchPaths = ["vendor/mylib/lib"]
var libraries = ["mylib"]
```
