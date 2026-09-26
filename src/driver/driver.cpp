#include "driver.h"
#include <bit>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <memory>
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
#include <llvm/Passes/PassBuilder.h>
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
#include "../ast/demangle.h"
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
#include "jit.h"

using namespace cx;
namespace cl = llvm::cl;

namespace cx {

cl::SubCommand build("build", "Build a cx project");
cl::SubCommand run("run", "Build and run a cx executable (program arguments follow '--')");
cl::SubCommand testSubcommand("test", "Build and run the unit tests in a cx project");
cl::SubCommand demangleSubcommand("demangle", "Demangle cx symbols (arguments, or stdin when empty; use as a filter for stack traces)");

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
BuildMode buildMode = BuildMode::Debug;
cl::opt<bool> releaseMode("release", cl::desc("Release mode: optimized, safety checks disabled"), cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory));
cl::opt<bool> releaseSafeMode("release-safe", cl::desc("Release-safe mode: optimized, safety checks enabled"), cl::sub(cl::SubCommand::getAll()),
                              cl::cat(outputCategory));
cl::opt<bool> cDispatch("c-dispatch", cl::desc("Generate goto-free C code using dispatch loops (for C compilers without goto support)"),
                        cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory));
cl::opt<bool> emitAssembly("emit-assembly", cl::desc("Emit assembly code"), cl::cat(outputCategory));
cl::alias emitAssemblyAlias("S", cl::aliasopt(emitAssembly), cl::cat(outputCategory));
cl::opt<bool> emitBitcode("emit-llvm-bitcode", cl::desc("Emit LLVM bitcode"), cl::cat(outputCategory));
cl::opt<bool> noPIE("no-pie", cl::desc("Don't produce a position-independent executable"), cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory));
cl::opt<bool> noJit("no-jit", cl::desc("Don't run in-process via JIT; link and execute a binary instead (named stack traces)"),
                    cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory));
cl::opt<bool> noLeakCheck("no-leak-check", cl::desc("Disable the leak detector in debug builds"), cl::sub(cl::SubCommand::getAll()), cl::cat(outputCategory));
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
cl::opt<bool> checkAll("check-all", cl::desc("Typecheck all code in imported modules, not just used code"), cl::sub(cl::SubCommand::getAll()),
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

static void addHeaderSearchPathsFromEnvVar(const char* name, std::vector<std::string>& paths) {
    if (auto pathList = llvm::sys::Process::GetEnv(name)) {
        llvm::SmallVector<llvm::StringRef, 16> splitPaths;
        llvm::StringRef(*pathList).split(splitPaths, llvm::sys::EnvPathSeparator, -1, false);

        for (llvm::StringRef path : splitPaths) {
            paths.push_back(path.str());
        }
    }
}

// Standard search paths shared by every package: the caller adds its own
// source directories and package settings on top.
static void appendSystemImportSearchPaths(std::vector<std::string>& paths) {
    // The standard library root is resolved at runtime (see getCxRootDir) so
    // a compiler built on one machine works when distributed to another.
    if (auto rootDir = getCxRootDir(); !rootDir.empty()) {
        paths.push_back(std::move(rootDir));
    }
    paths.push_back(CLANG_BUILTIN_INCLUDE_PATH);
    paths.push_back("/usr/include");
    paths.push_back("/usr/local/include");
    addHeaderSearchPathsFromEnvVar("CPATH", paths);
    addHeaderSearchPathsFromEnvVar("C_INCLUDE_PATH", paths);
    addHeaderSearchPathsFromEnvVar("INCLUDE", paths);
    // Compiler-reported header paths are queried lazily on first C import
    // (see getCCompilerSearchPaths): most builds never import C headers.
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
        // Vendored packages are imported by name, so each source directory's
        // vendor/ subdirectory is a package container: `import foo` finds vendor/foo.
        importSearchPaths.push_back((keyValue.getKey() + "/vendor").str());
    }

    appendSystemImportSearchPaths(importSearchPaths);
}

