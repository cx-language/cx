#pragma once

#include <string>
#include <vector>

namespace llvm {
class StringRef;
}

namespace cx {

struct BuildConfig;

void fetchDependencies(const BuildConfig& config);
std::vector<std::string> getSourceFiles(llvm::StringRef rootDirectory);
// True when path lies under a 'vendor' directory. Vendored packages are
// imported by name, never compiled as part of the importing module.
bool isVendoredPath(llvm::StringRef path);
// True when path is rootDirectory's build.cx, the one reserved build file.
// A build.cx anywhere else is an ordinary source file and compiles as usual.
bool isRootBuildFile(llvm::StringRef path, llvm::StringRef rootDirectory);

} // namespace cx
