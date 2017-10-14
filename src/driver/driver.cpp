#include <iostream>
#include <string>
#include <system_error>
#include <vector>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include "driver.h"
#include "../ast/ast-printer.h"
#include "../ast/module.h"
#include "../irgen/irgen.h"
#include "../support/utility.h"
#include "../package-manager/manifest.h"
#include "../package-manager/package-manager.h"
#include "../parser/parse.h"
#include "../sema/typecheck.h"

using namespace delta;

bool delta::checkFlag(llvm::StringRef flag, std::vector<llvm::StringRef>& args) {
    const auto it = std::find(args.begin(), args.end(), flag);
    const bool contains = it != args.end();
    if (contains) args.erase(it);
    return contains;
}

namespace {

std::vector<std::string> collectStringOptionValues(llvm::StringRef flagPrefix,
                                                   std::vector<llvm::StringRef>& args) {
    std::vector<std::string> values;
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

} // anonymous namespace

int delta::buildPackage(llvm::StringRef packageRoot, std::vector<llvm::StringRef>& args, bool run) {
    PackageManifest manifest(packageRoot);
    fetchDependencies(packageRoot);
    auto sourceFiles = getSourceFiles(packageRoot);

    // TODO: Add support for library packages.
    return buildExecutable(sourceFiles, &manifest, args, run);
}

int delta::buildExecutable(llvm::ArrayRef<std::string> files, const PackageManifest* manifest,
                           std::vector<llvm::StringRef>& args, bool run) {
    bool parse = checkFlag("-parse", args);
    bool typecheck = checkFlag("-typecheck", args);
    bool compileOnly = checkFlag("-c", args);
    bool printAST = checkFlag("-print-ast", args);
    bool printIR = checkFlag("-print-ir", args);
    bool emitAssembly = checkFlag("-emit-assembly", args) || checkFlag("-S", args);
    bool emitPositionIndependentCode = checkFlag("-fPIC", args);
    auto importSearchPaths = collectStringOptionValues("-I", args);
    importSearchPaths.push_back(DELTA_ROOT_DIR); // For development.

    for (llvm::StringRef arg : args) {
        if (arg.startswith("-")) {
            printErrorAndExit("unsupported option '", arg, "'");
        }
    }

    if (files.empty()) {
        printErrorAndExit("no input files");
    }

    Module module("main");
    llvm::StringSet<> relativeImportSearchPaths;

    for (llvm::StringRef filePath : files) {
        ::parse(filePath, module);

        auto directoryPath = llvm::sys::path::parent_path(filePath);
        if (directoryPath.empty()) directoryPath = ".";
        relativeImportSearchPaths.insert(directoryPath);
    }

    for (auto& keyValue : relativeImportSearchPaths) {
        importSearchPaths.push_back(keyValue.getKey());
    }

    if (printAST) {
        std::cout << module;
        return 0;
    }

    if (parse) return 0;

    for (auto& importedModule : module.getImportedModules()) {
        typecheckModule(*importedModule, /* TODO: Pass the manifest of `*importedModule` here. */ nullptr,
                        importSearchPaths, ::parse);
    }
    typecheckModule(module, manifest, importSearchPaths, ::parse);

    bool treatAsLibrary = !module.getSymbolTable().contains("main") && !run;
    if (treatAsLibrary) {
        compileOnly = true;
    }

    if (typecheck) return 0;

    IRGenerator irGenerator;
    for (auto& module : getAllImportedModules()) {
        irGenerator.compile(*module);
    }
    auto& irModule = irGenerator.compile(module);

    if (printIR) {
        irModule.print(llvm::outs(), nullptr);
        return 0;
    }

    llvm::SmallString<128> temporaryOutputFilePath;
    auto* outputFileExtension = emitAssembly ? "s" : "o";
    if (auto error = llvm::sys::fs::createTemporaryFile("delta", outputFileExtension,
                                                        temporaryOutputFilePath)) {
        printErrorAndExit(error.message());
    }

    auto fileType = emitAssembly ? llvm::TargetMachine::CGFT_AssemblyFile
                                 : llvm::TargetMachine::CGFT_ObjectFile;
    auto relocModel = emitPositionIndependentCode ? llvm::Reloc::Model::PIC_
                                                  : llvm::Reloc::Model::Static;
    emitMachineCode(irModule, temporaryOutputFilePath, fileType, relocModel);

    if (compileOnly || emitAssembly) {
        if (auto error = llvm::sys::fs::rename(temporaryOutputFilePath,
                                               llvm::Twine("output.") + outputFileExtension)) {
            printErrorAndExit(error.message());
        }
        return 0;
    }

    // Link the output.

    llvm::SmallString<128> temporaryExecutablePath;
    if (auto error = llvm::sys::fs::createUniqueFile("delta-%%%%%%%%.out", temporaryExecutablePath)) {
        printErrorAndExit(error.message());
    }

    auto ccPath = getCCompilerPath();
    const char* ccArgs[] = {
        ccPath.c_str(),
        temporaryOutputFilePath.c_str(),
        "-o",
        temporaryExecutablePath.c_str(),
#ifndef __APPLE__
        // Don't add '-static' flag on macOS to avoid "ld: library not found for -lcrt0.o" error.
        "-static",
#endif
        nullptr
    };

    int ccExitStatus = llvm::sys::ExecuteAndWait(ccArgs[0], ccArgs);
    std::remove(temporaryOutputFilePath.c_str());
    if (ccExitStatus != 0) return ccExitStatus;

    if (run) {
        const char* executableArgs[] = { temporaryExecutablePath.c_str(), nullptr };
        int executableExitStatus = llvm::sys::ExecuteAndWait(executableArgs[0], executableArgs);
        std::remove(temporaryExecutablePath.c_str());
        return executableExitStatus;
    }

    if (auto error = llvm::sys::fs::rename(temporaryExecutablePath, "a.out")) {
        printErrorAndExit(error.message());
    }

    return 0;
}
