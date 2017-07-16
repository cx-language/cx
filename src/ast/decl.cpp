#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "../support/utility.h"

using namespace delta;

const FunctionType* FunctionDecl::getFunctionType() const {
    auto paramTypes = map(params, *[](const ParamDecl& p) -> Type { return p.type; });
    return &llvm::cast<FunctionType>(*FunctionType::get(returnType, std::move(paramTypes)));
}

bool FunctionDecl::signatureMatches(const FunctionDecl& other, bool matchReceiver) const {
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

void TypeDecl::addMethod(std::unique_ptr<Decl> decl) {
    assert(decl->isFunctionDecl() || decl->isInitDecl() || decl->isDeinitDecl());
    methods.emplace_back(std::move(decl));
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
            return PointerType::get(getType(genericArgs, isMutable), true);
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

SourceLocation Decl::getLocation() const {
    switch (getKind()) {
        case DeclKind::ParamDecl: return llvm::cast<ParamDecl>(*this).location;
        case DeclKind::FunctionDecl: return llvm::cast<FunctionDecl>(*this).location;
        case DeclKind::GenericParamDecl: return llvm::cast<GenericParamDecl>(*this).location;
        case DeclKind::InitDecl: return llvm::cast<InitDecl>(*this).location;
        case DeclKind::DeinitDecl: return llvm::cast<DeinitDecl>(*this).location;
        case DeclKind::TypeDecl: return llvm::cast<TypeDecl>(*this).location;
        case DeclKind::VarDecl: return llvm::cast<VarDecl>(*this).location;
        case DeclKind::FieldDecl: return llvm::cast<FieldDecl>(*this).location;
        case DeclKind::ImportDecl: return llvm::cast<ImportDecl>(*this).location;
    }
    llvm_unreachable("all cases handled");
}
