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
    class ParamDecl;
    class Arg;

    std::string mangle(const FuncDecl& decl);
    std::string mangleFuncDecl(llvm::StringRef receiverType, llvm::StringRef funcName,
                               llvm::ArrayRef<Arg> args);
    std::string mangle(const GenericFuncDecl& decl, llvm::ArrayRef<Type> genericArgs);
    std::string mangle(const InitDecl& decl);
    std::string mangleInitDecl(llvm::StringRef typeName, llvm::ArrayRef<ParamDecl> params);
    std::string mangleInitDecl(llvm::StringRef typeName, llvm::ArrayRef<Arg> args);
    std::string mangle(const DeinitDecl& decl);
    std::string mangleDeinitDecl(llvm::StringRef typeName);
}
