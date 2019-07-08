#include "driver.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/Linker/Linker.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#pragma warning(pop)
#include "clang.h"
#include "../ast/ast-printer.h"
#include "../ast/module.h"
#include "../irgen/irgen.h"
#include "../package-manager/manifest.h"
#include "../package-manager/package-manager.h"
#include "../parser/parse.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#define WEXITSTATUS(x) x
#endif

using namespace delta;

bool delta::checkFlag(llvm::StringRef flag, std::vector<llvm::StringRef>& args) {
    const auto it = std::find(args.begin(), args.end(), flag);
    const bool contains = it != args.end();
    if (contains) args.erase(it);
    return contains;
}

namespace {

std::vector<std::string> collectStringOptionValues(llvm::StringRef flagPrefix, std::vector<llvm::StringRef>& args) {
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

std::string collectStringOptionValue(llvm::StringRef flagPrefix, std::vector<llvm::StringRef>& args) {
    auto values = collectStringOptionValues(flagPrefix, args);
    if (values.empty()) {
        return "";
    } else {
        return std::move(values.back());
    }
}

void addHeaderSearchPathsFromEnvVar(std::vector<std::string>& importSearchPaths, const char* name) {
    if (const char* pathList = std::getenv(name)) {
        llvm::SmallVector<llvm::StringRef, 16> paths;
        llvm::StringRef(pathList).split(paths, llvm::sys::EnvPathSeparator, -1, false);

        for (llvm::StringRef path : paths) {
            importSearchPaths.push_back(path);
        }
    }
}

void addHeaderSearchPathsFromCCompilerOutput(std::vector<std::string>& importSearchPaths) {
    auto compilerPath = getCCompilerPath();
    if (compilerPath.empty()) return;

    if (llvm::StringRef(compilerPath).endswith_lower("cl.exe")) {
        addHeaderSearchPathsFromEnvVar(importSearchPaths, "INCLUDE");
    } else {
        std::string command = "echo | " + compilerPath + " -E -v - 2>&1 | grep '^ /'";
        std::shared_ptr<FILE> process(popen(command.c_str(), "r"), pclose);

        while (!std::feof(process.get())) {
            std::string path;

            while (true) {
                int ch = std::fgetc(process.get());

                if (ch == EOF || ch == '\n') {
                    break;
                } else if (!path.empty() || ch != ' ') {
                    path += (char) ch;
                }
            }

            if (llvm::sys::fs::is_directory(path)) {
                importSearchPaths.push_back(path);
            }
        }
    }
}

void addPredefinedImportSearchPaths(std::vector<std::string>& importSearchPaths, llvm::ArrayRef<std::string> inputFiles) {
    llvm::StringSet<> relativeImportSearchPaths;

    for (llvm::StringRef filePath : inputFiles) {
        auto directoryPath = llvm::sys::path::parent_path(filePath);
        if (directoryPath.empty()) directoryPath = ".";
        relativeImportSearchPaths.insert(directoryPath);
    }

    for (auto& keyValue : relativeImportSearchPaths) {
        importSearchPaths.push_back(keyValue.getKey());
    }

    importSearchPaths.push_back(DELTA_ROOT_DIR);
    importSearchPaths.push_back(CLANG_BUILTIN_INCLUDE_PATH);
    addHeaderSearchPathsFromCCompilerOutput(importSearchPaths);
    importSearchPaths.push_back("/usr/include");
    importSearchPaths.push_back("/usr/local/include");
    addHeaderSearchPathsFromEnvVar(importSearchPaths, "CPATH");
    addHeaderSearchPathsFromEnvVar(importSearchPaths, "C_INCLUDE_PATH");
}

void emitMachineCode(llvm::Module& module, llvm::StringRef fileName, llvm::TargetMachine::CodeGenFileType fileType, llvm::Reloc::Model relocModel) {
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
    auto* targetMachine = target->createTargetMachine(targetTriple, "generic", "", options, relocModel);
    module.setDataLayout(targetMachine->createDataLayout());

    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::F_None);
    if (error) printErrorAndExit(error.message());

    llvm::legacy::PassManager passManager;
    if (targetMachine->addPassesToEmitFile(passManager, file, nullptr, fileType)) {
        printErrorAndExit("TargetMachine can't emit a file of this type");
    }

    passManager.run(module);
    file.flush();
}

void emitLLVMBitcode(const llvm::Module& module, llvm::StringRef fileName) {
    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::F_None);
    if (error) printErrorAndExit(error.message());
    llvm::WriteBitcodeToFile(module, file);
    file.flush();
}

} // namespace

