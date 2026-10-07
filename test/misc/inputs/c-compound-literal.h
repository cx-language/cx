typedef struct Color {
    unsigned char r, g, b, a;
} Color;
#define RAYWHITE (Color){ 245, 245, 245, 255 }
#define RED (Color){ 230, 41, 55, 255 }
#define WITH_EXPR (Color){ 1 + 2, 10 * 2, 100 / 4, 255 }
#define TRAILING_COMMA (Color){ 9, 9, 9, 9, }
#define STRUCT_FORM (struct Color){ 5, 6, 7, 8 }
typedef struct Pixel {
    Color color;
    int x;
} Pixel;
#define NESTED (Pixel){ (Color){ 1, 2, 3, 4 }, 10 }
#define FWD (Later){ 1, 2 }
typedef struct Later {
    int a, b;
} Later;
#define BASE (Color){ 10, 20, 30, 40 }
typedef struct Wrap {
    Color c;
    int n;
} Wrap;
#define REUSES_BASE (Wrap){ BASE, 7 }
typedef struct HasChar {
    char ch;
    int n;
} HasChar;
#define WITH_CHAR (HasChar){ 'a', 5 }
typedef struct HasFloat {
    float f;
    double d;
} HasFloat;
#define WITH_FLOAT (HasFloat){ 2.5f, 3.5 }
typedef struct {
    int x, y;
} AnonPoint;
#define ANON_TYPEDEF (AnonPoint){ 3, 4 }
#define PARTIAL (Color){ 1, 2 }
#define TOO_MANY (Color){ 1, 2, 3, 4, 5 }
#define DESIGNATED (Color){ .r = 1, .g = 2, .b = 3, .a = 4 }
#define WRONG_TYPE (Nope){ 1, 2, 3, 4 }
#define SCALAR_FORM (int){ 5 }
#define OUT_OF_RANGE (Color){ 1, 2, 3, 999 }
#define FWD_REF (Wrap){ LATER_C, 8 }
#define LATER_C (Color){ 1, 2, 3, 4 }
int mutableGlobal;
#define USES_MUTABLE (Color){ mutableGlobal, 1, 1, 1 }
typedef union U {
    int a;
    float b;
} U;
#define UNION_LIT (U){ 1 }
typedef struct HasAnon {
    union {
        int a;
        float b;
    };
    int c;
} HasAnon;
#define ANON_MEMBER_LIT (HasAnon){ 1, 2 }
