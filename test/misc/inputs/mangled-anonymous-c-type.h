struct Outer {
    struct { int x, y; } inner;
    int z;
};
struct Outer getOuter(void);
