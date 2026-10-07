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
#include "../backend/jit-pins.h"
#include "../support/utility.h"

using namespace cx;

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
