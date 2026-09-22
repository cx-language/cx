# Tuples

A tuple groups a fixed set of named values into a single value.
Tuple elements are accessed by name with `.`, like struct fields,
but unlike structs, tuples need no declaration.

```cs
void main() {
    var point = (x = 3, y = 4);
    println(point.x); // prints 3
    println(point.y); // prints 4
}
```

Functions can return multiple values by returning a tuple:

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

Tuples can be nested, and compared for equality with `==` and `!=`.
Two tuples are equal if all their elements are equal.

```cs
void main() {
    var a = (x = 1, y = (z = true, w = 2.5));
    var b = (x = 1, y = (z = true, w = 2.5));
    println(a == b); // prints true
    println(a != b); // prints false
}
```

Tuples are also handy as element types in containers,
for example to store pairs of values in a list:

```cs
void main() {
    var settings = List([(name = "width", value = 800), (name = "height", value = 600)]);

    for setting in settings {
        println(setting.name, " = ", setting.value);
    }
}
```

## Planned features

- Destructuring tuples into separate variables; for now, access elements by name.
- Anonymous tuple members.