static llvm::TargetMachine* createTargetMachine(llvm::Module& module, llvm::Reloc::Model relocModel, BuildMode mode) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
    module.setTargetTriple(triple);

    std::string errorMessage;
    auto* target = llvm::TargetRegistry::lookupTarget(triple, errorMessage);
    if (!target) ABORT(errorMessage);

    llvm::TargetOptions options;
#ifdef __APPLE__
    // Tune debug info for LLDB so object files carry the STABS entries
    // dsymutil needs to collect DWARF into .dSYM bundles.
    options.DebuggerTuning = llvm::DebuggerKind::LLDB;
#endif
    auto optLevel = mode == BuildMode::Debug ? llvm::CodeGenOptLevel::Default : llvm::CodeGenOptLevel::Aggressive;
    auto* targetMachine = target->createTargetMachine(triple, "generic", "", options, relocModel, std::nullopt, optLevel);
    module.setDataLayout(targetMachine->createDataLayout());
    return targetMachine;
}

static void optimizeLLVMModule(llvm::Module& module, BuildMode mode, llvm::TargetMachine* targetMachine) {
    if (mode == BuildMode::Debug) return;

    llvm::LoopAnalysisManager loopAnalyses;
    llvm::FunctionAnalysisManager functionAnalyses;
    llvm::CGSCCAnalysisManager cgsccAnalyses;
    llvm::ModuleAnalysisManager moduleAnalyses;
    llvm::PassBuilder passBuilder(targetMachine);
    passBuilder.registerModuleAnalyses(moduleAnalyses);
    passBuilder.registerCGSCCAnalyses(cgsccAnalyses);
    passBuilder.registerFunctionAnalyses(functionAnalyses);
    passBuilder.registerLoopAnalyses(loopAnalyses);
    passBuilder.crossRegisterProxies(loopAnalyses, functionAnalyses, cgsccAnalyses, moduleAnalyses);
    llvm::ModulePassManager passManager = passBuilder.buildPerModuleDefaultPipeline(llvm::OptimizationLevel::O3);
    passManager.run(module, moduleAnalyses);
}

static void emitLLVMModuleToMachineCode(llvm::Module& module, llvm::TargetMachine& targetMachine, llvm::StringRef fileName, llvm::CodeGenFileType fileType) {
    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::OF_None);
    if (error) ABORT("couldn't open file '" << fileName << "': " << error.message());

    llvm::legacy::PassManager passManager;
    if (targetMachine.addPassesToEmitFile(passManager, file, nullptr, fileType)) {
        ABORT("TargetMachine can't emit a file of this type");
    }

    passManager.run(module);
    file.flush();
}

static bool isLibraryFilePath(llvm::StringRef value) {
    if (!llvm::sys::fs::exists(value)) return false;
    if (value.contains('/') || value.contains('\\')) return true;
    auto extension = llvm::sys::path::extension(value);
    return extension == ".a" || extension == ".so" || extension == ".dylib" || extension == ".lib" || extension == ".dll" || extension == ".o"
        || extension == ".obj";
}

