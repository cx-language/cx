#include "driver.h"
#include <cstdio>
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
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Host.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#pragma warning(pop)
#include "clang_driver.h"
#include "../ast/module.h"
#include "../backend/irgen.h"
#include "../backend/llvm.h"
#include "../package-manager/manifest.h"
#include "../package-manager/package-manager.h"
#include "../parser/parse.h"
#include "../sema/null-analyzer.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"

#ifdef _MSC_VER
#define popen _popen
#define pclose _pclose
#define WEXITSTATUS(x) x
#endif

using namespace cx;
namespace cl = llvm::cl;

namespace cx {
int errors = 0;
cl::SubCommand build("build", "Build a C* project");
cl::SubCommand run("run", "Build and run a C* executable");
cl::list<std::string> inputs(cl::Positional, cl::desc("<input files>"), cl::sub(*cl::AllSubCommands));
cl::opt<bool> parse("parse", cl::desc("Parse only"));
cl::opt<bool> typecheck("typecheck", cl::desc("Parse and type-check only"));
cl::opt<bool> compileOnly("c", cl::desc("Compile only, generating an object file; don't link"));
cl::opt<bool> printIR("print-ir", cl::desc("Print C* intermediate representation of main module"), cl::sub(build), cl::sub(*cl::TopLevelSubCommand));
cl::opt<bool> printIRAll("print-ir-all", cl::desc("Print C* intermediate representation of all compiled modules"), cl::sub(build), cl::sub(*cl::TopLevelSubCommand));
cl::opt<bool> printLLVM("print-llvm", cl::desc("Print LLVM intermediate representation of main module"), cl::sub(build), cl::sub(*cl::TopLevelSubCommand));
// TODO: Add -print-llvm-all option.
cl::opt<bool> emitAssembly("emit-assembly", cl::desc("Emit assembly code"));
cl::opt<bool> emitBitcode("emit-llvm-bitcode", cl::desc("Emit LLVM bitcode"));
cl::opt<bool> emitPositionIndependentCode("fPIC", cl::desc("Emit position-independent code"), cl::sub(*cl::AllSubCommands));
cl::opt<std::string> specifiedOutputFileName("o", cl::desc("Specify output file name"));
cl::opt<WarningMode> warningMode(cl::desc("Warning mode:"), cl::sub(*cl::AllSubCommands),
                                 cl::values(clEnumValN(WarningMode::Suppress, "w", "Suppress all warnings"),
                                            clEnumValN(WarningMode::TreatAsErrors, "Werror", "Treat warnings as errors")));
cl::list<std::string> disabledWarnings("Wno-", cl::desc("Disable warnings"), cl::value_desc("warning"), cl::Prefix, cl::sub(*cl::AllSubCommands));
cl::list<std::string> defines("D", cl::desc("Specify defines"), cl::Prefix, cl::sub(*cl::AllSubCommands));
cl::list<std::string> importSearchPaths("I", cl::desc("Add directory to import search paths"), cl::value_desc("path"), cl::Prefix, cl::sub(*cl::AllSubCommands));
cl::list<std::string> frameworkSearchPaths("F", cl::desc("Add directory to framework search paths"), cl::value_desc("path"), cl::Prefix,
                                           cl::sub(*cl::AllSubCommands));
cl::list<std::string> cflags(cl::Sink, cl::desc("Add C compiler flags"), cl::sub(*cl::AllSubCommands));
cl::alias emitAssemblyAlias("S", cl::aliasopt(emitAssembly));
} // namespace cx

