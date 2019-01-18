#pragma once

#include <string>
#include <vector>

namespace llvm {
template<typename T>
class ArrayRef;
class StringRef;
} // namespace llvm

namespace delta {

class PackageManifest;

/// If `args` contains `flag`, removes it and returns true, otherwise returns false.
bool checkFlag(llvm::StringRef flag, std::vector<llvm::StringRef>& args);
int buildPackage(llvm::StringRef packageRoot, const char* argv0, std::vector<llvm::StringRef>& args, bool run);
int buildExecutable(llvm::ArrayRef<std::string> files, const PackageManifest* manifest, const char* argv0,
                    std::vector<llvm::StringRef>& args, llvm::StringRef outputDirectory, llvm::StringRef outputFileName, bool run);

} // namespace delta