int delta::buildPackage(llvm::StringRef packageRoot, const char* argv0, std::vector<llvm::StringRef>& args, bool run) {
    auto manifestPath = (packageRoot + "/" + PackageManifest::manifestFileName).str();
    PackageManifest manifest(packageRoot);
    fetchDependencies(packageRoot);

    for (auto& targetRootDir : manifest.getTargetRootDirectories()) {
        llvm::StringRef outputFileName;
        if (manifest.isMultiTarget() || manifest.getPackageName().empty()) {
            outputFileName = llvm::sys::path::filename(targetRootDir);
        } else {
            outputFileName = manifest.getPackageName();
        }
        // TODO: Add support for library packages.
        int exitStatus = buildExecutable(getSourceFiles(targetRootDir, manifestPath), &manifest, argv0, args, manifest.getOutputDirectory(),
                                         outputFileName, run);
        if (exitStatus != 0) return exitStatus;
    }

    return 0;
}

int exec(const char* command, std::string& output) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        printErrorAndExit("failed to execute '", command, "'");
    }

    try {
        char buffer[128];
        while (fgets(buffer, sizeof buffer, pipe)) {
            output += buffer;
        }
    } catch (...) {
        pclose(pipe);
        throw;
    }

    int status = pclose(pipe);
    return WEXITSTATUS(status);
}

