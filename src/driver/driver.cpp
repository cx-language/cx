#include "driver.h"
#include <bit>
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
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/CodeGen.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Process.h>
#include <llvm/Support/Program.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../backend/c-backend.h"
#include "../backend/irgen.h"
#include "../backend/llvm.h"
#include "../package-manager/manifest.h"
#include "../package-manager/package-manager.h"
#include "../parser/parse.h"
#include "../sema/null-analyzer.h"
#include "../sema/typecheck.h"
#include "../support/utility.h"
#include "clang.h"

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

cl::OptionCategory dependencyCategory("Dependency Options");
cl::list<std::string> inputs(cl::Positional, cl::desc("<input files>"), cl::sub(cl::SubCommand::getAll()), cl::cat(dependencyCategory));
cl::list<std::string> defines("D", cl::desc("Specify defines"), cl::Prefix, cl::sub(cl::SubCommand::getAll()), cl::cat(dependencyCategory));
cl::list<std::string> importSearchPaths("I", cl::desc("Add directory to import search paths"), cl::value_desc("path"), cl::Prefix,
                                        cl::sub(cl::SubCommand::getAll()), cl::cat(dependencyCategory));
cl::list<std::string> libraries("l", cl::desc("Link against system library"), cl::value_desc("path"), cl::Prefix, cl::sub(cl::SubCommand::getAll()),
                                cl::cat(dependencyCategory));
cl::list<std::string> librarySearchPaths("L", cl::desc("Add directory to library search paths"), cl::value_desc("path"), cl::Prefix,
                                         cl::sub(cl::SubCommand::getAll()), cl::cat(dependencyCategory));
cl::list<std::string> frameworks("framework", cl::desc("(macOS) Link against framework"), cl::value_desc("path"), cl::Prefix, cl::sub(cl::SubCommand::getAll()),
                                 cl::cat(dependencyCategory));
cl::list<std::string> frameworkSearchPaths("F", cl::desc("(macOS) Add directory to framework search paths"), cl::value_desc("path"), cl::Prefix,
                                           cl::sub(cl::SubCommand::getAll()), cl::cat(dependencyCategory));
cl::list<std::string> cflags("cflags", cl::desc("Add C compiler flags"), cl::CommaSeparated, cl::sub(cl::SubCommand::getAll()), cl::cat(dependencyCategory));

cl::OptionCategory stageSelectionCategory("Stage Selection Options");
cl::opt<bool> parse("parse", cl::desc("Parse only"), cl::cat(stageSelectionCategory));
cl::opt<bool> typecheck("typecheck", cl::desc("Parse and type-check only"), cl::cat(stageSelectionCategory));
cl::opt<bool> compileOnly("c", cl::desc("Compile only, generating an object file; don't link"), cl::cat(stageSelectionCategory));

cl::OptionCategory outputCategory("Output Options");
// TODO: Add -print-llvm-all option.
// TODO: support simultaneous -print-c and -print-llvm? (requires both C backend and LLVM backend)
enum class PrintOpt { AST, IR, IRAll, C, LLVM };
cl::bits<PrintOpt> printOpts(cl::desc("Print output from intermediate steps:"), cl::sub(build), cl::sub(cl::SubCommand::getTopLevel()), cl::cat(outputCategory),
                             cl::values(clEnumValN(PrintOpt::AST, "print-ast", "Print the abstract syntax tree of main module"),
                                        clEnumValN(PrintOpt::IR, "print-ir", "Print C* intermediate representation of main module"),
                                        clEnumValN(PrintOpt::IRAll, "print-ir-all", "Print C* intermediate representation of all compiled modules"),
                                        clEnumValN(PrintOpt::C, "print-c", "Print generated C code"),
                                        clEnumValN(PrintOpt::LLVM, "print-llvm", "Print LLVM intermediate representation of main module")));
enum class Backend { LLVM, C };
cl::opt<Backend> backend("backend", cl::desc("Select code-generation backend to use:"), cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory),
                         cl::values(clEnumValN(Backend::LLVM, "llvm", "LLVM backend (default)"), clEnumValN(Backend::C, "c", "C backend")));
