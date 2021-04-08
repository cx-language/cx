#pragma once

#include <string>
#include <vector>

namespace llvm {
class StringRef;
}

namespace cx {

void fetchDependencies(llvm::StringRef packageRoot);
std::vector<std::string> getSourceFiles(llvm::StringRef rootDirectory, llvm::StringRef packageManifestPath);

} // namespace cx
