#pragma once

#include <string>
#include <vector>

namespace llvm {
class StringRef;
}

namespace delta {

void fetchDependencies(llvm::StringRef packageRoot);
std::vector<std::string> getSourceFiles(llvm::StringRef packageRoot);

}
