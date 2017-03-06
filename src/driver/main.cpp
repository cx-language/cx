#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <llvm/ADT/StringRef.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/FileSystem.h>
#include "../ast/ast_printer.h"
#include "../ast/decl.h"
#include "../parser/parser.hpp"
#include "../sema/typecheck.h"
#include "../irgen/irgen.h"

using namespace delta;

namespace delta {

extern FILE* inputFile;
const char* currentFileName;
extern std::vector<Decl> globalAST;

}

int yyparse();

namespace {

/// If `args` contains `flag`, removes it and returns true, otherwise returns false.
bool checkFlag(llvm::StringRef flag, std::vector<llvm::StringRef>& args) {
    const auto it = std::find(args.begin(), args.end(), flag);
    const bool contains = it != args.end();
    if (contains) args.erase(it);
    return contains;
}

bool emitMachineCode(llvm::Module& module, llvm::StringRef fileName,
                     llvm::TargetMachine::CodeGenFileType fileType) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    module.setTargetTriple(targetTriple);

    std::string errorMessage;
    auto* target = llvm::TargetRegistry::lookupTarget(targetTriple, errorMessage);
    if (!target) {
        llvm::errs() << errorMessage;
        return false;
    }

    llvm::TargetOptions options;
    auto* targetMachine = target->createTargetMachine(targetTriple, "generic", "", options,
                                                      llvm::Optional<llvm::Reloc::Model>());
    module.setDataLayout(targetMachine->createDataLayout());

    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::F_None);
    if (error) {
        llvm::errs() << error.message() << '\n';
        return false;
    }

    llvm::legacy::PassManager passManager;
    if (targetMachine->addPassesToEmitFile(passManager, file, fileType)) {
        llvm::errs() << "TargetMachine can't emit a file of this type\n";
        return false;
    }

    passManager.run(module);
    file.flush();
    return true;
}

} // anonymous namespace

int main(int argc, char** argv) {
    std::vector<llvm::StringRef> args(argv + 1, argv + argc);
    const bool syntaxOnly = checkFlag("-fsyntax-only", args);
    const bool compileOnly = checkFlag("-c", args);
    const bool printAST = checkFlag("-print-ast", args);
    const bool outputToStdout = checkFlag("-o=stdout", args);
    const bool emitAssembly = checkFlag("-emit-assembly", args) || checkFlag("-S", args);

    if (args.empty()) {
        std::cout << "error: no input files" << std::endl;
        return 1;
    }

    for (llvm::StringRef filePath : args) {
        inputFile = fopen(filePath.data(), "rb");

        if (!inputFile) {
            llvm::outs() << "error: no such file: '"  << filePath << "'\n";
            return 1;
        }

        currentFileName = filePath.data();
        int result = yyparse();
        if (result != 0) return result;
    }

    typecheck(globalAST);

    if (syntaxOnly) return 0;

    if (printAST) {
        std::cout << globalAST;
        return 0;
    }

    auto& module = irgen::compile(globalAST);

    if (outputToStdout) {
        module.print(llvm::outs(), nullptr);
        return 0;
    }

    std::string outputFile = emitAssembly ? "output.s" : "output.o";
    auto fileType = emitAssembly ? llvm::TargetMachine::CGFT_AssemblyFile
                                 : llvm::TargetMachine::CGFT_ObjectFile;
    emitMachineCode(module, outputFile, fileType);

    if (compileOnly || emitAssembly) return 0;

    // Link the output.
    int ccExitStatus = std::system(("cc " + outputFile).c_str());
    std::remove(outputFile.c_str());
    return ccExitStatus;
}
