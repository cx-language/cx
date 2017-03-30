#pragma once

#include <string>

namespace llvm {
    template<typename T> class ArrayRef;
}

namespace delta {
    struct Type;
    class FuncDecl;
    class GenericFuncDecl;
    class InitDecl;
    class DeinitDecl;

    std::string mangle(const FuncDecl& decl);
    std::string mangle(const GenericFuncDecl& decl, llvm::ArrayRef<Type> genericArgs);
    std::string mangle(const InitDecl& decl);
    std::string mangleInitDecl(llvm::StringRef typeName);
    std::string mangle(const DeinitDecl& decl);
    std::string mangleDeinitDecl(llvm::StringRef typeName);
}
