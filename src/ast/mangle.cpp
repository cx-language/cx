#include <llvm/ADT/ArrayRef.h>
#include "mangle.h"
#include "../ast/decl.h"

using namespace delta;

static void appendGenericArgList(std::string& mangled, llvm::ArrayRef<Type> genericArgs) {
    mangled += '<';
    for (const Type& genericArg : genericArgs) {
        mangled += genericArg.toString();
        if (&genericArg != &genericArgs.back()) mangled += ", ";
    }
    mangled += '>';
}

std::string delta::mangle(const FuncDecl& decl) {
    return mangleFuncDecl(decl.receiverType, decl.name);
}

std::string delta::mangleFuncDecl(llvm::StringRef receiverType, llvm::StringRef funcName) {
    if (receiverType.empty()) return funcName.str();
    return receiverType.str() + "." + funcName.str();
}

std::string delta::mangle(const GenericFuncDecl& decl, llvm::ArrayRef<Type> genericArgs) {
    std::string mangled = decl.func->name;
    appendGenericArgList(mangled, genericArgs);
    return mangled;
}

std::string delta::mangle(const InitDecl& decl) {
    return mangleInitDecl(decl.getTypeName());
}

std::string delta::mangleInitDecl(llvm::StringRef typeName) {
    return typeName.str() + ".init";
}

std::string delta::mangle(const DeinitDecl& decl) {
    return mangleDeinitDecl(decl.getTypeName());
}

std::string delta::mangleDeinitDecl(llvm::StringRef typeName) {
    return typeName.str() + ".deinit";
}

std::string delta::mangle(const TypeDecl& decl, llvm::ArrayRef<Type> genericArgs) {
    std::string mangled = decl.name;
    appendGenericArgList(mangled, genericArgs);
    return mangled;
}
