#include <llvm/ADT/ArrayRef.h>
#include "mangle.h"
#include "../ast/decl.h"

using namespace delta;

static void appendGenericArgs(std::string& mangled, llvm::ArrayRef<Type> genericArgs) {
    if (genericArgs.empty()) return;

    mangled += '<';
    for (const Type& genericArg : genericArgs) {
        mangled += genericArg.toString();
        if (&genericArg != &genericArgs.back()) mangled += ", ";
    }
    mangled += '>';
}

std::string delta::mangle(const FunctionDecl& decl, llvm::ArrayRef<Type> typeGenericArgs,
                          llvm::ArrayRef<Type> functionGenericArgs) {
    std::string receiverTypeName = decl.getTypeDecl() ? decl.getTypeDecl()->getName() : "";
    appendGenericArgs(receiverTypeName, typeGenericArgs);
    return mangleFunctionDecl(receiverTypeName, decl.getName(), functionGenericArgs);
}

std::string delta::mangleFunctionDecl(llvm::StringRef receiverType, llvm::StringRef functionName,
                                      llvm::ArrayRef<Type> genericArgs) {
    std::string mangled;
    if (receiverType.empty()) {
        mangled = functionName.str();
    } else {
        mangled = receiverType.str() + "." + functionName.str();
    }
    appendGenericArgs(mangled, genericArgs);
    return mangled;
}

std::string delta::mangle(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs) {
    std::string mangled = decl.getName();
    appendGenericArgs(mangled, genericArgs);
    return mangled;
}
