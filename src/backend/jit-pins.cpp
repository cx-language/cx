#include "jit-pins.h"

#ifdef _WIN32
#pragma warning(push, 0)
#include <llvm/ADT/DenseSet.h>
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)
#include "../ast/mangle.h"
// C headers so the pins take addresses of the plain C-linkage functions.
#include <cstddef>
#include <ctype.h>
#include <direct.h>
#include <io.h>
#include <malloc.h>
#include <math.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Several libc names are function-like macros on MSVC; undefine them so the
// pins take the real functions (each also exists as a function; the link
// fails loudly if not).
// ctype.h names:
#undef isalnum
#undef isalpha
#undef isblank
#undef iscntrl
#undef isdigit
#undef isgraph
#undef islower
#undef isprint
#undef ispunct
#undef isspace
#undef isupper
#undef isxdigit
#undef tolower
#undef toupper
// stdio.h names:
#undef fgetc
#undef fputc
#undef getchar
#undef ungetc

// Exclude the min/max, GDI, and OLE APIs: their macros would otherwise break
// LLVM and cx headers (same as api.cpp).
#define NOMINMAX
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)

using namespace cx;

static const llvm::DenseSet<llvm::StringRef>& pinnedLibcNames() {
    static const auto names = [] {
        llvm::DenseSet<llvm::StringRef> set;
#define PIN(name, sig) set.insert(#name);
        CX_JIT_LIBC_PINS(PIN)
#undef PIN
        return set;
    }();
    return names;
}

void* cx::lookupPinnedLibcSymbol(llvm::StringRef name) {
    llvm::StringRef stripped = stripAsmLabelMarker(name);
#define PIN(pinned, sig) \
    if (stripped == #pinned) return (void*)static_cast<sig>(&pinned);
    CX_JIT_LIBC_PINS(PIN)
#undef PIN
    return nullptr;
}

bool cx::isMsvcrtAmbiguous(llvm::StringRef name) {
    llvm::StringRef stripped = stripAsmLabelMarker(name);
    if (pinnedLibcNames().contains(stripped)) return false;
    // Unpinned names that legacy msvcrt.dll also exports would bind there
    // instead of the UCRT (llvm/llvm-project#200554): heap/stdio state would
    // split across CRTs. Keep the link-and-exec path when that can happen.
    HMODULE msvcrt = GetModuleHandleW(L"msvcrt.dll");
    return msvcrt && GetProcAddress(msvcrt, stripped.str().c_str()) != nullptr;
}
#endif
