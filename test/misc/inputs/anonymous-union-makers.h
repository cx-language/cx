// Defined maker functions for the anonymous-union.h shapes, so the C backend
// test can build and run. The definitions live here (instead of a separate
// .c file) because the generated C is a single translation unit that
// includes this header.

#pragma once

#include "anonymous-union.h"

struct WithAnon makeWithAnon(void) {
    struct WithAnon w;
    w.tag = 1;
    w.a = 11;
    return w;
}

struct OnlyUnion makeOnlyUnion(void) {
    struct OnlyUnion o;
    o.a = 22;
    return o;
}

struct Flat makeFlat(void) {
    struct Flat f;
    f.x = 1;
    f.y = 2;
    f.z = 3;
    return f;
}

struct NestedFlat makeNestedFlat(void) {
    struct NestedFlat n;
    n.x = 1;
    n.y = 2;
    n.z = 3;
    return n;
}

struct TwoUnions makeTwoUnions(void) {
    struct TwoUnions t;
    t.a = 10;
    t.b = 20;
    return t;
}

union StructInUnion makeStructInUnion(void) {
    union StructInUnion s;
    s.x = 1;
    s.y = 2;
    return s;
}

struct NameClash makeNameClash(void) {
    struct NameClash c;
    c.a = 30;
    c.unnamed_0 = 40;
    return c;
}
