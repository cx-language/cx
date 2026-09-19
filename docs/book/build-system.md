# Build system

`cx build` builds a C* project with no configuration:
run it in the project directory and it compiles every `.cx` file there, recursively,
into an executable named after the directory.

```sh
myproject/
├── main.cx
└── util.cx

$ cx build
$ ./myproject
```

Extra build settings go in a `build.cx` file in the project directory,
written in plain C* syntax. Each setting is a variable declaration:

```cx
var name = "myproject"
var pkgConfigDependencies = ["sdl3"]
var libraries = ["m"]
```

Supported settings:

- `name`: the executable name. Output goes in the `bin` directory by default.
- `outputDirectory`: where to place executables (`bin` by default).
- `multitarget`: if true, build each subdirectory of `src/` (or of the project directory, if there is no `src/`) as a separate executable.
- `defines`: preprocessor definitions, as if passed with `-D`.
- `libraries`: system libraries to link, as if passed with `-l`.
- `frameworks`: macOS frameworks to link, as if passed with `-framework`.
- `pkgConfigDependencies`: system libraries resolved via `pkg-config --cflags --libs`.
- `dependencies`: C* libraries to fetch from Git, as `(package = "...", url = "...", version = "...")` entries (see below).

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

## Installing dependencies

Dependencies are C* libraries hosted in Git repositories.
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

### Vendoring

If you'd rather not fetch over the network, you can vendor dependencies instead:
copy the library sources into your project (for example under `vendor/`),
and `cx build` compiles them as part of it, with no `import` or `build.cx` entry needed.
Sources kept outside the project can likewise be used with `-I` plus `import`.
