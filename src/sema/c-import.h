#pragma once

namespace llvm {
    class StringRef;
    template<typename T> class ArrayRef;
}

namespace delta {

void importCHeader(llvm::StringRef headerName, llvm::ArrayRef<llvm::StringRef> includePaths);

}
