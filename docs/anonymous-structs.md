# Anonymous structs

An anonymous struct groups a fixed set of named fields into a single value.
Fields are accessed by name with `.`, like named struct fields,
but unlike named structs, anonymous structs need no declaration.

```cs
void main() {
    var point = (x = 3, y = 4);
    println(point.x); // prints 3
    println(point.y); // prints 4
}
```

Functions can return multiple values by returning an anonymous struct:

```cs
(string name, int age) parseUser(string input) {
    return (name = input, age = 42);
}

void main() {
    var user = parseUser("Bo");
    println(user.name); // prints Bo
    println(user.age); // prints 42
}
```

Anonymous structs can be nested, and compared for equality with `==` and `!=`.
Two anonymous structs are equal if all their fields are equal.

```cs
void main() {
    var a = (x = 1, y = (z = true, w = 2.5));
    var b = (x = 1, y = (z = true, w = 2.5));
    println(a == b); // prints true
    println(a != b); // prints false
}
```

Anonymous structs are also handy as element types in containers,
for example to store pairs of values in a list:

```cs
void main() {
    var settings = List([(name = "width", value = 800), (name = "height", value = 600)]);

    for setting in settings {
        println(setting.name, " = ", setting.value);
    }
}
```

## Structural typing

Unlike named structs, which are nominal (a value of one struct type is never
assignable to another struct type, even with identical fields), anonymous
structs are structural: two anonymous struct types with the same field names
and field types are the same type, so values convert when each field converts.

## Planned features

- Destructuring anonymous structs into separate variables; for now, access fields by name.
- Unnamed members.
