#pragma once

// Windows JIT libc pin table, shared by the LLVM-JIT driver path
// (src/driver/jit.cpp) and the AsmJit backends: every libc function
// reachable from JITed code on Windows, pinned to the host's address so
// JITed code binds the UCRT instead of legacy msvcrt.dll
// (llvm/llvm-project#200554). website/check_jit_pins.py verifies the table
// stays exhaustive against the `extern` declarations visible under
// `#if Windows` in std/.

namespace llvm {
class StringRef;
}

#ifdef _WIN32
#if defined(_M_IX86) || defined(__i386__)
#error "JIT libc pinning assumes unprefixed symbol names (x64/ARM64); 32-bit Windows is unsupported"
#endif

// Every libc function reachable from JITed code on Windows: all `extern`
// declarations visible under `#if Windows` in std/, plus heap-family
// extras (realloc, _msize, ...) that user code or C imports may reference,
// plus memmove/memcmp which LLVM itself may emit. Casts pin the exact C
// signature (math.h overloads would make &name ambiguous).
// Consumers take addresses of the plain C-linkage functions, so they must
// include the C headers and undefine MSVC's function-like macros first
// (see src/driver/jit.cpp and src/backend/jit-pins.cpp).
#define CX_JIT_LIBC_PINS(PIN) \
    PIN(_access, int (*)(const char*, int)) \
    PIN(_aligned_free, void (*)(void*)) \
    PIN(_aligned_malloc, void* (*)(std::size_t, std::size_t)) \
    PIN(_aligned_realloc, void* (*)(void*, std::size_t, std::size_t)) \
    PIN(_chdir, int (*)(const char*)) \
    PIN(_fdopen, FILE* (*)(int, const char*)) \
    PIN(_getcwd, char* (*)(char*, int)) \
    PIN(_hypotf, float (*)(float, float)) \
    PIN(_mkdir, int (*)(const char*)) \
    PIN(_msize, std::size_t (*)(void*)) \
    PIN(_pclose, int (*)(FILE*)) \
    PIN(_popen, FILE* (*)(const char*, const char*)) \
    PIN(_putenv, int (*)(const char*)) \
    PIN(_rmdir, int (*)(const char*)) \
    PIN(_set_abort_behavior, unsigned int (*)(unsigned int, unsigned int)) \
    PIN(_strdup, char* (*)(const char*)) \
    PIN(_wcsdup, wchar_t* (*)(const wchar_t*)) \
    PIN(abort, void (*)()) \
    PIN(acos, double (*)(double)) \
    PIN(acosf, float (*)(float)) \
    PIN(asin, double (*)(double)) \
    PIN(asinf, float (*)(float)) \
    PIN(atan, double (*)(double)) \
    PIN(atan2, double (*)(double, double)) \
    PIN(atan2f, float (*)(float, float)) \
    PIN(atanf, float (*)(float)) \
    PIN(calloc, void* (*)(std::size_t, std::size_t)) \
    PIN(cbrt, double (*)(double)) \
    PIN(cbrtf, float (*)(float)) \
    PIN(ceil, double (*)(double)) \
    PIN(ceilf, float (*)(float)) \
    PIN(cos, double (*)(double)) \
    PIN(cosf, float (*)(float)) \
    PIN(cosh, double (*)(double)) \
    PIN(coshf, float (*)(float)) \
    PIN(exit, void (*)(int)) \
    PIN(exp, double (*)(double)) \
    PIN(exp2, double (*)(double)) \
    PIN(exp2f, float (*)(float)) \
    PIN(expf, float (*)(float)) \
    PIN(fclose, int (*)(FILE*)) \
    PIN(feof, int (*)(FILE*)) \
    PIN(fflush, int (*)(FILE*)) \
    PIN(fgetc, int (*)(FILE*)) \
    PIN(floor, double (*)(double)) \
    PIN(floorf, float (*)(float)) \
    PIN(fmod, double (*)(double, double)) \
    PIN(fmodf, float (*)(float, float)) \
    PIN(fopen, FILE* (*)(const char*, const char*)) \
    PIN(fprintf, int (*)(FILE*, const char*, ...)) \
    PIN(fputc, int (*)(int, FILE*)) \
    PIN(fputs, int (*)(const char*, FILE*)) \
    PIN(fread, std::size_t (*)(void*, std::size_t, std::size_t, FILE*)) \
    PIN(free, void (*)(void*)) \
    PIN(fseek, int (*)(FILE*, long, int)) \
    PIN(ftell, long (*)(FILE*)) \
    PIN(fwrite, std::size_t (*)(const void*, std::size_t, std::size_t, FILE*)) \
    PIN(getchar, int (*)()) \
    PIN(getenv, char* (*)(const char*)) \
    PIN(hypot, double (*)(double, double)) \
    PIN(isalnum, int (*)(int)) \
    PIN(isalpha, int (*)(int)) \
    PIN(isblank, int (*)(int)) \
    PIN(iscntrl, int (*)(int)) \
    PIN(isdigit, int (*)(int)) \
    PIN(isgraph, int (*)(int)) \
    PIN(islower, int (*)(int)) \
    PIN(isprint, int (*)(int)) \
    PIN(ispunct, int (*)(int)) \
    PIN(isspace, int (*)(int)) \
    PIN(isupper, int (*)(int)) \
    PIN(isxdigit, int (*)(int)) \
    PIN(log, double (*)(double)) \
    PIN(log10, double (*)(double)) \
    PIN(log10f, float (*)(float)) \
    PIN(log2, double (*)(double)) \
    PIN(log2f, float (*)(float)) \
    PIN(logf, float (*)(float)) \
    PIN(malloc, void* (*)(std::size_t)) \
    PIN(memcmp, int (*)(const void*, const void*, std::size_t)) \
    PIN(memcpy, void* (*)(void*, const void*, std::size_t)) \
    PIN(memmove, void* (*)(void*, const void*, std::size_t)) \
    PIN(memset, void* (*)(void*, int, std::size_t)) \
    PIN(pow, double (*)(double, double)) \
    PIN(powf, float (*)(float, float)) \
    PIN(printf, int (*)(const char*, ...)) \
    PIN(puts, int (*)(const char*)) \
    PIN(realloc, void* (*)(void*, std::size_t)) \
    PIN(remove, int (*)(const char*)) \
    PIN(rename, int (*)(const char*, const char*)) \
    PIN(rewind, void (*)(FILE*)) \
    PIN(round, double (*)(double)) \
    PIN(roundf, float (*)(float)) \
    PIN(sin, double (*)(double)) \
    PIN(sinf, float (*)(float)) \
    PIN(sinh, double (*)(double)) \
    PIN(sinhf, float (*)(float)) \
    PIN(sprintf, int (*)(char*, const char*, ...)) \
    PIN(sqrt, double (*)(double)) \
    PIN(sqrtf, float (*)(float)) \
    PIN(strlen, std::size_t (*)(const char*)) \
    PIN(strtod, double (*)(const char*, char**)) \
    PIN(strtof, float (*)(const char*, char**)) \
    PIN(tan, double (*)(double)) \
    PIN(tanf, float (*)(float)) \
    PIN(tanh, double (*)(double)) \
    PIN(tanhf, float (*)(float)) \
    PIN(tolower, int (*)(int)) \
    PIN(toupper, int (*)(int)) \
    PIN(trunc, double (*)(double)) \
    PIN(truncf, float (*)(float)) \
    PIN(ungetc, int (*)(int, FILE*))

namespace cx {

// Host address of a pinned libc name, or nullptr when the name is not
// pinned. Strips asm label markers before comparing.
void* lookupPinnedLibcSymbol(llvm::StringRef name);

// True when an unpinned name is also exported by legacy msvcrt.dll, in
// which case JITed code might bind there instead of the UCRT and the
// caller must keep the link-and-exec path.
bool isMsvcrtAmbiguous(llvm::StringRef name);

} // namespace cx
#endif
