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

} // namespace cx
