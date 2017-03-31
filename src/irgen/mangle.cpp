#include <llvm/ADT/ArrayRef.h>
#include "mangle.h"
#include "../ast/decl.h"

using namespace delta;

namespace {

template<typename T>
std::string appendLabels(std::string&& result, llvm::ArrayRef<T> labeledItems) {
    for (const T& item : labeledItems) {
        if (item.label.empty()) continue;
        result.append("$").append(item.label);
    }
    return std::move(result);
}

}

std::string delta::mangle(const FuncDecl& decl) {
    llvm::ArrayRef<ParamDecl> params = decl.params;
    if (decl.receiverType.empty()) return appendLabels(std::string(decl.name), params);
    return appendLabels(decl.receiverType + "." + decl.name, params);
}

std::string delta::mangleFuncDecl(llvm::StringRef receiverType, llvm::StringRef funcName,
                                  llvm::ArrayRef<Arg> args) {
    if (receiverType.empty()) return appendLabels(funcName.str(), args);
    return appendLabels(receiverType.str() + "." + funcName.str(), args);
}

std::string delta::mangle(const GenericFuncDecl& decl, llvm::ArrayRef<Type> genericArgs) {
    std::string mangled = decl.func->name;
    mangled += '<';
    for (const Type& genericArg : genericArgs) {
        mangled += genericArg.toString();
        if (&genericArg != &genericArgs.back()) mangled += ", ";
    }
    mangled += '>';
    return mangled;
}

std::string delta::mangle(const InitDecl& decl) {
    return mangleInitDecl(decl.getTypeName(), decl.params);
}

std::string delta::mangleInitDecl(llvm::StringRef typeName, llvm::ArrayRef<ParamDecl> params) {
    return appendLabels(typeName.str() + ".init", params);
}

std::string delta::mangleInitDecl(llvm::StringRef typeName, llvm::ArrayRef<Arg> args) {
    return appendLabels(typeName.str() + ".init", args);
}

std::string delta::mangle(const DeinitDecl& decl) {
    return mangleDeinitDecl(decl.getTypeName());
}

std::string delta::mangleDeinitDecl(llvm::StringRef typeName) {
    return typeName.str() + ".deinit";
}
