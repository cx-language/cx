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

std::string delta::mangle(const FunctionDecl& decl) {
    Type receiver = decl.getTypeDecl() ? decl.getTypeDecl()->getType() : nullptr;
    return mangleFunctionDecl(receiver, decl.getName(), decl.getGenericArgs());
}

std::string delta::mangleFunctionDecl(Type receiver, llvm::StringRef name, llvm::ArrayRef<Type> genericArgs) {
    std::string mangled;

    if (receiver) {
        mangled = receiver.toString(true);
        mangled += '.';
    }

    mangled += name;
    appendGenericArgs(mangled, genericArgs);
    return mangled;
}

std::string delta::mangle(const FunctionTemplate& decl) {
    std::string receiverTypeName;

    if (decl.getFunctionDecl()->getTypeDecl()) {
        receiverTypeName = decl.getFunctionDecl()->getTypeDecl()->getName();
        appendGenericArgs(receiverTypeName, decl.getFunctionDecl()->getTypeDecl()->getGenericArgs());
    }

    return mangleFunctionTemplate(receiverTypeName, decl.getFunctionDecl()->getName());
}

std::string delta::mangleFunctionTemplate(llvm::StringRef receiverType, llvm::StringRef functionName) {
    std::string mangled;

    if (receiverType.empty()) {
        mangled = functionName;
    } else {
        mangled = (receiverType + "." + functionName).str();
    }

    return mangled;
}

std::string delta::mangle(const TypeDecl& decl) {
    return mangleTypeDecl(decl.getName(), decl.getGenericArgs());
}

std::string delta::mangleTypeDecl(llvm::StringRef typeName, llvm::ArrayRef<Type> genericArgs) {
    std::string mangled = typeName;
    appendGenericArgs(mangled, genericArgs);
    return mangled;
}

std::string delta::mangle(const FieldDecl& decl) {
    return (mangle(*decl.getParent()) + "." + decl.getName()).str();
}

std::string delta::mangle(const EnumDecl& decl) {
    return mangleTypeDecl(decl.getName(), {});
}
