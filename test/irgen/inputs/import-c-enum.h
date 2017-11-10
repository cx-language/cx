typedef enum {
    FooA = -1,
    FooB = 0x7FFFFFFF
} Foo;

typedef enum {
    BarA,
    BarB = 0x7FFFFFFF
} Bar;

typedef enum {
    QuxA,
    QuxB = 0xFFFFFFFFFFFFFFFF
} Qux;
