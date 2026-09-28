#include "jit.h"
#pragma warning(push, 0)
#include <llvm/ADT/DenseSet.h>
#include <llvm/ExecutionEngine/Orc/AbsoluteSymbols.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>
#pragma warning(pop)
#include "../ast/mangle.h"
#include "../support/utility.h"
#ifdef _WIN32
#include <libloaderapi.h>
#endif

using namespace cx;

#ifdef _WIN32
#if defined(_M_IX86) || defined(__i386__)
#error "JIT libc pinning assumes unprefixed symbol names (x64/ARM64); 32-bit Windows is unsupported"
#endif

// Every libc function reachable from JITed code on Windows: all `extern` declarations
// visible under `#if Windows` in std/, plus heap-family extras (realloc, _msize, ...)
// that user code or C imports may reference, plus memmove/memcmp which LLVM itself
// may emit. Casts pin the exact C signature (math.h overloads would make &name
// ambiguous). website/check_jit_pins.py verifies the table stays exhaustive.
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

static bool isMsvcrtAmbiguous(llvm::StringRef name) {
    llvm::StringRef stripped = stripAsmLabelMarker(name);
    if (pinnedLibcNames().contains(stripped)) return false;
    // Unpinned names that legacy msvcrt.dll also exports would bind there instead
    // of the UCRT (llvm/llvm-project#200554): heap/stdio state would split across
    // CRTs. Keep the link-and-exec path when that can happen.
    HMODULE msvcrt = GetModuleHandleW(L"msvcrt.dll");
    return msvcrt && GetProcAddress(msvcrt, stripped.str().c_str()) != nullptr;
}
#endif

static bool allExternsResolvable(const llvm::Module& module) {
    // Open the process handle first: symbol search only sees it after getPermanentLibrary(nullptr).
    // The math library may not be loaded into this process (Linux keeps it separate from libc);
    // preload it so JITed code can call sin(), pow(), etc.
    static bool librariesLoaded = [] {
        llvm::sys::DynamicLibrary::getPermanentLibrary(nullptr);
        for (const char* name : {"libm.so.6", "libm.so"}) {
            if (!llvm::sys::DynamicLibrary::LoadLibraryPermanently(name)) return true;
        }
        return false;
    }();
    (void)librariesLoaded;

    auto isResolvable = [](llvm::StringRef name) {
        if (name.empty()) return true;
#ifdef _WIN32
        if (isMsvcrtAmbiguous(name)) return false;
#endif
        return llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(stripAsmLabelMarker(name).str()) != nullptr;
    };

    for (const auto& function : module.functions()) {
        if (!function.isDeclaration() || function.isIntrinsic()) continue;
        if (!isResolvable(function.getName())) return false;
    }
    for (const auto& global : module.globals()) {
        if (!global.isDeclaration()) continue;
        if (!isResolvable(global.getName())) return false;
    }
    return true;
}

bool cx::jitEligible(const llvm::Module& linkedModule) {
    const auto* main = linkedModule.getFunction("main");
    if (!main || main->isDeclaration()) return false;
    // User mains lower to main() or main(argc, argv); anything else keeps the old path.
    if (main->arg_size() != 0 && main->arg_size() != 2) return false;
    return allExternsResolvable(linkedModule);
}

#ifdef _WIN32
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

// Several libc names are function-like macros on MSVC; undefine them so the pins take
// the real functions (each also exists as a function; the link fails loudly if not).
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

static void pinLibcSymbols(llvm::orc::LLJIT& jit, llvm::ExitOnError& exitOnError) {
    // ORC process-symbol lookup binds libc calls to legacy msvcrt.dll instead of the
    // UCRT the host uses: floats print legacy formats (1e+010, 1.#INF), puts/printf
    // split across CRT buffers, and _fdopen stderr output is lost
    // (llvm/llvm-project#200554). Definitions in a JITDylib beat its generators, so pin
    // every reachable libc name to the host's address (following ossia/score#2127).
    // The table must stay exhaustive per interacting group (heap, stdio, ...); anything
    // still unpinned that msvcrt.dll also exports keeps the link-and-exec path via
    // isMsvcrtAmbiguous.
    auto& executionSession = jit.getExecutionSession();
    llvm::orc::SymbolMap pinned;
#define PIN(name, sig) \
    pinned[executionSession.intern(#name)] = {llvm::orc::ExecutorAddr::fromPtr(static_cast<sig>(&name)), \
                                              llvm::JITSymbolFlags::Exported | llvm::JITSymbolFlags::Callable};
    CX_JIT_LIBC_PINS(PIN)
#undef PIN
    exitOnError(jit.getMainJITDylib().define(llvm::orc::absoluteSymbols(std::move(pinned))));
}
#endif

int cx::jitRun(std::unique_ptr<llvm::Module> linkedModule, std::unique_ptr<llvm::LLVMContext> ctx, const std::string& argv0,
               const std::vector<std::string>& programArgs) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::ExitOnError exitOnError;
    exitOnError.setBanner("error: couldn't JIT-run the program: ");
    auto jit = exitOnError(llvm::orc::LLJITBuilder().create());
    auto processSymbols = exitOnError(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(jit->getDataLayout().getGlobalPrefix()));
    jit->getMainJITDylib().addGenerator(std::move(processSymbols));
#ifdef _WIN32
    pinLibcSymbols(*jit, exitOnError);
#endif

    auto* mainFunction = linkedModule->getFunction("main");
    ASSERT(mainFunction && "jitEligible guarantees a defined main");
    bool mainTakesArgs = mainFunction->arg_size() == 2;
    exitOnError(jit->addIRModule(llvm::orc::ThreadSafeModule(std::move(linkedModule), std::move(ctx))));
    auto mainSymbol = exitOnError(jit->lookup("main"));

    std::vector<char*> argv;
    argv.reserve(programArgs.size() + 2);
    argv.push_back(const_cast<char*>(argv0.c_str()));
    for (const auto& arg : programArgs) {
        argv.push_back(const_cast<char*>(arg.c_str()));
    }
    int argc = static_cast<int>(argv.size());
    argv.push_back(nullptr); // argv[argc] == NULL per the C standard.

    if (mainTakesArgs) {
        auto* main = mainSymbol.toPtr<int(int, char**)>();
        return main(argc, argv.data());
    }
    auto* main = mainSymbol.toPtr<int()>();
    return main();
}