static void emitLLVMBitcode(const llvm::Module& module, llvm::StringRef fileName) {
    std::error_code error;
    llvm::raw_fd_ostream file(fileName, error, llvm::sys::fs::OF_None);
    if (error) ABORT("couldn't open file '" << fileName << "': " << error.message());
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

// Collects top-level test functions and synthesizes a main that runs them.
// Returns false after reporting an error.
static bool synthesizeTestMain(Module& mainModule) {
    std::vector<FunctionDecl*> tests;
    for (auto& sourceFile : mainModule.sourceFiles) {
        for (Decl* decl : sourceFile.topLevelDecls) {
            FunctionDecl* functionDecl = nullptr;
            if (auto* functionTemplate = llvm::dyn_cast<FunctionTemplate>(decl)) {
                if (!functionTemplate->functionDecl->isTest) continue;
                REPORT_ERROR(decl->getLocation(), "generic test functions are not supported");
                return false;
            } else if (auto* function = llvm::dyn_cast<FunctionDecl>(decl)) {
                if (!function->isTest) continue;
                functionDecl = function;
            } else {
                continue;
            }
            if (!functionDecl->getParams().empty() || !functionDecl->getReturnType().isVoid()) {
                REPORT_ERROR(functionDecl->getLocation(), "test function '" << functionDecl->getName() << "' must take no parameters and return void");
                return false;
            }
            if (functionDecl->accessLevel == AccessLevel::Private) {
                REPORT_ERROR(functionDecl->getLocation(), "test function '" << functionDecl->getName() << "' cannot be private");
                return false;
            }
            tests.push_back(functionDecl);
        }
    }

    if (tests.empty()) {
        llvm::outs() << "no tests found\n";
    }

    // The synthesized main takes the entry point; a user-defined main is kept
    // as an ordinary (uncalled) function so test files can live next to it.
    for (Decl* decl : mainModule.symbolTable.findInTopLevelScope("main")) {
        if (auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl)) {
            functionDecl->proto.name = internString("__cx_test_user_main");
            functionDecl->referenced = true;
        }
    }

    FunctionProto proto("main", {}, Type::getVoid());
    auto* mainDecl = makeAST<FunctionDecl>(std::move(proto), std::vector<GenericArg>(), AccessLevel::Default, mainModule,
                                           tests.empty() ? Location() : tests.front()->getLocation());
    std::vector<Stmt*> body;
    for (FunctionDecl* test : tests) {
        body.push_back(makeAST<ExprStmt>(makeAST<CallExpr>(makeAST<VarExpr>(test->getName(), test->getLocation()), std::vector<NamedValue>(),
                                                           std::vector<GenericArg>(), test->getLocation())));
        std::vector<NamedValue> printArgs;
        printArgs.emplace_back(makeAST<StringLiteralExpr>(std::string("ok "), test->getLocation()));
        printArgs.emplace_back(makeAST<StringLiteralExpr>(std::string(test->getName()), test->getLocation()));
        body.push_back(makeAST<ExprStmt>(
            makeAST<CallExpr>(makeAST<VarExpr>("println", test->getLocation()), std::move(printArgs), std::vector<GenericArg>(), test->getLocation())));
    }
    mainDecl->body = std::move(body);
    mainModule.sourceFiles.front().topLevelDecls.push_back(mainDecl);
    // Added directly to skip the duplicate check: a renamed user main may
    // still be keyed under "main", but it no longer matches by prototype.
    mainModule.symbolTable.addGlobal("main", mainDecl);
    return true;
}