int delta::buildExecutable(llvm::ArrayRef<std::string> files, const PackageManifest* manifest, const char* argv0,
                           std::vector<llvm::StringRef>& args, llvm::StringRef outputDirectory, llvm::StringRef outputFileName, bool run) {
    bool parse = checkFlag("-parse", args);
    bool typecheck = checkFlag("-typecheck", args);
    bool compileOnly = checkFlag("-c", args);
    bool printAST = checkFlag("-print-ast", args);
    bool printIR = checkFlag("-print-ir", args);
    bool emitAssembly = checkFlag("-emit-assembly", args) || checkFlag("-S", args);
    bool emitLLVMBitcode = checkFlag("-emit-llvm-bitcode", args);
    bool emitPositionIndependentCode = checkFlag("-fPIC", args);
    if (checkFlag("-w", args)) warningMode = WarningMode::Suppress;
    if (checkFlag("-Werror", args)) warningMode = WarningMode::TreatAsErrors;
    auto disabledWarnings = collectStringOptionValues("-Wno-", args);
    auto defines = collectStringOptionValues("-D", args);
#ifdef _WIN32
    defines.push_back("Windows");
#endif
    auto importSearchPaths = collectStringOptionValues("-I", args);
    auto frameworkSearchPaths = collectStringOptionValues("-F", args);
    auto specifiedOutputFileName = collectStringOptionValue("-o", args);
    if (!specifiedOutputFileName.empty()) {
        outputFileName = specifiedOutputFileName;
    }

    for (llvm::StringRef arg : args) {
        if (arg.startswith("-")) {
            printErrorAndExit("unsupported option '", arg, "'");
        }
    }

    if (files.empty()) {
        printErrorAndExit("no input files");
    }

    addPredefinedImportSearchPaths(importSearchPaths, files);
    Module module("main", std::move(defines));

    for (llvm::StringRef filePath : files) {
        Parser parser(filePath, module, importSearchPaths, frameworkSearchPaths);
        parser.parse();
    }

    if (printAST) {
        ASTPrinter astPrinter(std::cout);
        astPrinter.printModule(module);
        return 0;
    }

    if (parse) return 0;

    Typechecker typechecker(std::move(disabledWarnings));
    for (auto& importedModule : module.getImportedModules()) {
        typechecker.typecheckModule(*importedModule, nullptr, importSearchPaths, frameworkSearchPaths);
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

    auto& mainModule = irGenerator.compile(module);

    if (printIR) {
        mainModule.setModuleIdentifier("");
        mainModule.setSourceFileName("");
        mainModule.print(llvm::outs(), nullptr);
        return 0;
    }

    llvm::Module linkedModule("", irGenerator.getLLVMContext());
    llvm::Linker linker(linkedModule);

    for (auto& module : irGenerator.getGeneratedModules()) {
        bool error = linker.linkInModule(std::move(module));
        if (error) printErrorAndExit("LLVM module linking failed");
    }

    if (emitLLVMBitcode) {
        ::emitLLVMBitcode(linkedModule, "output.bc");
        return 0;
    }

    auto ccPath = getCCompilerPath();
    bool msvc = llvm::StringRef(ccPath).endswith_lower("cl.exe");

    llvm::SmallString<128> temporaryOutputFilePath;
    auto* outputFileExtension = emitAssembly ? "s" : msvc ? "obj" : "o";
    if (auto error = llvm::sys::fs::createTemporaryFile("delta", outputFileExtension, temporaryOutputFilePath)) {
        printErrorAndExit(error.message());
    }

    auto fileType = emitAssembly ? llvm::TargetMachine::CGFT_AssemblyFile : llvm::TargetMachine::CGFT_ObjectFile;
    auto relocModel = emitPositionIndependentCode ? llvm::Reloc::Model::PIC_ : llvm::Reloc::Model::Static;
    emitMachineCode(linkedModule, temporaryOutputFilePath, fileType, relocModel);

    if (!outputDirectory.empty()) {
        auto error = llvm::sys::fs::create_directories(outputDirectory);
        if (error) printErrorAndExit(error.message());
    }

    if (compileOnly || emitAssembly) {
        llvm::SmallString<128> outputFilePath = outputDirectory;
        llvm::sys::path::append(outputFilePath, llvm::Twine("output.") + outputFileExtension);
        renameFile(temporaryOutputFilePath, outputFilePath);
        return 0;
    }

    // Link the output.

    llvm::SmallString<128> temporaryExecutablePath;
    const char* executableNamePattern = msvc ? "delta-%%%%%%%%.exe" : "delta-%%%%%%%%.out";

    if (auto error = llvm::sys::fs::createUniqueFile(executableNamePattern, temporaryExecutablePath)) {
        printErrorAndExit(error.message());
    }

    std::vector<const char*> ccArgs = {
        msvc ? ccPath.c_str() : argv0,
        temporaryOutputFilePath.c_str(),
    };

    std::string outputPathFlag = ((msvc ? "-Fe" : "-o") + temporaryExecutablePath).str();
    ccArgs.push_back(outputPathFlag.c_str());

    if (msvc) {
        ccArgs.push_back("-link");
        ccArgs.push_back("-DEBUG");
        ccArgs.push_back("legacy_stdio_definitions.lib");
        ccArgs.push_back("ucrt.lib");
        ccArgs.push_back("msvcrt.lib");
    }

    // Redirect stdout and stderr to files to prevent them from interfering with tests.
    llvm::StringRef out = "c-compiler-stdout.txt";
    llvm::StringRef err = "c-compiler-stderr.txt";
    llvm::Optional<llvm::StringRef> redirects[3] = { llvm::None, out, err };

    std::vector<llvm::StringRef> ccArgStringRefs(ccArgs.begin(), ccArgs.end());
    int ccExitStatus = msvc ? llvm::sys::ExecuteAndWait(ccArgs[0], ccArgStringRefs, llvm::None, redirects) : invokeClang(ccArgs);

    llvm::sys::fs::remove(temporaryOutputFilePath);
    uint64_t fileSize;
    if (!llvm::sys::fs::file_size(out, fileSize) && fileSize == 0) llvm::sys::fs::remove(out);
    if (!llvm::sys::fs::file_size(err, fileSize) && fileSize == 0) llvm::sys::fs::remove(err);
    if (ccExitStatus != 0) return ccExitStatus;

    if (run) {
        if (auto error = llvm::sys::fs::make_absolute(temporaryExecutablePath)) {
            printErrorAndExit("couldn't make an absolute path: ", error.message());
        }

        std::string command = (temporaryExecutablePath + " 2>&1").str();
        std::string output;
        int executableExitStatus = exec(command.c_str(), output);
        llvm::outs() << output;
        llvm::sys::fs::remove(temporaryExecutablePath);

        if (msvc) {
            auto path = temporaryExecutablePath;
            llvm::sys::path::replace_extension(path, "ilk");
            llvm::sys::fs::remove(path);
            llvm::sys::path::replace_extension(path, "pdb");
            llvm::sys::fs::remove(path);
        }

        return executableExitStatus;
    }

    llvm::SmallString<128> outputPathPrefix = outputDirectory;
    if (!outputPathPrefix.empty()) {
        outputPathPrefix.append(llvm::sys::path::get_separator());
    }

    if (outputFileName.empty()) {
        outputFileName = "a";
        renameFile(temporaryExecutablePath, outputPathPrefix + outputFileName + (msvc ? ".exe" : ".out"));
    } else {
        renameFile(temporaryExecutablePath, outputPathPrefix + outputFileName + (msvc ? ".exe" : ""));
    }

    if (msvc) {
        auto path = temporaryExecutablePath;
        llvm::sys::path::replace_extension(path, "ilk");
        renameFile(path, outputPathPrefix + outputFileName + ".ilk");
        llvm::sys::path::replace_extension(path, "pdb");
        renameFile(path, outputPathPrefix + outputFileName + ".pdb");
    }

    return 0;
}
