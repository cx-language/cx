# Namespaces

A file can scope all of its declarations under a namespace with a single
declarator on its first line:

```cs {.noCompile}
namespace File;

bool exists(string path) {
    return File.size(path) != null;
}
```

Everything declared in the file (functions, types, constants, global
variables) is then only visible through the qualified name: `File.exists`,
`File.Error`, `File.read`. There is no fallback: unqualified `exists`
does not find `File.exists`, not even inside the namespace's own files.
Qualify every use, including calls between members of the same namespace.

Callers use the same syntax for every member kind:

```cs
void main() {
    if File.exists("notes.txt") {
        println("found it");
    }
    var size = File.size("notes.txt");
    println(size == null);
}
```

Types work the same way, including in annotations and generic arguments:

```cs
void main() {
    File.Error? error = null;
    Result<string, File.Error> result = Err(File.Error.CouldNotOpen);
    println(error == null);
    println(result == Err);
}
```

A namespace can span files: every file with the same declarator adds to the
same namespace. Nest with dots:

```cs {.noCompile}
namespace outer.inner;

int helper() {
    return 1;
}

int deep() {
    return outer.inner.helper();
}
```

A namespace root must not collide with a top-level declaration name in the
same module: with `namespace File;` in the module, declaring `struct File` is an
error. For the same reason, types and functions take precedence when both
could match `a.b`; values never hide a namespace, so a parameter named
`path` does not break `Path.join` calls.

Name namespaces in PascalCase, like types: `File`, `Path`, `Json`. A qualified
call then reads like a static method call (`Path.join(...)`), and never
confuses the reader the way `path.join(path)` would, with the same spelling
meaning two different things on one line.

Two declarations stay global even in a namespaced file, because lookup for
them is global: `extern` declarations name C symbols, which have no
namespaces, and operator overloads resolve through global lookup. The `main`
function must be global too: declaring it inside a namespace is an error.

The standard library scopes its crowded areas this way: `File.exists`,
`Path.join`, `Process.run`, `Json.parse`. Most of the standard
library stays global (`List`, `println`); namespaces are for groups of
related declarations that would otherwise claim generic names. `private`
works as usual: it restricts a declaration to its module, qualified or not.