static int exec(const char* command, std::string& output) {
    FILE* pipe = popen(command, "r");
    if (!pipe) {
        ABORT("failed to execute '" << command << "'");
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

static void addHeaderSearchPathsFromEnvVar(const char* name) {
    if (auto pathList = llvm::sys::Process::GetEnv(name)) {
        llvm::SmallVector<llvm::StringRef, 16> paths;
        llvm::StringRef(*pathList).split(paths, llvm::sys::EnvPathSeparator, -1, false);

        for (llvm::StringRef path : paths) {
            importSearchPaths.push_back(path.str());
        }
    }
}

static void addHeaderSearchPathsFromCCompilerOutput() {
    auto compilerPath = getCCompilerPath();
    if (compilerPath.empty()) return;

    if (llvm::sys::path::filename(compilerPath) != "cl.exe") {
        std::string command = "echo | " + compilerPath + " -E -v - 2>&1 | grep '^ /'";
        std::string output;
        exec(command.c_str(), output);

        llvm::SmallVector<llvm::StringRef, 8> lines;
        llvm::SplitString(output, lines, "\n");

        for (auto line : lines) {
            auto path = line.trim();
            if (llvm::sys::fs::is_directory(path)) {
                importSearchPaths.push_back(path.str());
            }
        }
    }
}

static void addPredefinedImportSearchPaths(llvm::ArrayRef<std::string> inputFiles) {
    llvm::StringSet<> relativeImportSearchPaths;

    for (llvm::StringRef filePath : inputFiles) {
        auto directoryPath = llvm::sys::path::parent_path(filePath);
        if (directoryPath.empty()) directoryPath = ".";
        relativeImportSearchPaths.insert(directoryPath);
    }

    for (auto& keyValue : relativeImportSearchPaths) {
        importSearchPaths.push_back(keyValue.getKey().str());
    }

    importSearchPaths.push_back(CX_ROOT_DIR);
    importSearchPaths.push_back(CLANG_BUILTIN_INCLUDE_PATH);
    importSearchPaths.push_back("/usr/include");
    importSearchPaths.push_back("/usr/local/include");
    addHeaderSearchPathsFromEnvVar("CPATH");
    addHeaderSearchPathsFromEnvVar("C_INCLUDE_PATH");
    addHeaderSearchPathsFromEnvVar("INCLUDE");
    addHeaderSearchPathsFromCCompilerOutput();
}

static void emitMachineCode(llvm::Module& module, llvm::StringRef fileName, llvm::CodeGenFileType fileType, llvm::Reloc::Model relocModel) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    const std::string& targetTriple = triple.str();
    module.setTargetTriple(targetTriple);

    std::string errorMessage;
    auto* target = llvm::TargetRegistry::lookupTarget(targetTriple, errorMessage);
    if (!target) ABORT(errorMessage);

    llvm::TargetOptions options;
    auto* targetMachine = target->createTargetMachine(targetTriple, "generic", "", options, relocModel);
    module.setDataLayout(targetMachine->createDataLayout());

    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::F_None);
    if (error) ABORT(error.message());

    llvm::legacy::PassManager passManager;
    if (targetMachine->addPassesToEmitFile(passManager, file, nullptr, fileType)) {
        ABORT("TargetMachine can't emit a file of this type");
    }

    passManager.run(module);
    file.flush();
}

static void emitLLVMBitcode(const llvm::Module& module, llvm::StringRef fileName) {
    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::F_None);
    if (error) ABORT(error.message());
    llvm::WriteBitcodeToFile(module, file);
    file.flush();
}

