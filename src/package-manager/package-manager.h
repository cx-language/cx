#pragma once

#include <string>
#include <vector>

namespace llvm {
class StringRef;
}

namespace delta {

std::vector<std::string> getSourceFiles(llvm::StringRef packageRoot);

}
