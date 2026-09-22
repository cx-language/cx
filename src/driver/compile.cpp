#include "compile.h"
#include "../ast/module.h"
#include "../backend/c-backend.h"
#include "../backend/irgen.h"
#include "../parser/parse.h"
#include "../sema/null-analyzer.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"
#include "driver.h"
#pragma warning(push, 0)
#include <llvm/Support/MemoryBuffer.h>
#pragma warning(pop)

using namespace cx;

llvm::MemoryBufferRef cx::addFileBufferToModule(llvm::StringRef filePath, Module& targetModule) {
    auto buffer = llvm::MemoryBuffer::getFile(filePath);
    if (!buffer) ABORT("couldn't open file '" << filePath << "'");
    ASSERT((*buffer)->getBufferIdentifier() == filePath);
    targetModule.fileBuffers.push_back(std::move(*buffer));
    return targetModule.fileBuffers.back()->getMemBufferRef();
}

CompileToCResult cx::compileToC(const char* fileName, const char* source, const CompileToCOptions& options) {
    // Diagnostics must reach the caller incrementally: API users such as the
    // WebAssembly build capture stdout/stderr, where buffered output might
    // never be flushed (there is no process exit to flush it).
    llvm::outs().SetUnbuffered();
    llvm::errs().SetUnbuffered();

    Module mainModule("main");
    auto buffer = llvm::MemoryBuffer::getMemBufferCopy(source, fileName);
    mainModule.fileBuffers.push_back(std::move(buffer));

    CompileOptions compileOptions;
    compileOptions.noUnusedWarnings = options.noUnusedWarnings;
    compileOptions.importSearchPaths = options.importSearchPaths;

    // The stages below mirror buildModule()'s frontend stages (see driver.cpp).
    for (auto& fileBuffer : mainModule.fileBuffers) {
        Parser parser(fileBuffer->getMemBufferRef(), mainModule, compileOptions);
        parser.parse();
    }

    if (errors) return CompileToCResult{.status = 1};

    Typechecker typechecker(compileOptions);
    for (auto& importedModule : mainModule.getImportedModules()) {
        typechecker.typecheckModule(*importedModule, compileOptions);
    }
    typechecker.typecheckModule(mainModule, compileOptions);
    typechecker.checkUnusedDecls(mainModule);

    if (errors) return CompileToCResult{.status = 1};

    IRGenerator irGenerator(compileOptions);
    for (auto* importedModule : Module::getAllImportedModules()) {
        irGenerator.emitModule(*importedModule);
    }
    irGenerator.emitModule(mainModule);

    NullAnalyzer nullAnalyzer;
    for (auto module : irGenerator.generatedModules) {
        nullAnalyzer.analyze(module);
    }

    if (errors) return CompileToCResult{.status = 1};

    CGenerator cGenerator(options.dispatchMode);
    for (auto* irModule : irGenerator.generatedModules) {
        cGenerator.codegenModule(*irModule);
    }

    return CompileToCResult{.status = 0, .cCode = cGenerator.finish()};
}
