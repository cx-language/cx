#pragma once

namespace llvm {
    class StringRef;
    template<typename T> class ArrayRef;
}

namespace delta {

class FileUnit;

/// Returns true if the header was found and successfully imported.
bool importCHeader(FileUnit& importer, llvm::StringRef headerName,
                   llvm::ArrayRef<llvm::StringRef> includePaths);

}
