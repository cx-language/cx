typedef long long myint64;

struct WithAnon {
    int tag;
    union {
        myint64 a;
        int b;
    };
};

struct OnlyUnion {
    union {
        myint64 a;
        int b;
    };
};

struct Flat {
    int x;
    struct {
        int y;
        int z;
    };
};

struct NestedFlat {
    int x;
    struct {
        int y;
        struct {
            int z;
        };
    };
};

struct TwoUnions {
    union {
        int a;
    };
    union {
        int b;
    };
};

union StructInUnion {
    struct {
        int x;
        int y;
    };
    myint64 d;
};

struct NameClash {
    union {
        int a;
    };
    int unnamed_0;
};

struct WithAnon makeWithAnon(void);
struct OnlyUnion makeOnlyUnion(void);
struct Flat makeFlat(void);
struct NestedFlat makeNestedFlat(void);
struct TwoUnions makeTwoUnions(void);
union StructInUnion makeStructInUnion(void);
struct NameClash makeNameClash(void);
