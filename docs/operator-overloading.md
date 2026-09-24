# Operator overloading

Operators can be overloaded for user-defined types by declaring a function
named `operator` followed by the operator. Binary operators are overloaded
with non-member functions taking two parameters. Operator operands can be
borrowed implicitly: `T&` parameters accept values as usual, and lvalues can
also fill `T*` parameters without an explicit `&`. Ordinary function calls
still require `&` for a `T*` parameter. Value parameters copy their operands.

```cs
struct Vec2: Copyable {
    int x;
    int y;
}

Vec2 operator+(Vec2 a, Vec2 b) {
    return Vec2(a.x + b.x, a.y + b.y);
}

bool operator==(Vec2 a, Vec2 b) {
    return a.x == b.x && a.y == b.y;
}

void main() {
    var a = Vec2(1, 2);
    var b = Vec2(3, 4);
    var c = a + b;
    println(c.x); // prints 4
    println(c.y); // prints 6
    println(a + b == c); // prints true
}
```

The overloadable operators are `==`, `!=`, `<`, `<=`, `>`, `>=`,
`+`, `-`, `*`, `/`, `%`, and the subscript operator `[]` below.
Operator functions can be generic, like any other function.

Comparison operators are synthesized, so only `==` and `<` need overloads:
`!=` is the negation of `==`, and `==`/`!=` also match with their operands
swapped. `a > b` checks `b < a`, `a >= b` checks `!(a < b)`,
and `a <= b` checks `!(b < a)`.

The subscript operator is overloaded with a member function,
so it can access the instance through `this`.
A typical use is indexed access to a type's components,
like the axes of a vector:

```cs
struct Vec3: Copyable {
    int x;
    int y;
    int z;

    int operator[](int index) {
        if index == 0 {
            return x;
        } else if index == 1 {
            return y;
        }
        return z;
    }

    void operator[]=(int index, int value) {
        if index == 0 {
            x = value;
        } else if index == 1 {
            y = value;
        } else {
            z = value;
        }
    }
}

void main() {
    var v = Vec3(1, 2, 3);
    println(v[0] + v[1] + v[2]); // prints 6
    v[1] = 20; // calls operator[]=
    println(v.y); // prints 20
}
```
