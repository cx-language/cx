#pragma once

#include <string>
#include <vector>

namespace llvm {
class StringRef;
}

namespace cx {

struct PackageManifest;

void fetchDependencies(const PackageManifest& manifest);
std::vector<std::string> getSourceFiles(llvm::StringRef rootDirectory);

} // namespace cx
