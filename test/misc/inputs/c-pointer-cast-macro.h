#define C_NULL ((void*)0)
#define C_CHAR_NULL ((char*)0)
#define C_CONST_NULL ((const void*)0)
#define C_BARE_NULL (void*)0
#define C_INVALID ((void*)-1)
#define C_PPTR ((void**)0)
#define C_NEG ((void*)-42)

// Typedefs resolve wherever they are defined, even after the macro.
#define C_FWD_REF ((FwdWidget*)-1)
typedef struct FwdWidget { int x; } FwdWidget;

typedef struct Handle__* Phandle;
#define C_BAD_HANDLE ((Phandle)-1)
#define C_HANDLE_NULL ((Phandle)0)

// Benign redefinitions across headers collapse (like NULL across CRT
// headers); the part header repeats these two identically.
#define C_SHARED_NULL ((void*)0)
#define C_SHARED_INVALID ((void*)-1)
#include "c-pointer-cast-macro-part.h"

// Unsupported shapes (still skipped; see c-pointer-cast-macro-errors.cx):
typedef int MyInt;
#define C_INT_CAST ((MyInt)-1)
#define C_MULTIWORD ((unsigned long*)0)
#define C_PLUS ((void*)+1)
#define C_FLOAT_CAST ((void*)1.5)

void takePhandle(Phandle h);