static int buildExecutable(llvm::ArrayRef<std::string> files, const PackageManifest* manifest, const char* argv0, llvm::StringRef outputDirectory,
                           std::string outputFileName) {
    if (files.empty()) {
        ABORT("no input files");
    }

    addPredefinedImportSearchPaths(files);

    CompileOptions options = { disabledWarnings, importSearchPaths, frameworkSearchPaths, defines, cflags };

    if (!specifiedOutputFileName.empty()) {
        outputFileName = specifiedOutputFileName;
    }

    Module mainModule("main");
    std::vector<std::string> nonCxInputFiles;

    for (auto& filePath : files) {
        if (llvm::StringRef(filePath).endswith(".cx")) {
            Parser parser(filePath, mainModule, options);
            parser.parse();
        } else {
            llvm::SmallString<128> absolutePath(filePath);
            llvm::sys::fs::make_absolute(absolutePath);
            nonCxInputFiles.push_back(absolutePath.str().str());
        }
    }

    if (parse) return errors ? 1 : 0;

    Typechecker typechecker(options);
    for (auto& importedModule : mainModule.getImportedModules()) {
        typechecker.typecheckModule(*importedModule, nullptr);
    }
    typechecker.typecheckModule(mainModule, manifest);

    if (errors) return 1;

    IRGenerator irGenerator;
    for (auto* importedModule : Module::getAllImportedModules()) {
        irGenerator.emitModule(*importedModule);
    }
    irGenerator.emitModule(mainModule);

    NullAnalyzer nullAnalyzer;
    for (auto module : irGenerator.generatedModules) {
        nullAnalyzer.analyze(module);
    }

    if (errors) return 1;
    if (typecheck) return 0;

    if (printIRAll) {
        for (auto* module : irGenerator.generatedModules) {
            module->print(llvm::outs());
        }
        return 0;
    } else if (printIR) {
        irGenerator.generatedModules.back()->print(llvm::outs());
        return 0;
    }

    LLVMGenerator llvmGenerator;
    for (auto* irModule : irGenerator.generatedModules) {
        llvmGenerator.codegenModule(*irModule);
    }
    llvm::Module* llvmModule = llvmGenerator.generatedModules.back();

    if (printLLVM) {
        llvmModule->setModuleIdentifier("");
        llvmModule->setSourceFileName("");
        llvmModule->print(llvm::outs(), nullptr);
        return 0;
    }

    llvm::Module linkedModule("", llvmGenerator.ctx);
    llvm::Linker linker(linkedModule);

    for (auto& module : llvmGenerator.generatedModules) {
        bool error = linker.linkInModule(std::unique_ptr<llvm::Module>(module));
        if (error) ABORT("LLVM module linking failed");
    }

    if (emitBitcode) {
        emitLLVMBitcode(linkedModule, "output.bc");
        return 0;
    }

    auto ccPath = getCCompilerPath();
    bool msvc = llvm::sys::path::extension(ccPath) == ".exe";

    llvm::SmallString<128> temporaryOutputFilePath;
    auto* outputFileExtension = emitAssembly ? "s" : msvc ? "obj" : "o";
    if (auto error = llvm::sys::fs::createTemporaryFile("cx", outputFileExtension, temporaryOutputFilePath)) {
        ABORT(error.message());
    }

    auto fileType = emitAssembly ? llvm::CGFT_AssemblyFile : llvm::CGFT_ObjectFile;
    if (msvc) emitPositionIndependentCode = true;
    auto relocModel = emitPositionIndependentCode ? llvm::Reloc::Model::PIC_ : llvm::Reloc::Model::Static;
    emitMachineCode(linkedModule, temporaryOutputFilePath, fileType, relocModel);

    if (!outputDirectory.empty()) {
        auto error = llvm::sys::fs::create_directories(outputDirectory);
        if (error) ABORT(error.message());
    }

    bool treatAsLibrary = mainModule.getSymbolTable().find("main").empty() && !run;
    if (treatAsLibrary) {
        compileOnly = true;
    }

    if (compileOnly || emitAssembly) {
        llvm::SmallString<128> outputFilePath = outputDirectory;
        llvm::sys::path::append(outputFilePath, llvm::Twine("output.") + outputFileExtension);
        renameFile(temporaryOutputFilePath, outputFilePath);
        return 0;
    }

    // Link the output.

    llvm::SmallString<128> temporaryExecutablePath;
    llvm::sys::fs::createUniquePath(msvc ? "cx-%%%%%%%%.exe" : "cx-%%%%%%%%.out", temporaryExecutablePath, true);

    std::vector<const char*> ccArgs = {
        msvc ? ccPath.c_str() : argv0,
        temporaryOutputFilePath.c_str(),
    };

    for (auto& filePath : nonCxInputFiles) {
        ccArgs.push_back(filePath.c_str());
    }

    ccArgs.push_back(msvc ? "-Fe:" : "-o");
    ccArgs.push_back(temporaryExecutablePath.c_str());

    for (auto& cflag : options.cflags) {
        ccArgs.push_back(cflag.c_str());
    }

    if (msvc) {
        ccArgs.push_back("-link");
        ccArgs.push_back("-DEBUG");
        ccArgs.push_back("legacy_stdio_definitions.lib");
        ccArgs.push_back("ucrt.lib");
        ccArgs.push_back("msvcrt.lib");
    }

    std::vector<llvm::StringRef> ccArgStringRefs(ccArgs.begin(), ccArgs.end());
    std::vector<std::string> aa;
    std::vector<const char*> bb;
    for (auto& p : ccArgs) {aa.push_back(p);llvm::outs()<<p<<" ";}
    for (auto& p : aa) {bb.push_back(p.c_str());}
    int ccExitStatus = msvc ? llvm::sys::ExecuteAndWait(ccArgs[0], ccArgStringRefs) : clang_main(bb.size(), bb.data());
    llvm::sys::fs::remove(temporaryOutputFilePath);
    if (ccExitStatus != 0) return ccExitStatus;

    if (run) {
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
        outputFileName = files.size() == 1 ? llvm::sys::path::stem(files[0]).str() : "main";
        outputFileName.append(msvc ? ".exe" : ".out");
    }

    renameFile(temporaryExecutablePath, outputPathPrefix + outputFileName);

    if (msvc) {
        auto path = temporaryExecutablePath;
        auto outputPath = outputPathPrefix;
        outputPath += outputFileName;

        llvm::sys::path::replace_extension(path, "ilk");
        llvm::sys::path::replace_extension(outputPath, "ilk");
        renameFile(path, outputPath);

        llvm::sys::path::replace_extension(path, "pdb");
        llvm::sys::path::replace_extension(outputPath, "pdb");
        renameFile(path, outputPath);
    }

    return 0;
}

