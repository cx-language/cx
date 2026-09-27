# Access specifiers

Declarations are public by default: any file in the same module, or any file
that imports the module, can use them.
Marking a declaration `private` restricts its use to the module it's declared in:
any file in the same module can use it, but a file that merely imports the
module gets a warning when using it.
`private` can be applied to struct members, functions, global variables, and types.

```cs
struct Settings {
    private int retries;
}

private int defaultRetries() {
    return 3;
}

void main() {
    // Using private declarations is fine in the file they're declared in.
    var settings = Settings(retries = defaultRetries());
    println(settings.retries); // prints 3
}
```

Using a `private` declaration from another module produces a warning, not an error:
`private` marks an implementation detail, but nothing stops you from using it
if you really need to.

Note that `private` is the only access specifier: there is no `public` keyword.
(The `public` marker on function parameters is unrelated:
it enables [named arguments](functions), not access control.)
Interface members cannot be private either,
since implementing types in other files must provide them.