cl::opt<bool> emitAssembly("emit-assembly", cl::desc("Emit assembly code"), cl::cat(outputCategory));
cl::alias emitAssemblyAlias("S", cl::aliasopt(emitAssembly), cl::cat(outputCategory));
cl::opt<bool> emitBitcode("emit-llvm-bitcode", cl::desc("Emit LLVM bitcode"), cl::cat(outputCategory));
cl::opt<bool> noPIE("no-pie", cl::desc("Don't produce a position-independent executable"), cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory));
cl::opt<std::string> specifiedOutputFileName("o", cl::desc("Specify output file name"), cl::cat(outputCategory));

cl::OptionCategory diagnosticCategory("Diagnostic Options");
cl::opt<bool> disableWarnings("w", cl::desc("Disable all warnings"), cl::sub(cl::SubCommand::getAll()), cl::cat(diagnosticCategory));
cl::opt<bool> warningsAsErrors("Werror", cl::desc("Treat warnings as errors"), cl::sub(cl::SubCommand::getAll()), cl::cat(diagnosticCategory));
cl::opt<bool> noUnusedWarnings("Wno-unused", cl::desc("Disable warnings about unused entities"), cl::sub(cl::SubCommand::getAll()),
                               cl::cat(diagnosticCategory));
cl::opt<int> errorLimit("error-limit", cl::desc("Limit the number of reported errors (10 by default, 0 removes limit)"), cl::init(10),
                        cl::sub(cl::SubCommand::getAll()), cl::cat(diagnosticCategory));

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
    auto cCompilerPath = findExternalCCompiler();
    if (!cCompilerPath) return;

    if (llvm::sys::path::filename(*cCompilerPath) != "cl.exe") {
        std::string command = "echo | " + *cCompilerPath + " -E -v - 2>&1 | grep '^ /'";
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

static void emitLLVMModuleToMachineCode(llvm::Module& module, llvm::StringRef fileName, llvm::CodeGenFileType fileType, llvm::Reloc::Model relocModel) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    module.setTargetTriple(triple);

    std::string errorMessage;
    auto* target = llvm::TargetRegistry::lookupTarget(triple, errorMessage);
    if (!target) ABORT(errorMessage);

    llvm::TargetOptions options;
    auto* targetMachine = target->createTargetMachine(triple, "generic", "", options, relocModel);
    module.setDataLayout(targetMachine->createDataLayout());

    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::OF_None);
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
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::OF_None);
    if (error) ABORT(error.message());
    llvm::WriteBitcodeToFile(module, file);
    file.flush();
}

llvm::MemoryBufferRef cx::addFileBufferToModule(llvm::StringRef filePath, Module& targetModule) {
    auto buffer = llvm::MemoryBuffer::getFile(filePath);
    if (!buffer) ABORT("couldn't open file '" << filePath << "'");
    ASSERT((*buffer)->getBufferIdentifier() == filePath);
    targetModule.fileBuffers.push_back(std::move(*buffer));
    return targetModule.fileBuffers.back()->getMemBufferRef();
}

static int buildModuleFromFiles(BuildParams buildParams) {
    Module mainModule("main");
    for (llvm::StringRef filePath : buildParams.filePaths) {
        addFileBufferToModule(filePath, mainModule);
    }
    return buildModule(mainModule, std::move(buildParams));
}

