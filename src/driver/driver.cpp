#include "driver.h"
#include <bit>
#include <cctype>
#include <cstdio>
#include <cstring>
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
#include "../build/config.h"
#include "../build/dependencies.h"
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

cl::SubCommand build("build", "Build a cx project");
cl::SubCommand run("run", "Build and run a cx executable (program arguments follow '--')");

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
enum class PrintOpt { AST, IR, IRAll, C, LLVM, LLVMAll };
cl::bits<PrintOpt> printOpts(cl::desc("Print output from intermediate steps:"), cl::sub(build), cl::sub(cl::SubCommand::getTopLevel()), cl::cat(outputCategory),
                             cl::values(clEnumValN(PrintOpt::AST, "print-ast", "Print the abstract syntax tree of main module"),
                                        clEnumValN(PrintOpt::IR, "print-ir", "Print cx intermediate representation of main module"),
                                        clEnumValN(PrintOpt::IRAll, "print-ir-all", "Print cx intermediate representation of all compiled modules"),
                                        clEnumValN(PrintOpt::C, "print-c", "Print generated C code"),
                                        clEnumValN(PrintOpt::LLVM, "print-llvm", "Print LLVM intermediate representation of main module"),
                                        clEnumValN(PrintOpt::LLVMAll, "print-llvm-all", "Print LLVM intermediate representation of all compiled modules")));
enum class Backend { LLVM, C };
cl::opt<Backend> backend("backend", cl::desc("Select code-generation backend to use:"), cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory),
                         cl::values(clEnumValN(Backend::LLVM, "llvm", "LLVM backend (default)"), clEnumValN(Backend::C, "c", "C backend")));
cl::opt<BuildMode> buildMode("mode", cl::desc("Select build mode:"), cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory),
                             cl::values(clEnumValN(BuildMode::Debug, "debug", "Debug mode (default): safety checks enabled"),
                                        clEnumValN(BuildMode::ReleaseSafe, "release-safe", "Release-safe mode: safety checks enabled"),
                                        clEnumValN(BuildMode::ReleaseFast, "release-fast", "Release-fast mode: safety checks disabled")),
                             cl::init(BuildMode::Debug));
cl::opt<bool> cDispatch("c-dispatch", cl::desc("Generate goto-free C code using dispatch loops (for C compilers without goto support)"),
                        cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory));
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
cl::opt<bool> warnUndefinedMacros("Wundef", cl::desc("Warn about undefined macros in #if conditions"), cl::sub(cl::SubCommand::getAll()),
                                  cl::cat(diagnosticCategory));
cl::opt<bool> warnUnusedResult("Wunused-result", cl::desc("Warn about unused expression results"), cl::sub(cl::SubCommand::getAll()),
                               cl::cat(diagnosticCategory));
cl::opt<int> errorLimit("error-limit", cl::desc("Limit the number of reported errors (10 by default, 0 removes limit)"), cl::init(10),
                        cl::sub(cl::SubCommand::getAll()), cl::cat(diagnosticCategory));

cl::SubCommand lspSubcommand("lsp", "Start the cx language server (LSP over stdio)");

} // namespace cx

// Arguments after '--' on the command line, passed to the executed program by 'run'.
static std::vector<std::string> programArgs;

