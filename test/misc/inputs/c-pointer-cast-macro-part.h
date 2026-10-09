// Benign redefinitions of the main header's macros (like NULL across CRT
// headers): identical pointer constants collapse instead of erroring.
#define C_SHARED_NULL ((void*)0)
#define C_SHARED_INVALID ((void*)-1)