int cx::buildModule(Module& mainModule, BuildParams buildParams) {
    PhaseTimer totalTimer("buildModule-total");
    if (mainModule.fileBuffers.empty()) {
        ABORT("no input files");
    }

    {
        PhaseTimer timer("search-paths");
        addPredefinedImportSearchPaths(buildParams.filePaths);
    }

    CompileOptions options = {
        buildMode, noUnusedWarnings, checkAll, warnUndefinedMacros, warnUnusedResult, noLeakCheck, importSearchPaths, frameworkSearchPaths, defines, cflags};
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

    {
        PhaseTimer timer("parse");
        for (auto& fileBuffer : mainModule.fileBuffers) {
            Parser parser(*fileBuffer, mainModule, options);
            parser.parse();
        }
    }

    if (parse) return errors ? 1 : 0;

    if (buildParams.runTests && !synthesizeTestMain(mainModule)) return 1;

    Typechecker typechecker(options, buildParams.config ? &buildParams.config->resolvedDependencies : nullptr);
    {
        PhaseTimer timer("typecheck-imports");
        for (auto& importedModule : mainModule.getImportedModules()) {
            typechecker.typecheckModule(*importedModule, options, false);
        }
    }
    {
        PhaseTimer timer("typecheck-main");
        typechecker.typecheckModule(mainModule, options, true);
        typechecker.checkUnusedDecls(mainModule);
    }

    if (errors) return 1;

    if (handlePrintOpt(PrintOpt::AST)) {
        printSection("AST", [&] { mainModule.print(llvm::outs()); });
        if (!remainingPrintOpts) return 0;
    }

    IRGenerator irGenerator(options);
    {
        PhaseTimer timer("irgen");
        for (auto* importedModule : Module::getAllImportedModules()) {
            irGenerator.emitModule(*importedModule);
        }
        irGenerator.emitModule(mainModule);
    }

    NullAnalyzer nullAnalyzer;
    {
        PhaseTimer timer("null-analyzer");
        for (auto module : irGenerator.generatedModules) {
            nullAnalyzer.analyze(module);
        }
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
            printLLVMGenerator.emitDebugInfo = options.mode == BuildMode::Debug;
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
            ABORT("couldn't create temporary file: " << error.message());
        }

        llvm::raw_fd_ostream file(fileDescriptor, /* shouldClose */ true);
        file << cCode;
        break;
    }
    case Backend::LLVM: {
        LLVMGenerator llvmGenerator;
        llvmGenerator.emitDebugInfo = options.mode == BuildMode::Debug;
        {
            PhaseTimer timer("llvm-codegen");
            for (auto* irModule : irGenerator.generatedModules) {
                llvmGenerator.codegenModule(*irModule);
            }
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

        auto linkedModule = std::make_unique<llvm::Module>("", llvmGenerator.ctx);

        {
            // Scoped: the linker holds refs into the linked module, which the JIT path destroys.
            llvm::Linker linker(*linkedModule);
            PhaseTimer timer("llvm-link");
            for (auto& module : llvmGenerator.generatedModules) {
                bool error = linker.linkInModule(std::unique_ptr<llvm::Module>(module));
                if (error) ABORT("LLVM module linking failed");
            }
        }

        auto relocModel = noPIE ? llvm::Reloc::Model::Static : llvm::Reloc::Model::PIC_;
        llvm::TargetMachine* targetMachine;
        {
            PhaseTimer timer("llvm-opt");
            targetMachine = createTargetMachine(*linkedModule, relocModel, options.mode);
            optimizeLLVMModule(*linkedModule, options.mode, targetMachine);
        }

        if (emitBitcode) {
            emitLLVMBitcode(*linkedModule, "output.bc");
            return 0;
        }

        // JIT runs the program in-process, skipping object emission, the C compiler link,
        // and (on macOS) first-execution signature validation of a fresh binary.
        // Search paths without libraries are inert (macOS always adds framework search paths), so only -l/-framework/--no-jit decline JIT.
        if ((run || testSubcommand) && !compileOnly && !emitAssembly && !noJit && libraries.empty() && frameworks.empty() && jitEligible(*linkedModule)) {
            PhaseTimer timer("jit-run");
            std::string argv0 = buildParams.filePaths.empty() ? "main" : std::string(buildParams.filePaths.front());
            return jitRun(std::move(linkedModule), llvmGenerator.takeContext(), argv0, programArgs);
        }

        outputFileExtension = emitAssembly ? "s" : isWindows ? "obj" : "o";
        if (auto error = llvm::sys::fs::createTemporaryFile("cx", outputFileExtension, tempIntermediateFilePath)) {
            ABORT("couldn't create temporary file: " << error.message());
        }

        auto fileType = emitAssembly ? llvm::CodeGenFileType::AssemblyFile : llvm::CodeGenFileType::ObjectFile;
        {
            PhaseTimer timer("emit-obj");
            emitLLVMModuleToMachineCode(*linkedModule, *targetMachine, tempIntermediateFilePath, fileType);
        }
    } break;
    }

    // Skip existing directories: creating "." unconditionally fails on Windows
    // in non-writable current directories (e.g. C:\) even though there is
    // nothing to create.
    if (!buildParams.outputDirectory.empty() && !llvm::sys::fs::is_directory(buildParams.outputDirectory)) {
        if (auto error = llvm::sys::fs::create_directories(buildParams.outputDirectory)) {
            ABORT("couldn't create output directory '" << buildParams.outputDirectory << "': " << error.message());
        }
    }

    bool treatAsLibrary = mainModule.symbolTable.findInTopLevelScope("main").empty() && !run;
    if (treatAsLibrary && !buildParams.createSharedLib) {
        compileOnly = true;
    }
    if (compileOnly || emitAssembly) {
        std::string fileName = buildParams.outputFileName;
        if (fileName.empty()) {
            if (mainModule.fileBuffers.size() == 1) {
                fileName = llvm::sys::path::stem(mainModule.fileBuffers.front()->getBufferIdentifier()).str();
            }
            if (fileName.empty()) fileName = "output";
            fileName += ".";
            fileName += outputFileExtension;
        }
        // An absolute -o path is used as is; otherwise the output directory is prepended.
        llvm::SmallString<128> outputFilePath;
        if (llvm::sys::path::is_absolute(fileName)) {
            outputFilePath = fileName;
        } else {
            outputFilePath = buildParams.outputDirectory;
            llvm::sys::path::append(outputFilePath, fileName);
        }
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

    if (backend == Backend::C && options.mode != BuildMode::Debug) {
        // External MSVC-compatible compilers (cl, clang-cl) take /O2. The
        // embedded Clang driver runs in GNU mode, so it takes -O3.
        ccArgs.push_back(isMSVC && useExternalCCompiler ? "/O2" : "-O3");
    }

    for (auto& flag : options.cflags) {
        // The C importer (Clang) needs MSVC extensions to parse system headers on Windows,
        // but cl itself rejects the Clang-only flag with a D9002 warning, so don't pass it on.
        if (isMSVC && flag == "-fms-extensions") continue;
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
    for (const auto& library : libraries) {
        // A -l value naming an existing library file is passed to the linker as
        // an input file; -l with a path is otherwise rejected ("library ... not found").
        if (isLibraryFilePath(library)) {
            ccArgs.push_back(library.c_str());
        } else {
            ccArgs.push_back("-l");
            ccArgs.push_back(library.c_str());
        }
    }
    addFlaggedArgs("-F", frameworkSearchPaths);
    addFlaggedArgs("-framework", frameworks);
    if (!isMSVC) {
        // The standard library uses the C math library.
        ccArgs.push_back("-lm");
        // Debug info is Debug-only; release stack traces resolve names
        // through the symbol table instead.
        if (options.mode == BuildMode::Debug) ccArgs.push_back("-g");
#ifndef __APPLE__
        // Export symbols so backtrace() resolves cx function names (macOS
        // resolves them from the static symbol table instead).
        ccArgs.push_back("-rdynamic");
#endif
    }

    if (isMSVC) {
        ccArgs.push_back("-link");
        ccArgs.push_back("-DEBUG");
        // The default 1MB stack overflows on deeply recursive programs that
        // run fine elsewhere (e.g. JSON parsing); reserve 8MB to match the
        // Unix default. Reservation costs only address space until used.
        ccArgs.push_back("/STACK:8388608");
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
    bool captureCcOutput = (run || testSubcommand) && useExternalCCompiler && !llvm::sys::fs::createTemporaryFile("cx-cc-stdout", "log", ccStdoutLog);
    std::vector<std::optional<llvm::StringRef>> ccRedirects;
    if (captureCcOutput) {
        ccRedirects = {std::nullopt, ccStdoutLog.str(), std::nullopt};
    }
    int ccExitStatus;
    {
        PhaseTimer timer("cc-link");
        ccExitStatus = useExternalCCompiler ? llvm::sys::ExecuteAndWait(ccArgs[0], ccArgStringRefs, std::nullopt, ccRedirects) : invokeClang(ccArgs);
    }
    if (ccExitStatus != 0) {
        llvm::sys::fs::remove(tempIntermediateFilePath);
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

    if (run || testSubcommand) {
        std::string command = tempOutputFilePath.str().str();
        for (const auto& arg : programArgs) {
            command += " " + shellEscape(arg);
        }
        command += " 2>&1";
        std::string output;
        int executableExitStatus = exec(command.c_str(), output);
        llvm::outs() << output;
        llvm::sys::fs::remove(tempIntermediateFilePath);
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

#ifdef __APPLE__
    // Collect DWARF from the object files into a .dSYM bundle so debuggers
    // show cx functions with file and line info (the linker leaves it behind).
    // Debug-only: release builds emit no DWARF, so there is nothing to collect
    // (and a failed dsymutil would warn spuriously).
    if (!buildParams.createSharedLib && options.mode == BuildMode::Debug) {
        PhaseTimer timer("dsymutil");
        std::string dsymutilCommand = "xcrun dsymutil " + shellEscape(outputPath.str()) + " 2>/dev/null";
        std::string dsymutilOutput;
        if (exec(dsymutilCommand.c_str(), dsymutilOutput) != 0) {
            llvm::errs() << "warning: couldn't collect debug info ('" << dsymutilCommand << "' failed)\n";
        }
    }
#endif
    llvm::sys::fs::remove(tempIntermediateFilePath);

    return 0;
}

static PkgConfigSplit queryPkgConfigFlagsOrAbort(llvm::ArrayRef<std::string> packages) {
    if (auto split = queryPkgConfigFlags(packages)) return std::move(*split);
    std::string joined;
    for (auto& package : packages) {
        if (!joined.empty()) joined += " ";
        joined += package;
    }
    ABORT("couldn't query pkg-config for '" << joined << "'");
}

static void addPkgConfigFlags(llvm::ArrayRef<std::string> packages) {
    auto split = queryPkgConfigFlagsOrAbort(packages);
    for (auto& define : split.defines) {
        defines.push_back(define);
    }
    for (auto& path : split.headerSearchPaths) {
        importSearchPaths.push_back(path);
    }
    for (auto& path : split.librarySearchPaths) {
        librarySearchPaths.push_back(path);
    }
    for (auto& library : split.libraries) {
        libraries.push_back(library);
    }
    for (auto& path : split.frameworkSearchPaths) {
        frameworkSearchPaths.push_back(path);
    }
    for (auto& framework : split.frameworks) {
        frameworks.push_back(framework);
    }
    for (auto& flag : split.cflags) {
        cflags.push_back(flag);
    }
}

static void addConfigBuildFlags(const BuildConfig& config) {
    for (auto& define : config.defines) {
        defines.push_back(define);
    }
    // The project root's vendor/ holds importable packages (multitarget builds
    // compile sources under src/, whose parents never include the root).
    importSearchPaths.push_back((llvm::StringRef(config.rootDirectory) + "/vendor").str());
    for (auto& path : config.headerSearchPaths) {
        importSearchPaths.push_back(absolutizePackagePath(config.rootDirectory, path));
    }
    for (auto& path : config.librarySearchPaths) {
        librarySearchPaths.push_back(absolutizePackagePath(config.rootDirectory, path));
    }
    for (auto& library : config.libraries) {
        libraries.push_back(absolutizeLibraryPath(config.rootDirectory, library));
    }
    for (auto& framework : config.frameworks) {
        frameworks.push_back(framework);
    }
    addPkgConfigFlags(config.pkgConfigDependencies);
}

static std::string getDefaultOutputFileName(llvm::StringRef targetRootDir) {
    llvm::StringRef filename = llvm::sys::path::filename(targetRootDir);
    if (!filename.empty() && filename != "." && filename != "..") return filename.str();
    llvm::SmallString<128> currentPath;
    if (auto error = llvm::sys::fs::current_path(currentPath)) return "main";
    filename = llvm::sys::path::filename(currentPath);
    if (filename.empty()) return "main";
    return filename.str();
}

static int buildDirectory(llvm::StringRef directory, const char* argv0, bool runTests = false) {
    BuildConfig config(directory.str(), {defines.begin(), defines.end()});

    // Snapshot invocation flags before project settings merge: dependencies
    // compose their options from these, isolated from the main project.
    CompileOptions baseOptions;
    baseOptions.mode = buildMode;
    baseOptions.noUnusedWarnings = noUnusedWarnings;
    baseOptions.checkAll = checkAll;
    baseOptions.warnUndefinedMacros = warnUndefinedMacros;
    baseOptions.warnUnusedResult = warnUnusedResult;
    baseOptions.noLeakCheck = noLeakCheck;
    baseOptions.importSearchPaths = importSearchPaths;
    baseOptions.frameworkSearchPaths = frameworkSearchPaths;
    baseOptions.defines = defines;
    baseOptions.cflags = cflags;
    appendSystemImportSearchPaths(baseOptions.importSearchPaths);

    resolveDependencyClosure(config, baseOptions, /*fetchMissing=*/true);

    // Route each dependency's pkg-config output and union its link
    // contributions. Defines, search paths, and cflags stay package-scoped in
    // the closure records; there is one binary, so linking is global.
    for (auto& record : config.resolvedDependencies) {
        auto split = queryPkgConfigFlagsOrAbort(record.pkgConfigDependencies);
        for (auto& define : split.defines) {
            record.options.defines.push_back(define);
        }
        for (auto& path : split.headerSearchPaths) {
            record.options.importSearchPaths.push_back(absolutizePackagePath(record.rootDirectory, path));
        }
        for (auto& path : split.frameworkSearchPaths) {
            record.options.frameworkSearchPaths.push_back(absolutizePackagePath(record.rootDirectory, path));
            record.frameworkSearchPaths.push_back(absolutizePackagePath(record.rootDirectory, path));
        }
        for (auto& flag : split.cflags) {
            record.options.cflags.push_back(flag);
        }
        for (auto& path : split.librarySearchPaths) {
            librarySearchPaths.push_back(absolutizePackagePath(record.rootDirectory, path));
        }
        for (auto& library : split.libraries) {
            libraries.push_back(library);
        }
        for (auto& framework : split.frameworks) {
            frameworks.push_back(framework);
        }
        for (auto& path : record.librarySearchPaths) {
            librarySearchPaths.push_back(path);
        }
        for (auto& library : record.libraries) {
            libraries.push_back(library);
        }
        for (auto& framework : record.frameworks) {
            frameworks.push_back(framework);
        }
        for (auto& path : record.frameworkSearchPaths) {
            frameworkSearchPaths.push_back(path);
        }
    }

    addConfigBuildFlags(config);

    for (auto& targetRootDir : config.getTargetRootDirectories()) {
        std::string outputFileName;
        if (config.multitarget || config.name.empty()) {
            outputFileName = getDefaultOutputFileName(targetRootDir);
        } else {
            outputFileName = config.name;
        }
        auto sourceFiles = getSourceFiles(targetRootDir, config.rootDirectory);
        // TODO: Add support for library packages.
        int exitStatus = buildModuleFromFiles({
            .filePaths = sourceFiles,
            .config = &config,
            .argv0 = argv0,
            .outputDirectory = config.outputDirectory,
            .outputFileName = outputFileName,
            .runTests = runTests,
        });
        if (exitStatus != 0) return exitStatus;
    }

    return 0;
}

static int runDemangle() {
    if (!inputs.empty()) {
        for (auto& input : inputs) {
            llvm::outs() << demangleLine(input) << "\n";
        }
        return 0;
    }
    std::string line;
    while (std::getline(std::cin, line)) {
        llvm::outs() << demangleLine(line) << "\n";
    }
    return 0;
}

static void addPlatformCompileOptions() {
    if (buildMode == BuildMode::Debug) {
        defines.push_back("Debug");
        if (!noLeakCheck) defines.push_back("LeakCheck");
    }
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
    if (releaseMode && releaseSafeMode) ABORT("can't combine --release with --release-safe");
    if (releaseMode) {
        buildMode = BuildMode::ReleaseFast;
    } else if (releaseSafeMode) {
        buildMode = BuildMode::ReleaseSafe;
    }
    if (!programArgs.empty() && !run) {
        ABORT("program arguments require the 'run' subcommand");
    }
    if (demangleSubcommand) return runDemangle();
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
            .runTests = bool(testSubcommand),
        });
    } else if (build || run) {
        // Build the current directory by relative path so diagnostics show
        // relative paths.
        return buildDirectory(".", argv[0]);
    } else if (testSubcommand) {
        return buildDirectory(".", argv[0], true);
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
