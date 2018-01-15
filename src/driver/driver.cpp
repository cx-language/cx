#include <iostream>
#include <string>
#include <system_error>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Bitcode/BitcodeWriter.h>
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
#pragma warning(pop)
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

    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    const std::string& targetTriple = triple.str();
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

void emitLLVMBitcode(const llvm::Module& module, llvm::StringRef fileName) {
    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::F_None);
    if (error) printErrorAndExit(error.message());
    llvm::WriteBitcodeToFile(&module, file);
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
    bool emitLLVMBitcode = checkFlag("-emit-llvm-bitcode", args);
    bool emitPositionIndependentCode = checkFlag("-fPIC", args);
    treatWarningsAsErrors = checkFlag("-Werror", args);
    auto defines = collectStringOptionValues("-D", args);
#ifdef _WIN32
    defines.push_back("Windows");
#endif
    auto importSearchPaths = collectStringOptionValues("-I", args);
    auto frameworkSearchPaths = collectStringOptionValues("-F", args);
    importSearchPaths.push_back(DELTA_ROOT_DIR); // For development.

    for (llvm::StringRef arg : args) {
        if (arg.startswith("-")) {
            printErrorAndExit("unsupported option '", arg, "'");
        }
    }

    if (files.empty()) {
        printErrorAndExit("no input files");
    }

    Module module("main", std::move(defines));
    llvm::StringSet<> relativeImportSearchPaths;

    for (llvm::StringRef filePath : files) {
        Parser parser(filePath, module);
        parser.parse();

        auto directoryPath = llvm::sys::path::parent_path(filePath);
        if (directoryPath.empty()) directoryPath = ".";
        relativeImportSearchPaths.insert(directoryPath);
    }

    for (auto& keyValue : relativeImportSearchPaths) {
        importSearchPaths.push_back(keyValue.getKey());
    }

    if (printAST) {
        ASTPrinter astPrinter(std::cout);
        astPrinter.printModule(module);
        return 0;
    }

    if (parse) return 0;

    Typechecker typechecker;
    for (auto& importedModule : module.getImportedModules()) {
        typechecker.typecheckModule(*importedModule, /* TODO: Pass the manifest of `*importedModule` here. */ nullptr,
                                    importSearchPaths, frameworkSearchPaths);
    }
    typechecker.typecheckModule(module, manifest, importSearchPaths, frameworkSearchPaths);

    bool treatAsLibrary = !module.getSymbolTable().contains("main") && !run;
    if (treatAsLibrary) {
        compileOnly = true;
    }

    if (typecheck) return 0;

    IRGenerator irGenerator;
    for (auto* module : Module::getAllImportedModules()) {
        irGenerator.compile(*module);
    }
    auto& irModule = irGenerator.compile(module);

    if (printIR) {
        irModule.print(llvm::outs(), nullptr);
        return 0;
    }

    if (emitLLVMBitcode) {
        ::emitLLVMBitcode(irModule, "output.bc");
        return 0;
    }

    auto ccPath = getCCompilerPath();
    bool msvc = llvm::StringRef(ccPath).endswith_lower("cl.exe");

    llvm::SmallString<128> temporaryOutputFilePath;
    auto* outputFileExtension = emitAssembly ? "s" : msvc ? "obj" : "o";
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
    const char* executableNamePattern = msvc ? "delta-%%%%%%%%.exe" : "delta-%%%%%%%%.out";

    if (auto error = llvm::sys::fs::createUniqueFile(executableNamePattern, temporaryExecutablePath)) {
        printErrorAndExit(error.message());
    }

    std::vector<const char*> ccArgs = {
        ccPath.c_str(),
        temporaryOutputFilePath.c_str(),
#ifdef __linux__
        "-static"
#endif
    };

    std::string outputPathFlag = ((msvc ? "-Fe" : "-o") + temporaryExecutablePath).str();
    ccArgs.push_back(outputPathFlag.c_str());

    if (msvc) {
        ccArgs.push_back("-link");
        ccArgs.push_back("-DEBUG");

        // TODO: This probably won't work with pre-2015 Visual Studio.
        ccArgs.push_back("legacy_stdio_definitions.lib");
        ccArgs.push_back("ucrt.lib");
        ccArgs.push_back("msvcrt.lib");
    }

    ccArgs.push_back(nullptr);

    // Redirect stdout and stderr to files to prevent them from interfering with tests.
    llvm::StringRef out = "c-compiler-stdout.txt";
    llvm::StringRef err = "c-compiler-stderr.txt";
    const llvm::StringRef* redirects[3] = { nullptr, &out, &err };

    int ccExitStatus = llvm::sys::ExecuteAndWait(ccArgs[0], ccArgs.data(), nullptr, redirects);
    llvm::sys::fs::remove(temporaryOutputFilePath);
    if (ccExitStatus != 0) return ccExitStatus;

    if (run) {
        std::string error;
        const char* executableArgs[] = { temporaryExecutablePath.c_str(), nullptr };
        int executableExitStatus = llvm::sys::ExecuteAndWait(executableArgs[0], executableArgs,
                                                             nullptr, nullptr, 0, 0, &error);
        llvm::sys::fs::remove(temporaryExecutablePath);

        if (msvc) {
            auto path = temporaryExecutablePath;
            llvm::sys::path::replace_extension(path, "ilk");
            llvm::sys::fs::remove(path);
            llvm::sys::path::replace_extension(path, "pdb");
            llvm::sys::fs::remove(path);
        }

        if (!error.empty()) {
            llvm::outs() << error << '\n';
        }

        return executableExitStatus;
    }

    llvm::StringRef executableNameStem = "a";
    llvm::sys::fs::rename(temporaryExecutablePath, executableNameStem + (msvc ? ".exe" : ".out"));

    if (msvc) {
        auto path = temporaryExecutablePath;
        llvm::sys::path::replace_extension(path, "ilk");
        llvm::sys::fs::rename(path, executableNameStem + ".ilk");
        llvm::sys::path::replace_extension(path, "pdb");
        llvm::sys::fs::rename(path, executableNameStem + ".pdb");
    }

    return 0;
}
