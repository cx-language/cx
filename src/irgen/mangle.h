#pragma once

#include <string>

namespace llvm {
    template<typename T> class ArrayRef;
}

namespace delta {
    class Type;
    struct GenericFuncDecl;

    std::string mangle(const GenericFuncDecl& decl, llvm::ArrayRef<Type> genericArgs);
}
