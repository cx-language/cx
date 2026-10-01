// C counterparts for extern-float-struct-abi.cx: echo each aggregate shape by
// value in both directions, so the test proves cx agrees with the C ABI
// (register vs memory passing) rather than with itself.
#include <stdarg.h>

typedef struct { float x, y; } V2;
typedef struct { float x, y, z; } V3;
typedef struct { float x, y, z, w; } V4;
typedef struct { double x, y; } D2;
typedef struct { int i; float f; } IF;
typedef struct { float f; double d; } FD;
typedef struct { double d; float f; } DF;
typedef struct { char c; float f; } CF;
typedef struct { char c; double d; } CD;
typedef struct { double d; char c; } DC;
typedef struct { int a, b; float f; } IIF;
typedef struct { _Bool b; float f; } BF;
typedef struct { int *p; float f; } PF;
typedef struct { double d; int i; } DI;
typedef struct { IF m[2]; } IF2;
typedef struct { double d[4]; } D4;
typedef struct { float a[3]; } FA3;
typedef struct { V2 a, b; } NV2;
typedef union UF {
    float a;
    float b;
} UF;
typedef union UI {
    float f;
    int i;
} UI;
typedef struct { float f; char c[24]; } BigF;
typedef struct { int x, y; } Pair;
typedef struct { long long a, b; } LL2;

V2 echo_v2(V2 v) { return v; }
V3 echo_v3(V3 v) { return v; }
V4 echo_v4(V4 v) { return v; }
D2 echo_d2(D2 v) { return v; }
IF echo_if(IF v) { return v; }
FD echo_fd(FD v) { return v; }
DF echo_df(DF v) { return v; }
CF echo_cf(CF v) { return v; }
CD echo_cd(CD v) { return v; }
DC echo_dc(DC v) { return v; }
IIF echo_iif(IIF v) { return v; }
BF echo_bf(BF v) { return v; }
PF echo_pf(PF v) { return v; }
DI echo_di(DI v) { return v; }
IF2 echo_if2(IF2 v) { return v; }
D4 echo_d4(D4 v) { return v; }
FA3 echo_fa3(FA3 v) { return v; }
NV2 echo_nv2(NV2 v) { return v; }
UF echo_uf(UF v) { return v; }
UI echo_ui(UI v) { return v; }
BigF echo_bigf(BigF v) { return v; }
Pair echo_pair(Pair v) { return v; }
LL2 echo_ll2(LL2 v) { return v; }

// Several aggregates in one call exhaust the first registers.
float sum3_v2(V2 a, V2 b, V2 c) { return a.x + a.y + b.x + b.y + c.x + c.y; }
double mix_args(V2 v, int n, double d, V2 w) { return v.x + v.y + n + d + w.x + w.y; }

// Structs in variadic position classify like named arguments.
float vfirst(int tag, ...) {
    va_list ap;
    va_start(ap, tag);
    V2 v = va_arg(ap, V2);
    va_end(ap);
    return tag + v.x + v.y;
}

int vsumpair(int tag, ...) {
    va_list ap;
    va_start(ap, tag);
    Pair p = va_arg(ap, Pair);
    va_end(ap);
    return tag + p.x + p.y;
}

// Arrays decay to pointers in variadic position too.
int vfirsti(int tag, ...) {
    va_list ap;
    va_start(ap, tag);
    int *p = va_arg(ap, int *);
    va_end(ap);
    return tag + p[0] + p[1];
}

// The cx side defines these with C linkage; calling through here exercises
// the C-to-cx direction (callee-side register reads).
V2 cx_id_v2(V2 v);
V4 cx_id_v4(V4 v);
D4 cx_id_d4(D4 v);
IF cx_id_if(IF v);
V2 trig_v2(V2 v) { return cx_id_v2(v); }
V4 trig_v4(V4 v) { return cx_id_v4(v); }
D4 trig_d4(D4 v) { return cx_id_d4(v); }
IF trig_if(IF v) { return cx_id_if(v); }
