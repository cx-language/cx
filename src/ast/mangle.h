#pragma once

#include <string>

namespace llvm {
    class StringRef;
    template<typename T> class ArrayRef;
}

namespace delta {
    struct Type;
    class FunctionDecl;
    class InitDecl;
    class DeinitDecl;
    class ParamDecl;
    class Argument;
    class TypeDecl;

    std::string mangle(const FunctionDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {},
                       llvm::ArrayRef<Type> functionGenericArgs = {});
    std::string mangleFunctionDecl(llvm::StringRef receiverType, llvm::StringRef functionName,
                                   llvm::ArrayRef<Type> genericArgs = {});
    std::string mangle(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {},
                       llvm::ArrayRef<Type> functionGenericArgs = {});
    std::string mangleInitDecl(llvm::StringRef typeName,
                               llvm::ArrayRef<Type> genericArgs = {});
    std::string mangle(const DeinitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {});
    std::string mangleDeinitDecl(llvm::StringRef typeName);
    std::string mangle(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs);
}
