#include <llvm/ADT/ArrayRef.h>
#include "mangle.h"
#include "../ast/decl.h"

using namespace delta;

static void appendGenericArgList(std::string& mangled, llvm::ArrayRef<Type> genericArgs) {
    if (genericArgs.empty()) return;
    
    mangled += '<';
    for (const Type& genericArg : genericArgs) {
        mangled += genericArg.toString();
        if (&genericArg != &genericArgs.back()) mangled += ", ";
    }
    mangled += '>';
}

std::string delta::mangle(const FuncDecl& decl, llvm::ArrayRef<Type> typeGenericArgs,
                          llvm::ArrayRef<Type> funcGenericArgs) {
    auto receiverTypeName = decl.getReceiverTypeDecl() ? decl.getReceiverTypeDecl()->name : "";
    appendGenericArgList(receiverTypeName, typeGenericArgs);
    return mangleFuncDecl(receiverTypeName, decl.name, funcGenericArgs);
}

std::string delta::mangleFuncDecl(llvm::StringRef receiverType, llvm::StringRef funcName,
                                  llvm::ArrayRef<Type> genericArgs) {
    std::string mangled;
    if (receiverType.empty()) {
        mangled = funcName.str();
    } else {
        mangled = receiverType.str() + "." + funcName.str();
    }
    appendGenericArgList(mangled, genericArgs);
    return mangled;
}

std::string delta::mangle(const InitDecl& decl, llvm::ArrayRef<Type> typeGenericArgs,
                          llvm::ArrayRef<Type> funcGenericArgs) {
    std::string typeName = decl.getTypeName();
    appendGenericArgList(typeName, typeGenericArgs);
    return mangleInitDecl(typeName, funcGenericArgs);
}

std::string delta::mangleInitDecl(llvm::StringRef typeName, llvm::ArrayRef<Type> genericArgs) {
    auto mangled = typeName.str() + ".init";
    appendGenericArgList(mangled, genericArgs);
    return mangled;
}

std::string delta::mangle(const DeinitDecl& decl) {
    return mangleDeinitDecl(decl.getTypeName());
}

std::string delta::mangleDeinitDecl(llvm::StringRef typeName) {
    return typeName.str() + ".deinit";
}

std::string delta::mangle(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs) {
    auto mangled = decl.name;
    appendGenericArgList(mangled, genericArgs);
    return mangled;
}