static int buildPackage(llvm::StringRef packageRoot, const char* argv0) {
    auto manifestPath = (packageRoot + "/" + PackageManifest::manifestFileName).str();
    PackageManifest manifest(packageRoot.str());
    fetchDependencies(packageRoot);

    for (auto& targetRootDir : manifest.getTargetRootDirectories()) {
        llvm::StringRef outputFileName;
        if (manifest.isMultiTarget() || manifest.getPackageName().empty()) {
            outputFileName = llvm::sys::path::filename(targetRootDir);
        } else {
            outputFileName = manifest.getPackageName();
        }
        auto sourceFiles = getSourceFiles(targetRootDir, manifestPath);
        // TODO: Add support for library packages.
        int exitStatus = buildExecutable(sourceFiles, &manifest, argv0, manifest.getOutputDirectory(), outputFileName.str());
        if (exitStatus != 0) return exitStatus;
    }

    return 0;
}

static void addPlatformCompileOptions() {
    cflags.push_back("-fgnuc-version=4.2.1");
#ifdef _WIN32
    defines.push_back("Windows");
    cflags.push_back("-fms-extensions");
#endif
#ifdef __APPLE__
    std::string sdkPath;
    exec("xcrun --show-sdk-path", sdkPath);
    sdkPath = llvm::StringRef(sdkPath).trim();
    if (!sdkPath.empty()) {
        cflags.push_back("-isysroot");
        cflags.push_back(std::move(sdkPath));
    }
#endif
}

int main(int argc, const char** argv) {
    llvm::setBugReportMsg("Please submit a bug report to https://github.com/cx-language/cx/issues and include the crash backtrace.\n");
    llvm::InitLLVM x(argc, argv);
    cl::ParseCommandLineOptions(argc, argv, "C* compiler\n");
    addPlatformCompileOptions();

    if (!inputs.empty()) {
        return buildExecutable(inputs, nullptr, argv[0], ".", "");
    } else if (build || run) {
        llvm::SmallString<128> currentPath;
        if (auto error = llvm::sys::fs::current_path(currentPath)) {
            ABORT(error.message());
        }
        return buildPackage(currentPath, argv[0]);
    } else {
        cl::PrintHelpMessage();
        return 0;
    }
}
