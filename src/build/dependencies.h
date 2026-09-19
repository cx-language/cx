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

} // namespace cx
