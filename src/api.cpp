#include "cx.h"
#ifdef _WIN32
// Exclude the min/max, GDI, and OLE APIs: their macros (min, max, ERROR,
// interface) would otherwise break cx headers included below.
#define NOMINMAX
#define NOGDI
#define WIN32_LEAN_AND_MEAN
#pragma warning(push, 0)
#include <windows.h>
#pragma warning(pop)
#else
#include <dlfcn.h>
#endif
#pragma warning(push, 0)
#include <llvm/Support/CommandLine.h>
#pragma warning(pop)
#include "ast/mangle.h"
#include "ast/module.h"
#include "driver/driver.h"
#include "parser/parse.h"

using namespace cx;

extern "C" {

struct CxModule {
    Module module;
};

CxModule* cxCreateModule(const char* name) {
    return new CxModule{.module = Module(name)};
}

void cxLoadScriptFromFile(CxModule* module, const char* filePath) {
    auto fileBuffer = llvm::MemoryBuffer::getFile(filePath);
    if (!fileBuffer) {
        llvm::errs() << "Error loading script from file '" << filePath << "': " << fileBuffer.getError().message();
        abort();
    }
    module->module.fileBuffers.push_back(std::move(*fileBuffer));
}

CxCompileResult cxCompileModule(CxModule* module) {
    const char* argv[] = {nullptr, "--backend=c"};
    llvm::cl::ParseCommandLineOptions(2, argv);
    int status = buildModule(module->module, {.createSharedLib = true});
    return CxCompileResult{.status = status};
}

CxFunction cxGetFunction(CxModule* module, const char* name) {
    CxFunction function = {};
    auto* decl = module->module.symbolTable.findOne(name);

    if (auto* functionDecl = llvm::dyn_cast_or_null<FunctionDecl>(decl)) {
        // Dynamic loaders see the plain symbol; the "\01" marker is LLVM-only.
        auto symbol = stripAsmLabelMarker(mangleFunctionDecl(*functionDecl)).str();

#ifdef _WIN32
        HMODULE lib = LoadLibraryA("main.dll");
        if (lib) {
            function.ptr = reinterpret_cast<void*>(GetProcAddress(lib, symbol.c_str()));
        }
#else
        void* lib = dlopen("main.so", RTLD_LAZY);
        function.ptr = dlsym(lib, symbol.c_str());
#endif
    }

    return function;
}

} // extern "C"