// Quotes one program argument for the shell that runs 'cx run' programs (POSIX sh, cmd.exe on Windows).
static std::string shellEscape(llvm::StringRef arg) {
    if (!arg.empty() && llvm::all_of(arg, [](char ch) { return std::isalnum(static_cast<unsigned char>(ch)) || std::strchr("_@%+=:,./-", ch); })) {
        return arg.str();
    }
#ifdef _WIN32
    // cmd.exe passes double-quoted text through (except %var% expansion, defeated by doubling);
    // the C runtime then turns \\ into \ and \" into ". Track cmd.exe's quote-toggle state
    // (\" flips it) so metacharacters outside quotes can still be caret-escaped.
    std::string result = "\"";
    bool quoted = true;
    for (char ch : arg) {
        if (ch == '"') {
            result += "\\\"";
            quoted = !quoted;
        } else {
            if (!quoted && std::strchr("&|<>()^!", ch)) result += '^';
            if (ch == '\\')
                result += "\\\\";
            else if (ch == '%')
                result += "%%";
            else
                result += ch;
        }
    }
    return result + "\"";
#else
    std::string result = "'";
    for (char ch : arg) {
        if (ch == '\'')
            result += "'\\''";
        else
            result += ch;
    }
    return result + "'";
#endif
}

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

    // The standard library root is resolved at runtime (see getCxRootDir) so
    // a compiler built on one machine works when distributed to another.
    if (auto rootDir = getCxRootDir(); !rootDir.empty()) {
        importSearchPaths.push_back(std::move(rootDir));
    }
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

    CompileOptions options = {buildMode, noUnusedWarnings, warnUndefinedMacros, warnUnusedResult, importSearchPaths, frameworkSearchPaths, defines, cflags};
    auto remainingPrintOpts = std::popcount(printOpts.getBits());
    bool printSectionDividers = remainingPrintOpts > 1;

    auto handlePrintOpt = [&](PrintOpt o) {
        bool isSet = printOpts.isSet(o);
        if (isSet) remainingPrintOpts--;
        return isSet;
    };

    // Prints one --print-* section, bracketed with dividers when several
    // sections are requested.
    auto printSection = [&](const char* name, auto printBody) {
        if (printSectionDividers) llvm::outs() << "=== BEGIN " << name << " ===\n";
        printBody();
        if (printSectionDividers) llvm::outs() << "=== END " << name << " ===\n";
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
    typechecker.typecheckModule(mainModule, buildParams.config);
    typechecker.checkUnusedDecls(mainModule);

    if (errors) return 1;

    if (handlePrintOpt(PrintOpt::AST)) {
        printSection("AST", [&] { mainModule.print(llvm::outs()); });
        if (!remainingPrintOpts) return 0;
    }

    IRGenerator irGenerator(options);
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
        printSection("IR", [&] {
            for (auto* module : irGenerator.generatedModules) {
                module->print(llvm::outs());
            }
        });
        if (!remainingPrintOpts) return 0;
    } else if (handlePrintOpt(PrintOpt::IR)) {
        printSection("IR", [&] { irGenerator.generatedModules.back()->print(llvm::outs()); });
        if (!remainingPrintOpts) return 0;
    }

    llvm::SmallString<128> tempIntermediateFilePath;
    const char* outputFileExtension;
    // Prefer external C compiler for better system compatibility, fallback to embedded Clang.
    std::string ccPath = findExternalCCompiler().value_or(buildParams.argv0);
    bool useExternalCCompiler = buildParams.argv0 == nullptr || ccPath != buildParams.argv0;
    bool isWindows = llvm::sys::path::extension(ccPath) == ".exe";
    bool isMSVC = isWindows; // Assuming MSVC-compatible C compiler.

    auto printCSection = [&](const std::string& cCode) {
        if (handlePrintOpt(PrintOpt::C)) {
            printSection("C", [&] { llvm::outs() << cCode << "\n"; });
            return true;
        }
        return false;
    };

    auto printLLVMSections = [&](LLVMGenerator& llvmGenerator) {
        if (handlePrintOpt(PrintOpt::LLVMAll)) {
            handlePrintOpt(PrintOpt::LLVM);
            printSection("LLVM", [&] {
                for (auto* module : llvmGenerator.generatedModules) {
                    module->setModuleIdentifier("");
                    module->setSourceFileName("");
                    module->print(llvm::outs(), nullptr);
                }
            });
            return true;
        }
        if (handlePrintOpt(PrintOpt::LLVM)) {
            printSection("LLVM", [&] {
                auto* llvmModule = llvmGenerator.generatedModules.back();
                llvmModule->setModuleIdentifier("");
                llvmModule->setSourceFileName("");
                llvmModule->print(llvm::outs(), nullptr);
            });
            return true;
        }
        return false;
    };

    switch (backend.getValue()) {
    case Backend::C: {
        CGenerator cGen(cDispatch);
        for (auto* irModule : irGenerator.generatedModules) {
            cGen.codegenModule(*irModule);
        }
        std::string cCode = cGen.finish();

        bool printed = printCSection(cCode);

        if (printOpts.isSet(PrintOpt::LLVM) || printOpts.isSet(PrintOpt::LLVMAll)) {
            LLVMGenerator printLLVMGenerator;
            for (auto* irModule : irGenerator.generatedModules) {
                printLLVMGenerator.codegenModule(*irModule);
            }
            printed = printLLVMSections(printLLVMGenerator) || printed;
        }

        if (printed && !remainingPrintOpts) return 0;

        if (emitAssembly) ABORT("--emit-assembly is not supported with the C backend");
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

        bool printed = false;

        if (printOpts.isSet(PrintOpt::C)) {
            CGenerator printCGen(cDispatch);
            for (auto* irModule : irGenerator.generatedModules) {
                printCGen.codegenModule(*irModule);
            }
            printed = printCSection(printCGen.finish());
        }

        printed = printLLVMSections(llvmGenerator) || printed;

        if (printed && !remainingPrintOpts) return 0;

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

    if (backend == Backend::C && !isMSVC) {
        // TODO: remove these and fix errors
        // Note: GCC/Clang-only flags, MSVC rejects unknown -W options.
        ccArgs.push_back("-Wno-incompatible-pointer-types");
        ccArgs.push_back("-Wno-format");
    }

    for (auto& flag : options.cflags) {
        ccArgs.push_back(flag.c_str());
    }
    auto addFlaggedArgs = [&](const char* flag, const auto& values) {
        for (const auto& value : values) {
            ccArgs.push_back(flag);
            ccArgs.push_back(value.c_str());
        }
    };
    // The generated C code includes the same C headers that were imported,
    // so pass the import search paths to the C compiler as well.
    addFlaggedArgs("-I", options.importSearchPaths);
    addFlaggedArgs("-D", options.defines);
    addFlaggedArgs("-L", librarySearchPaths);
    addFlaggedArgs("-l", libraries);
    addFlaggedArgs("-F", frameworkSearchPaths);
    addFlaggedArgs("-framework", frameworks);
    if (!isMSVC) {
        // The standard library uses the C math library.
        ccArgs.push_back("-lm");
    }

    if (isMSVC) {
        ccArgs.push_back("-link");
        ccArgs.push_back("-DEBUG");
        ccArgs.push_back("legacy_stdio_definitions.lib");
        ccArgs.push_back("ucrt.lib");
        ccArgs.push_back("msvcrt.lib");
    }

    std::vector<llvm::StringRef> ccArgStringRefs(ccArgs.begin(), ccArgs.end());

    // When running the program right away, capture the C compiler's stdout so
    // chatty compilers (MSVC cl.exe echoes banners and compiled filenames)
    // don't pollute the executed program's stdout. Stderr stays visible, and
    // the captured output is shown if compilation fails.
    llvm::SmallString<128> ccStdoutLog;
    bool captureCcOutput = run && useExternalCCompiler && !llvm::sys::fs::createTemporaryFile("cx-cc-stdout", "log", ccStdoutLog);
    std::vector<std::optional<llvm::StringRef>> ccRedirects;
    if (captureCcOutput) {
        ccRedirects = {std::nullopt, ccStdoutLog.str(), std::nullopt};
    }
    int ccExitStatus = useExternalCCompiler ? llvm::sys::ExecuteAndWait(ccArgs[0], ccArgStringRefs, std::nullopt, ccRedirects) : invokeClang(ccArgs);
    llvm::sys::fs::remove(tempIntermediateFilePath);
    if (ccExitStatus != 0) {
        if (captureCcOutput) {
            if (auto output = llvm::MemoryBuffer::getFile(ccStdoutLog)) {
                llvm::errs() << (*output)->getBuffer();
            }
            llvm::sys::fs::remove(ccStdoutLog);
        }
        return ccExitStatus;
    }
    if (captureCcOutput) {
        llvm::sys::fs::remove(ccStdoutLog);
    }

    if (run) {
        std::string command = tempOutputFilePath.str().str();
        for (const auto& arg : programArgs) {
            command += " " + shellEscape(arg);
        }
        command += " 2>&1";
        std::string output;
        int executableExitStatus = exec(command.c_str(), output);
        llvm::outs() << output;
        llvm::sys::fs::remove(tempOutputFilePath);

        if (isMSVC) {
            for (llvm::StringRef extension : {"ilk", "pdb"}) {
                auto path = tempOutputFilePath;
                llvm::sys::path::replace_extension(path, extension);
                llvm::sys::fs::remove(path);
            }
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

    // An absolute -o path is used as is; otherwise the output directory is prepended.
    llvm::SmallString<128> outputPath;
    if (!llvm::sys::path::is_absolute(buildParams.outputFileName)) {
        outputPath = outputPathPrefix;
    }
    outputPath += buildParams.outputFileName;

    renameFile(tempOutputFilePath, outputPath);

    if (isMSVC) {
        for (llvm::StringRef extension : {"ilk", "pdb"}) {
            auto path = tempOutputFilePath;
            llvm::sys::path::replace_extension(path, extension);
            llvm::sys::path::replace_extension(outputPath, extension);
            renameFile(path, outputPath);
        }
    }

    return 0;
}

static void addPkgConfigFlags(llvm::ArrayRef<std::string> packages) {
    if (packages.empty()) return;

    auto pkgConfig = llvm::sys::findProgramByName("pkg-config");
    if (!pkgConfig) {
        ABORT("couldn't find 'pkg-config'");
    }

    std::string command = *pkgConfig + " --cflags --libs";
    for (llvm::StringRef package : packages) {
        command += " ";
        command += package;
    }

    std::string output;
    if (exec(command.c_str(), output) != 0) {
        ABORT("'" << command << "' failed");
    }

    llvm::SmallVector<llvm::StringRef, 16> tokens;
    llvm::StringRef(output).trim().split(tokens, ' ', -1, false);

    for (size_t i = 0; i < tokens.size(); ++i) {
        llvm::StringRef token = tokens[i];
        if (token.starts_with("-D")) {
            defines.push_back(token.drop_front(2).str());
        } else if (token.starts_with("-I")) {
            importSearchPaths.push_back(token.drop_front(2).str());
        } else if (token.starts_with("-L")) {
            librarySearchPaths.push_back(token.drop_front(2).str());
        } else if (token.starts_with("-l")) {
            libraries.push_back(token.drop_front(2).str());
        } else if (token.starts_with("-F")) {
            frameworkSearchPaths.push_back(token.drop_front(2).str());
        } else if (token == "-framework" && i + 1 < tokens.size()) {
            frameworks.push_back(tokens[++i].str());
        } else {
            cflags.push_back(token.str());
        }
    }
}

static void addConfigBuildFlags(const BuildConfig& config) {
    for (auto& define : config.defines) {
        defines.push_back(define);
    }
    for (auto& library : config.libraries) {
        libraries.push_back(library);
    }
    for (auto& framework : config.frameworks) {
        frameworks.push_back(framework);
    }
    addPkgConfigFlags(config.pkgConfigDependencies);
}

static int buildDirectory(llvm::StringRef directory, const char* argv0) {
    BuildConfig config(directory.str(), {defines.begin(), defines.end()});
    fetchDependencies(config);
    addConfigBuildFlags(config);

    for (auto& targetRootDir : config.getTargetRootDirectories()) {
        llvm::StringRef outputFileName;
        if (config.multitarget || config.name.empty()) {
            outputFileName = llvm::sys::path::filename(targetRootDir);
        } else {
            outputFileName = config.name;
        }
        auto sourceFiles = getSourceFiles(targetRootDir);
        // TODO: Add support for library packages.
        int exitStatus = buildModuleFromFiles({
            .filePaths = sourceFiles,
            .config = &config,
            .argv0 = argv0,
            .outputDirectory = config.outputDirectory,
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
    llvm::setBugReportMsg("Please submit a bug report to https://github.com/emillaine/cx/issues and include the crash backtrace.\n");
    llvm::InitLLVM x(argc, argv);
    // Like cargo and npm, everything after '--' is passed to the executed program by 'run'.
    // Split it off before option parsing so it is never mistaken for input files or flags.
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--") == 0) {
            programArgs.assign(argv + i + 1, argv + argc);
            argc = i;
            break;
        }
    }
    cl::HideUnrelatedOptions({&stageSelectionCategory, &outputCategory, &dependencyCategory, &diagnosticCategory});
    cl::ParseCommandLineOptions(argc, argv, "cx compiler\n");
    if (!programArgs.empty() && !run) {
        ABORT("program arguments require the 'run' subcommand");
    }
    addPlatformCompileOptions();

    diagnosticOptions.disableWarnings = disableWarnings;
    diagnosticOptions.warningsAsErrors = warningsAsErrors;
    diagnosticOptions.errorLimit = errorLimit;

    if (!inputs.empty()) {
        return buildModuleFromFiles({
            .filePaths = inputs,
            .config = nullptr,
            .argv0 = argv[0],
            .outputDirectory = ".",
            .outputFileName = "",
        });
    } else if (build || run) {
        // Build the current directory by relative path so diagnostics show
        // relative paths.
        return buildDirectory(".", argv[0]);
    } else if (lspSubcommand) {
        // The server lives in the cx-lsp binary so that every compilation it
        // triggers runs in a fresh process (see src/lsp/). Forward stdio.
        std::string lspExecutable;
        {
            // Probe both spellings: the binary is cx-lsp.exe on Windows,
            // where the extensionless probe below would miss it.
            for (llvm::StringRef suffix : {"", ".exe"}) {
                llvm::SmallString<128> candidate(argv[0]);
                llvm::sys::path::remove_filename(candidate);
                llvm::sys::path::append(candidate, "cx-lsp");
                candidate += suffix;
                if (llvm::sys::fs::can_execute(candidate)) {
                    lspExecutable = candidate.str().str();
                    break;
                }
            }
            if (lspExecutable.empty()) {
                if (auto path = llvm::sys::findProgramByName("cx-lsp")) {
                    lspExecutable = *path;
                } else {
                    ABORT("couldn't find the 'cx-lsp' language server binary");
                }
            }
        }
        std::string errorMessage;
        bool executionFailed = false;
        int status = llvm::sys::ExecuteAndWait(lspExecutable, {lspExecutable}, std::nullopt, {}, 0, 0, &errorMessage, &executionFailed);
        if (executionFailed) ABORT("couldn't start the language server: " << errorMessage);
        return status;
    } else {
        cl::PrintHelpMessage(false, true);
        return 0;
    }
}
