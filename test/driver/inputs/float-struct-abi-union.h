// Union shape for extern-float-struct-abi.cx, imported from C: cx cannot
// declare unions yet, so the test cannot spell this shape itself. Tagged so
// the C backend can reference it as `union UF`.
typedef union UF {
    float a;
    float b;
} UF;
UF echo_uf(UF v);
typedef union UI {
    float f;
    int i;
} UI;
UI echo_ui(UI v);
