#pragma once

#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#pragma warning(pop)

namespace llvm {
class MemoryBufferRef;
} // namespace llvm

namespace cx {

struct Module;
struct BuildConfig;

enum class BuildMode { Debug, ReleaseSafe, ReleaseFast };

struct CompileOptions {
    BuildMode mode = BuildMode::Debug;
    bool noUnusedWarnings = false;
    bool checkAll = false;
    bool warnUndefinedMacros = false;
    bool warnUnusedResult = false;
    bool noLeakCheck = false;
    bool dwarfDebugInfo = false;
    std::vector<std::string> importSearchPaths = {};
    std::vector<std::string> frameworkSearchPaths = {};
    std::vector<std::string> defines = {};
    std::vector<std::string> cflags = {};
};

struct BuildParams {
    llvm::ArrayRef<std::string> filePaths = {};
    const BuildConfig* config = nullptr;
    const char* argv0 = nullptr;
    llvm::StringRef outputDirectory = {};
    std::string outputFileName = {};
    bool createSharedLib = false;
    bool runTests = false;
};

int driverMain(int argc, const char** argv);
int buildModule(Module& mainModule, BuildParams buildParams);
llvm::MemoryBufferRef addFileBufferToModule(llvm::StringRef filePath, Module& targetModule);

} // namespace cx
