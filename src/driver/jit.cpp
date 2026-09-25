#include "jit.h"
#pragma warning(push, 0)
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/DynamicLibrary.h>
#include <llvm/Support/TargetSelect.h>
#pragma warning(pop)
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
        // "\01" escapes an exact symbol name (see mangleFunctionDecl); strip it for lookup.
        if (name.front() == '\01') name = name.drop_front();
        return llvm::sys::DynamicLibrary::SearchForAddressOfSymbol(name.str()) != nullptr;
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

int cx::jitRun(std::unique_ptr<llvm::Module> linkedModule, std::unique_ptr<llvm::LLVMContext> ctx, const std::string& argv0,
               const std::vector<std::string>& programArgs) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();

    llvm::ExitOnError exitOnError;
    exitOnError.setBanner("error: couldn't JIT-run the program: ");
    auto jit = exitOnError(llvm::orc::LLJITBuilder().create());
    auto processSymbols = exitOnError(llvm::orc::DynamicLibrarySearchGenerator::GetForCurrentProcess(jit->getDataLayout().getGlobalPrefix()));
    jit->getMainJITDylib().addGenerator(std::move(processSymbols));

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
