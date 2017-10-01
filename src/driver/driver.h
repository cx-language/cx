#pragma once

#include <string>
#include <vector>

namespace llvm {
template<typename T> class ArrayRef;
class StringRef;
}

namespace delta {

/// If `args` contains `flag`, removes it and returns true, otherwise returns false.
bool checkFlag(llvm::StringRef flag, std::vector<llvm::StringRef>& args);
int buildPackage(llvm::StringRef packageRoot, std::vector<llvm::StringRef>& args, bool run);
int buildExecutable(llvm::ArrayRef<std::string> files, std::vector<llvm::StringRef>& args, bool run);

}
