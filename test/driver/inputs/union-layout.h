// Shapes for extern-union-layout.cx: unions whose largest member is not
// their most-aligned one, nested in structs. Tagged so the C backend can
// reference them as `union U`.
typedef union U5 {
    char c[5];
    int i;
} U5;
typedef struct SU {
    char x;
    U5 u;
} SU;
SU echo_su(SU v);
typedef union U9D {
    char c[9];
    double d;
} U9D;
typedef struct SU9 {
    char x;
    U9D u;
} SU9;
SU9 echo_su9(SU9 v);
typedef union UI2 {
    float f;
    int i;
} UI2;
UI2 echo_ui2(UI2 v);
