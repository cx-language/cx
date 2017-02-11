struct Foo {
    unsigned bar;
    const char* baz;
};

inline unsigned getBar(const struct Foo* foo) {
    return foo->bar;
}
