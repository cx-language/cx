#pragma once

namespace llvm {
class StringRef;
template<typename T>
class ArrayRef;
}

namespace delta {

class SourceFile;

/// Returns true if the header was found and successfully imported.
bool importCHeader(SourceFile& importer, llvm::StringRef headerName, llvm::ArrayRef<std::string> importSearchPaths,
                   llvm::ArrayRef<std::string> frameworkSearchPaths);

}
