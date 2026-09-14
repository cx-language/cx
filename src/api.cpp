#include "cx.h"
#ifdef _WIN32
// TODO
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
        auto mangledName = mangleFunctionDecl(*functionDecl);

#ifdef _WIN32
        // TODO
#else
        void* lib = dlopen("main.so", RTLD_LAZY);
        function.ptr = dlsym(lib, mangledName.c_str());
#endif
    }

    return function;
}

} // extern "C"
