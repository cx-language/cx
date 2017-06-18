#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "../support/utility.h"

using namespace delta;

const FuncType* FuncDecl::getFuncType() const {
    auto paramTypes = map(params, *[](const ParamDecl& p) { return p.type; });
    return &llvm::cast<FuncType>(*FuncType::get(returnType, std::move(paramTypes)));
}

bool FuncDecl::signatureMatches(const FuncDecl& other, bool matchReceiver) const {
    if (name != other.name) return false;
    if (matchReceiver && receiverType != other.receiverType) return false;
    if (mutating != other.mutating) return false;
    if (returnType != other.returnType) return false;
    if (params != other.params) return false;
    return true;
}

llvm::StringRef InitDecl::getTypeName() const {
    return type.which() ? boost::get<TypeDecl*>(type)->name : boost::get<std::string>(type);
}

llvm::StringRef DeinitDecl::getTypeName() const {
    return type.which() ? boost::get<TypeDecl*>(type)->name : boost::get<std::string>(type);
}

Type TypeDecl::getType(bool isMutable) const {
    return BasicType::get(name, isMutable);
}

Type TypeDecl::getTypeForPassing(bool isMutable) const {
    return tag == TypeTag::Struct ? getType(isMutable) : PtrType::get(getType(isMutable), true);
}

unsigned TypeDecl::getFieldIndex(llvm::StringRef fieldName) const {
    for (unsigned index = 0, size = fields.size(); index < size; ++index) {
        if (fields[index].name == fieldName) return index;
    }
    fatalError("unknown field");
}

SrcLoc Decl::getSrcLoc() const {
    switch (getKind()) {
        case DeclKind::ParamDecl:  return getParamDecl().srcLoc;
        case DeclKind::FuncDecl:   return getFuncDecl().srcLoc;
        case DeclKind::GenericParamDecl: return getGenericParamDecl().srcLoc;
        case DeclKind::GenericFuncDecl: return getGenericFuncDecl().func->srcLoc;
        case DeclKind::InitDecl:   return getInitDecl().srcLoc;
        case DeclKind::DeinitDecl: return getDeinitDecl().srcLoc;
        case DeclKind::TypeDecl:   return getTypeDecl().srcLoc;
        case DeclKind::VarDecl:    return getVarDecl().srcLoc;
        case DeclKind::FieldDecl:  return getFieldDecl().srcLoc;
        case DeclKind::ImportDecl: return getImportDecl().srcLoc;
    }
    llvm_unreachable("all cases handled");
}
