# Structs

A `struct` is a user-defined data type used to group variables into a single type.
Additionally, structs may define member functions, that can be called on instances of the struct.
Inside member functions, `this` is a pointer to the instance that the function was called on.

```cs
struct Person {
    string name;
    int age;
    Person*? friend;
    // Specify default values so we don't have to specify them when we instantiate Person
    float x = 0;
    float y = 0;

    void moveLeft() {
        // Members may be accessed via 'this'
        this.x -= 0.1;
    }

    void moveRight() {
        // Or without 'this'
        x += 0.1;
    }
}

void main() {
    var foo = Person(name: "Foo", age: 42, friend: null);
    var bar = Person(name: "Bar", age: 42, friend: foo);
    foo.friend = bar;

    foo.moveLeft(); // foo.x is now -0.1
    bar.moveRight();  // bar.x is now 0.1

    println(foo.x);
    println(bar.x);
}
```
