# Operator overloading

Operators can be overloaded for user-defined types by declaring a function
named `operator` followed by the operator. Binary operators are overloaded
with free functions taking two parameters. Operator operands can be
borrowed implicitly: `T&` parameters accept values as usual, and lvalues can
also fill `T*` parameters without an explicit `&`. Ordinary function calls
still require `&` for a `T*` parameter.

```cs
struct Money {
    int cents;
}

Money operator+(Money a, Money b) {
    return Money(a.cents + b.cents);
}

bool operator==(Money a, Money b) {
    return a.cents == b.cents;
}

void main() {
    var a = Money(100);
    var b = Money(250);
    var c = a + b;
    println(c.cents); // prints 350
    println(a + b == c); // prints true
}
```

The overloadable operators are `==`, `!=`, `<`, `<=`, `>`, `>=`,
`+`, `-`, `*`, `/`, `%`, and the subscript operator `[]` below.
Operator functions can be generic, like any other function.

Comparison operators are synthesized, so only `==` and `<` need overloads:
`!=` is the negation of `==`, etc.

Structs also get a default `==` that compares every member with `==` and `&&`s
the results together. The default covers `!=` too (members compared with `!=`, `||`ed).
Comparing structs of different types is still an error.

The subscript operator is overloaded with a method,
so it can access the instance through `this`:

```cs
struct Date {
    int year;
    int month;
    int day;

    int operator[](int index) {
        if index == 0 {
            return year;
        } else if index == 1 {
            return month;
        }
        return day;
    }

    void operator[]=(int index, int value) {
        if index == 0 {
            year = value;
        } else if index == 1 {
            month = value;
        } else {
            day = value;
        }
    }
}

void main() {
    var v = Date(2024, 1, 31);
    println(v[0] + v[1] + v[2]); // prints 2056
    v[1] = 2; // calls operator[]=
    println(v.month); // prints 2
}
```
