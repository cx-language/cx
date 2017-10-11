#pragma once

#include <string>

namespace llvm {

class StringRef;
template<typename T> class ArrayRef;

}

namespace delta {

struct Type;
class FunctionDecl;
class TypeDecl;

std::string mangle(const FunctionDecl& decl, llvm::ArrayRef<Type> typeGenericArgs = {},
                   llvm::ArrayRef<Type> functionGenericArgs = {});
std::string mangleFunctionDecl(llvm::StringRef receiverType, llvm::StringRef functionName,
                               llvm::ArrayRef<Type> genericArgs = {});
std::string mangle(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs);

}
