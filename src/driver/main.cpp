#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include <boost/utility/string_ref.hpp>
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
#include "../cgen/codegen.h"
#include "../irgen/irgen.h"

extern FILE* inputFile;
const char* currentFileName;
int yyparse();
extern std::vector<Decl> globalAST;

namespace {

/// If `args` contains `flag`, removes it and returns true, otherwise returns false.
bool checkFlag(boost::string_ref flag, std::vector<boost::string_ref>& args) {
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
    if (argc == 1) {
        std::cout << "error: no input files" << std::endl;
        return 1;
    }

    std::vector<boost::string_ref> args(argv + 1, argv + argc);
    const bool syntaxOnly = checkFlag("-fsyntax-only", args);
    const bool compileOnly = checkFlag("-c", args);
    const bool printAST = checkFlag("-print-ast", args);
    const bool outputToStdout = checkFlag("-o=stdout", args);
    const bool codegenC = checkFlag("-codegen=c", args);
    const bool emitAssembly = checkFlag("-emit-assembly", args) || checkFlag("-S", args);

    for (boost::string_ref filePath : args) {
        inputFile = fopen(filePath.data(), "rb");

        if (!inputFile) {
            std::cout << "error: no such file: '"  << filePath << "'" << std::endl;
            return 1;
        }

        currentFileName = filePath.data();
        int result = yyparse();
        if (result != 0) return result;
    }

    typecheck(globalAST);

    std::string outputFile;

    if (printAST) {
        std::cout << globalAST;
    } else if (!syntaxOnly) {
        if (!codegenC) {
            auto& module = irgen::compile(globalAST);
            if (!outputToStdout) {
                outputFile = emitAssembly ? "output.s" : "output.o";
                auto fileType = emitAssembly ? llvm::TargetMachine::CGFT_AssemblyFile
                                             : llvm::TargetMachine::CGFT_ObjectFile;
                emitMachineCode(module, outputFile, fileType);
            } else {
                module.print(llvm::outs(), nullptr);
            }
        } else {
            outputFile = "output.c";
            cgen::compile(globalAST, outputToStdout ? "stdout" : outputFile);
        }
    }

    if (!syntaxOnly && !printAST && !compileOnly && !emitAssembly) {
        // Compile (if it's C) and link the output.
        assert(!outputFile.empty());
        const int ccExitStatus = std::system(("cc " + outputFile).c_str());
        std::remove(outputFile.c_str());
        if (ccExitStatus != 0) exit(ccExitStatus);
    }
}
