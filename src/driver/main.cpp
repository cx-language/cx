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
#include <llvm/Support/Program.h>
#include <llvm/Support/Path.h>
#include "repl.h"
#include "../ast/ast_printer.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../irgen/irgen.h"
#include "../parser/parse.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"

using namespace delta;

namespace {

/// If `args` contains `flag`, removes it and returns true, otherwise returns false.
bool checkFlag(llvm::StringRef flag, std::vector<llvm::StringRef>& args) {
    const auto it = std::find(args.begin(), args.end(), flag);
    const bool contains = it != args.end();
    if (contains) args.erase(it);
    return contains;
}

std::vector<llvm::StringRef> collectStringOptionValues(llvm::StringRef flagPrefix,
                                                       std::vector<llvm::StringRef>& args) {
    std::vector<llvm::StringRef> values;
    for (auto arg = args.begin(); arg != args.end();) {
        if (arg->startswith(flagPrefix)) {
            values.push_back(arg->substr(flagPrefix.size()));
            arg = args.erase(arg);
        } else {
            ++arg;
        }
    }
    return values;
}

void emitMachineCode(llvm::Module& module, llvm::StringRef fileName,
                     llvm::TargetMachine::CodeGenFileType fileType,
                     llvm::Reloc::Model relocModel) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    std::string targetTriple = llvm::sys::getDefaultTargetTriple();
    module.setTargetTriple(targetTriple);

    std::string errorMessage;
    auto* target = llvm::TargetRegistry::lookupTarget(targetTriple, errorMessage);
    if (!target) printErrorAndExit(errorMessage);

    llvm::TargetOptions options;
    auto* targetMachine = target->createTargetMachine(targetTriple, "generic", "", options,
                                                      relocModel);
    module.setDataLayout(targetMachine->createDataLayout());

    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::F_None);
    if (error) printErrorAndExit(error.message());

    llvm::legacy::PassManager passManager;
    if (targetMachine->addPassesToEmitFile(passManager, file, fileType)) {
        printErrorAndExit("TargetMachine can't emit a file of this type");
    }

    passManager.run(module);
    file.flush();
}

void printHelp() {
    llvm::outs() <<
        "OVERVIEW: Delta compiler\n"
        "\n"
        "USAGE: delta [options] <inputs>\n"
        "\n"
        "OPTIONS:\n"
        "  -c                    - Compile only, generating an .o file; don't link\n"
        "  -emit-assembly        - Emit assembly code\n"
        "  -fPIC                 - Emit position-independent code\n"
        "  -parse                - Perform parsing\n"
        "  -typecheck            - Perform parsing and type checking\n"
        "  -help                 - Display this help\n"
        "  -I<directory>         - Add a search path for module and C header import\n"
        "  -print-ast            - Print the abstract syntax tree to stdout\n"
        "  -print-ir             - Print the generated LLVM IR to stdout\n";
}

} // anonymous namespace

int main(int argc, char** argv) try {
    if (argc == 1) return replMain();

    std::vector<llvm::StringRef> args(argv + 1, argv + argc);

    if (checkFlag("-help", args) || checkFlag("--help", args) || checkFlag("-h", args)) {
        printHelp();
        return 0;
    }

    const bool parseFlag = checkFlag("-parse", args);
    const bool typecheckFlag = checkFlag("-typecheck", args);
    const bool compileOnly = checkFlag("-c", args);
    const bool printAST = checkFlag("-print-ast", args);
    const bool printIR = checkFlag("-print-ir", args);
    const bool emitAssembly = checkFlag("-emit-assembly", args) || checkFlag("-S", args);
    const bool emitPositionIndependentCode = checkFlag("-fPIC", args);
    std::vector<llvm::StringRef> importSearchPaths = collectStringOptionValues("-I", args);
    importSearchPaths.push_back(".");
    importSearchPaths.push_back(DELTA_ROOT_DIR); // For development.

    for (llvm::StringRef arg : args) {
        if (arg.startswith("-")) {
            printErrorAndExit("unsupported option '", arg, "'");
        }
    }

    if (args.empty()) {
        printErrorAndExit("no input files");
    }

    Module module("main");

    for (llvm::StringRef filePath : args) {
        parse(filePath, module);
        importSearchPaths.push_back(llvm::sys::path::parent_path(filePath)); // TODO: Don't add duplicates.
    }

    if (printAST) {
        std::cout << module;
        return 0;
    }

    if (parseFlag) return 0;

    for (auto& importedModule : module.getImportedModules()) {
        typecheckModule(*importedModule, importSearchPaths, parse);
    }
    typecheckModule(module, importSearchPaths, parse);

    if (typecheckFlag) return 0;

    for (auto& module : getAllImportedModules()) {
        irgen::compile(*module);
    }
    auto& irModule = irgen::compile(module);

    if (printIR) {
        irModule.print(llvm::outs(), nullptr);
        return 0;
    }

    std::string outputFile = emitAssembly ? "output.s" : "output.o";
    auto fileType = emitAssembly ? llvm::TargetMachine::CGFT_AssemblyFile
                                 : llvm::TargetMachine::CGFT_ObjectFile;
    auto relocModel = emitPositionIndependentCode ? llvm::Reloc::Model::PIC_
                                                  : llvm::Reloc::Model::Static;
    emitMachineCode(irModule, outputFile, fileType, relocModel);

    if (compileOnly || emitAssembly) return 0;

    // Link the output.

    llvm::ErrorOr<std::string> ccPath = llvm::sys::findProgramByName("cc");
    if (!ccPath) {
        printErrorAndExit("couldn't find C compiler");
    }

    const char* ccArgs[] = { ccPath->c_str(), outputFile.c_str(), nullptr };
    int ccExitStatus = llvm::sys::ExecuteAndWait(ccArgs[0], ccArgs);
    std::remove(outputFile.c_str());
    return ccExitStatus;
} catch (const CompileError& error) {
    error.print();
    exit(1);
}
