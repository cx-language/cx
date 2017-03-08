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
