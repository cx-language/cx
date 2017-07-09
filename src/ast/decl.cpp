#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "../support/utility.h"

using namespace delta;

const FuncType* FuncDecl::getFuncType() const {
    auto paramTypes = map(params, *[](const ParamDecl& p) -> Type { return p.type; });
    return &llvm::cast<FuncType>(*FuncType::get(returnType, std::move(paramTypes)));
}

bool FuncDecl::signatureMatches(const FuncDecl& other, bool matchReceiver) const {
    if (name != other.name) return false;
    if (matchReceiver && getReceiverTypeDecl() != other.getReceiverTypeDecl()) return false;
    if (mutating != other.mutating) return false;
    if (returnType != other.returnType) return false;
    if (params != other.params) return false;
    return true;
}

void TypeDecl::addField(FieldDecl&& field) {
    fields.emplace_back(std::move(field));
}

void TypeDecl::addMemberFunc(std::unique_ptr<Decl> decl) {
    assert(decl->isFuncDecl() || decl->isInitDecl() || decl->isDeinitDecl());
    memberFuncs.emplace_back(std::move(decl));
}

Type TypeDecl::getType(llvm::ArrayRef<Type> genericArgs, bool isMutable) const {
    assert(genericArgs.size() == genericParams.size());
    return BasicType::get(name, genericArgs, isMutable);
}

Type TypeDecl::getTypeForPassing(llvm::ArrayRef<Type> genericArgs, bool isMutable) const {
    switch (tag) {
        case TypeTag::Struct: case TypeTag::Union:
            return getType(genericArgs, isMutable);
        case TypeTag::Class: case TypeTag::Interface:
            return PtrType::get(getType(genericArgs, isMutable), true);
    }
    llvm_unreachable("invalid type tag");
}

unsigned TypeDecl::getFieldIndex(llvm::StringRef fieldName) const {
    for (auto p : llvm::enumerate(fields)) {
        if (p.Value.name == fieldName) {
            return static_cast<unsigned>(p.Index);
        }
    }
    fatalError("unknown field");
}

SrcLoc Decl::getSrcLoc() const {
    switch (getKind()) {
        case DeclKind::ParamDecl:  return getParamDecl().srcLoc;
        case DeclKind::FuncDecl:   return getFuncDecl().srcLoc;
        case DeclKind::GenericParamDecl: return getGenericParamDecl().srcLoc;
        case DeclKind::InitDecl:   return getInitDecl().srcLoc;
        case DeclKind::DeinitDecl: return getDeinitDecl().srcLoc;
        case DeclKind::TypeDecl:   return getTypeDecl().srcLoc;
        case DeclKind::VarDecl:    return getVarDecl().srcLoc;
        case DeclKind::FieldDecl:  return getFieldDecl().srcLoc;
        case DeclKind::ImportDecl: return getImportDecl().srcLoc;
    }
    llvm_unreachable("all cases handled");
}