int cx::buildModule(Module& mainModule, BuildParams buildParams) {
    if (mainModule.fileBuffers.empty()) {
        ABORT("no input files");
    }

    addPredefinedImportSearchPaths(buildParams.filePaths);

    CompileOptions options = {noUnusedWarnings, importSearchPaths, frameworkSearchPaths, defines, cflags};
    auto remainingPrintOpts = std::popcount(printOpts.getBits());
    bool printSectionDividers = remainingPrintOpts > 1;

    auto handlePrintOpt = [&](PrintOpt o) {
        bool isSet = printOpts.isSet(o);
        if (isSet) remainingPrintOpts--;
        return isSet;
    };

    if (!specifiedOutputFileName.empty()) {
        buildParams.outputFileName = specifiedOutputFileName;
    }

    for (auto& fileBuffer : mainModule.fileBuffers) {
        Parser parser(*fileBuffer, mainModule, options);
        parser.parse();
    }

    if (parse) return errors ? 1 : 0;

    Typechecker typechecker(options);
    for (auto& importedModule : mainModule.getImportedModules()) {
        typechecker.typecheckModule(*importedModule, nullptr);
    }
    typechecker.typecheckModule(mainModule, buildParams.manifest);

    if (errors) return 1;

    if (handlePrintOpt(PrintOpt::AST)) {
        if (printSectionDividers) llvm::outs() << "=== BEGIN AST ===\n";
        mainModule.print(llvm::outs());
        if (printSectionDividers) llvm::outs() << "=== END AST ===\n";
        if (!remainingPrintOpts) return 0;
    }

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

    if (handlePrintOpt(PrintOpt::IRAll)) {
        handlePrintOpt(PrintOpt::IR);
        if (printSectionDividers) llvm::outs() << "=== BEGIN IR ===\n";
        for (auto* module : irGenerator.generatedModules) {
            module->print(llvm::outs());
        }
        if (printSectionDividers) llvm::outs() << "=== END IR ===\n";
        if (!remainingPrintOpts) return 0;
    } else if (handlePrintOpt(PrintOpt::IR)) {
        if (printSectionDividers) llvm::outs() << "=== BEGIN IR ===\n";
        irGenerator.generatedModules.back()->print(llvm::outs());
        if (printSectionDividers) llvm::outs() << "=== END IR ===\n";
        if (!remainingPrintOpts) return 0;
    }

    llvm::SmallString<128> tempIntermediateFilePath;
    const char* outputFileExtension;
    // Prefer external C compiler for better system compatibility, fallback to embedded Clang.
    std::string ccPath = findExternalCCompiler().value_or(buildParams.argv0);
    bool useExternalCCompiler = buildParams.argv0 == nullptr || ccPath != buildParams.argv0;
    bool isWindows = llvm::sys::path::extension(ccPath) == ".exe";
    bool isMSVC = isWindows; // Assuming MSVC-compatible C compiler.

    switch (backend.getValue()) {
    case Backend::C: {
        CGenerator cGen;
        for (auto* irModule : irGenerator.generatedModules) {
            cGen.codegenModule(*irModule);
        }
        std::string cCode = cGen.finish();

        if (handlePrintOpt(PrintOpt::C)) {
            if (printSectionDividers) llvm::outs() << "=== BEGIN C ===\n";
            llvm::outs() << cCode << "\n";
            if (printSectionDividers) llvm::outs() << "=== END C ===\n";
            if (!remainingPrintOpts) return 0;
        }

        // TODO: emitAssembly not supported, report to user
        outputFileExtension = "c";
        int fileDescriptor;
        if (auto error = llvm::sys::fs::createTemporaryFile("cx", outputFileExtension, fileDescriptor, tempIntermediateFilePath)) {
            ABORT(error.message());
        }

        llvm::raw_fd_ostream file(fileDescriptor, /* shouldClose */ true);
        file << cCode;
        break;
    }
    case Backend::LLVM:
        LLVMGenerator llvmGenerator;
        for (auto* irModule : irGenerator.generatedModules) {
            llvmGenerator.codegenModule(*irModule);
        }
        llvm::Module* llvmModule = llvmGenerator.generatedModules.back();

        if (handlePrintOpt(PrintOpt::LLVM)) {
            llvmModule->setModuleIdentifier("");
            llvmModule->setSourceFileName("");
            if (printSectionDividers) llvm::outs() << "=== BEGIN LLVM ===\n";
            llvmModule->print(llvm::outs(), nullptr);
            if (printSectionDividers) llvm::outs() << "=== END LLVM ===\n";
            if (!remainingPrintOpts) return 0;
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

        outputFileExtension = emitAssembly ? "s" : isWindows ? "obj" : "o";
        if (auto error = llvm::sys::fs::createTemporaryFile("cx", outputFileExtension, tempIntermediateFilePath)) {
            ABORT(error.message());
        }

        auto fileType = emitAssembly ? llvm::CodeGenFileType::AssemblyFile : llvm::CodeGenFileType::ObjectFile;
        auto relocModel = noPIE ? llvm::Reloc::Model::Static : llvm::Reloc::Model::PIC_;
        emitLLVMModuleToMachineCode(linkedModule, tempIntermediateFilePath, fileType, relocModel);
        break;
    }

    if (!buildParams.outputDirectory.empty()) {
        auto error = llvm::sys::fs::create_directories(buildParams.outputDirectory);
        if (error) ABORT(error.message());
    }

    bool treatAsLibrary = mainModule.symbolTable.findInTopLevelScope("main").empty() && !run;
    if (treatAsLibrary && !buildParams.createSharedLib) {
        compileOnly = true;
    }
    if (compileOnly || emitAssembly) {
        llvm::SmallString<128> outputFilePath = buildParams.outputDirectory;
        llvm::sys::path::append(outputFilePath, llvm::Twine("output.") + outputFileExtension);
        renameFile(tempIntermediateFilePath, outputFilePath);
        return 0;
    }

    // Link the output:

    llvm::SmallString<128> tempOutputFilePath;
    llvm::SmallString<128> tempFileNamePattern("cx-%%%%%%%%");
    if (isMSVC) { // MSVC will append .exe to the output file anyway, so match that.
        tempFileNamePattern += (buildParams.createSharedLib ? ".dll" : ".exe");
    }
    llvm::sys::fs::createUniquePath(tempFileNamePattern, tempOutputFilePath, true);

    std::vector<const char*> ccArgs = {
        ccPath.c_str(),
        tempIntermediateFilePath.c_str(),
    };
    if (buildParams.createSharedLib) {
        ccArgs.push_back(isMSVC ? "-LD" : "-shared");
        if (!isMSVC) {
            ccArgs.push_back("-undefined");
            ccArgs.push_back("dynamic_lookup");
        }
    }
    ccArgs.push_back(isMSVC ? "-Fe:" : "-o");
    ccArgs.push_back(tempOutputFilePath.c_str());

    if (backend == Backend::C) {
        // TODO: remove these and fix errors
        ccArgs.push_back("-Wno-incompatible-pointer-types");
        ccArgs.push_back("-Wno-format");
    }

    for (auto& flag : options.cflags) {
        ccArgs.push_back(flag.c_str());
    }
    for (auto& flag : options.defines) {
        ccArgs.push_back("-D");
        ccArgs.push_back(flag.c_str());
    }
    for (auto& flag : librarySearchPaths) {
        ccArgs.push_back("-L");
        ccArgs.push_back(flag.c_str());
    }
    for (auto& flag : libraries) {
        ccArgs.push_back("-l");
        ccArgs.push_back(flag.c_str());
    }
    for (auto& flag : frameworkSearchPaths) {
        ccArgs.push_back("-F");
        ccArgs.push_back(flag.c_str());
    }
    for (auto& flag : frameworks) {
        ccArgs.push_back("-framework");
        ccArgs.push_back(flag.c_str());
    }

    if (isMSVC) {
        ccArgs.push_back("-link");
        ccArgs.push_back("-DEBUG");
        ccArgs.push_back("legacy_stdio_definitions.lib");
        ccArgs.push_back("ucrt.lib");
        ccArgs.push_back("msvcrt.lib");
    }

    std::vector<llvm::StringRef> ccArgStringRefs(ccArgs.begin(), ccArgs.end());
    int ccExitStatus = useExternalCCompiler ? llvm::sys::ExecuteAndWait(ccArgs[0], ccArgStringRefs) : invokeClang(ccArgs);
    llvm::sys::fs::remove(tempIntermediateFilePath);
    if (ccExitStatus != 0) return ccExitStatus;

    if (run) {
        std::string command = (tempOutputFilePath + " 2>&1").str();
        std::string output;
        int executableExitStatus = exec(command.c_str(), output);
        llvm::outs() << output;
        llvm::sys::fs::remove(tempOutputFilePath);

        if (isMSVC) {
            auto path = tempOutputFilePath;
            llvm::sys::path::replace_extension(path, "ilk");
            llvm::sys::fs::remove(path);
            llvm::sys::path::replace_extension(path, "pdb");
            llvm::sys::fs::remove(path);
        }

        return executableExitStatus;
    }

    llvm::SmallString<128> outputPathPrefix = buildParams.outputDirectory;
    if (!outputPathPrefix.empty()) {
        outputPathPrefix.append(llvm::sys::path::get_separator());
    }

    if (buildParams.outputFileName.empty()) {
        if (mainModule.fileBuffers.size() == 1) {
            buildParams.outputFileName = llvm::sys::path::stem(mainModule.fileBuffers.front()->getBufferIdentifier()).str();
        }
        if (buildParams.outputFileName.empty()) {
            buildParams.outputFileName = "main";
        }

        if (buildParams.createSharedLib) {
            buildParams.outputFileName.append(isWindows ? ".dll" : ".so");
        } else {
            buildParams.outputFileName.append(isWindows ? ".exe" : ".out");
        }
    }

    renameFile(tempOutputFilePath, outputPathPrefix + buildParams.outputFileName);

    if (isMSVC) {
        auto path = tempOutputFilePath;
        auto outputPath = outputPathPrefix;
        outputPath += buildParams.outputFileName;

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
        if (manifest.multitarget || manifest.packageName.empty()) {
            outputFileName = llvm::sys::path::filename(targetRootDir);
        } else {
            outputFileName = manifest.packageName;
        }
        auto sourceFiles = getSourceFiles(targetRootDir, manifestPath);
        // TODO: Add support for library packages.
        int exitStatus = buildModuleFromFiles({
            .filePaths = sourceFiles,
            .manifest = &manifest,
            .argv0 = argv0,
            .outputDirectory = manifest.outputDirectory,
            .outputFileName = outputFileName.str(),
        });
        if (exitStatus != 0) return exitStatus;
    }

    return 0;
}

static void addPlatformCompileOptions() {
#ifdef _WIN32
    defines.push_back("Windows");
    cflags.push_back("-fms-extensions");
#endif
#ifdef __APPLE__
    defines.push_back("macOS");
    std::string sdkPath;
    exec("xcrun --show-sdk-path", sdkPath);
    sdkPath = llvm::StringRef(sdkPath).trim();
    if (!sdkPath.empty()) {
        cflags.push_back("-isysroot");
        cflags.push_back(sdkPath);
        // The Clang driver implicitly searches the SDK frameworks directories, so do the same to find framework headers when importing C headers.
        frameworkSearchPaths.push_back(sdkPath + "/System/Library/Frameworks");
    }
#endif
}

int cx::driverMain(int argc, const char** argv) {
    llvm::setBugReportMsg("Please submit a bug report to https://github.com/cx-language/cx/issues and include the crash backtrace.\n");
    llvm::InitLLVM x(argc, argv);
    cl::HideUnrelatedOptions({&stageSelectionCategory, &outputCategory, &dependencyCategory, &diagnosticCategory});
    cl::ParseCommandLineOptions(argc, argv, "C* compiler\n");
    addPlatformCompileOptions();

    if (!inputs.empty()) {
        return buildModuleFromFiles({
            .filePaths = inputs,
            .manifest = nullptr,
            .argv0 = argv[0],
            .outputDirectory = ".",
            .outputFileName = "",
        });
    } else if (build || run) {
        llvm::SmallString<128> currentPath;
        if (auto error = llvm::sys::fs::current_path(currentPath)) {
            ABORT(error.message());
        }
        return buildPackage(currentPath, argv[0]);
    } else {
        cl::PrintHelpMessage(false, true);
        return 0;
    }
}
