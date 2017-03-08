#include <llvm/ADT/ArrayRef.h>
#include "mangle.h"
#include "../ast/decl.h"

using namespace delta;

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
    return mangleInitDecl(decl.type.which() ? decl.getTypeDecl().name : decl.getTypeName());
}

std::string delta::mangleInitDecl(llvm::StringRef typeName) {
    return typeName.str() + ".init";
}

std::string delta::mangle(const DeinitDecl& decl) {
    return mangleDeinitDecl(decl.type.which() ? decl.getTypeDecl().name : decl.getTypeName());
}

std::string delta::mangleDeinitDecl(llvm::StringRef typeName) {
    return typeName.str() + ".deinit";
}
