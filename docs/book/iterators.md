# Iterators

Iterators are objects used to traverse through the elements of a collection.
A `for` loop accepts anything that has an `iterator()` method returning an iterator,
which includes arrays, lists, strings, maps, sets, and numeric ranges.

An iterator is any type that implements the `Iterator` interface:

```cs
interface Iterator<Element> {
    bool hasValue();
    Element value();
    void increment();
}

void main() {
}
```

A `for` loop like `for (var element in collection)` is shorthand for
calling `iterator()` on the collection, then repeatedly checking `hasValue()`,
reading the current element with `value()`, and advancing with `increment()`.

## How to create a custom iterator

To make a type iterable, define an `iterator()` method that returns an iterator object.
The iterator itself holds the traversal state and implements `hasValue()`, `value()`, and `increment()`.

```cs
struct Countdown {
    int start;

    Countdown(int start) {
        this.start = start;
    }

    CountdownIterator iterator() {
        return CountdownIterator(this);
    }
}

struct CountdownIterator: Copyable, Iterator<int> {
    int current;

    CountdownIterator(Countdown countdown) {
        current = countdown.start;
    }

    bool hasValue() {
        return current > 0;
    }

    int value() {
        return current;
    }

    void increment() {
        current--;
    }
}

void main() {
    for (var element in Countdown(3)) {
        println(element); // prints 3 2 1
    }
}
```
