# Type aliases

A type alias gives another name to an existing type. Type aliases are declared at module scope with `using`, an identifier, and `=`:

```cs
using Count = int;
using MaybeCount = Count?;
using CountList = List<Count>;

struct User {
    Count id;
    MaybeCount previousId;
}

void main() {
    User user = User(id = 42, previousId = null);
    CountList counts = List<Count>();
    println(user.id);
    println(counts.size());
}
```

Aliases are transparent. `Count` is exactly `int`, and `MaybeCount` is exactly `int?`. They do not create a new runtime type or change the generated code. An alias can be used anywhere its aliased type can be used, including generic arguments, function signatures, constructors, scalar conversions, and enum case access. Diagnostics quote the spelling used in source: mismatching a `Count` complains about `Count`, not `int32`.

Aliases can refer to aliases declared later in the same module. Recursive aliases are rejected. Generic type aliases are not supported yet.
