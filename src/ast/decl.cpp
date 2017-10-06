#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "../support/utility.h"

using namespace delta;

Module* Decl::getModule() const {
    switch (getKind()) {
        case DeclKind::ParamDecl: return llvm::cast<ParamDecl>(this)->getParent()->getModule();
        case DeclKind::GenericParamDecl: return llvm::cast<GenericParamDecl>(this)->getParent()->getModule();
        case DeclKind::FunctionDecl: return llvm::cast<FunctionDecl>(this)->getModule();
        case DeclKind::MethodDecl: return llvm::cast<MethodDecl>(this)->getTypeDecl()->getModule();
        case DeclKind::InitDecl: return llvm::cast<InitDecl>(this)->getTypeDecl()->getModule();
        case DeclKind::DeinitDecl: return llvm::cast<DeinitDecl>(this)->getTypeDecl()->getModule();
        case DeclKind::TypeDecl: return llvm::cast<TypeDecl>(this)->getModule();
        case DeclKind::VarDecl: return llvm::cast<VarDecl>(this)->getModule();
        case DeclKind::FieldDecl: return llvm::cast<FieldDecl>(this)->getParent()->getModule();
        case DeclKind::ImportDecl: return llvm::cast<ImportDecl>(this)->getModule();
    }
    llvm_unreachable("all cases handled");
}

const FunctionType* FunctionLikeDecl::getFunctionType() const {
    auto paramTypes = map(getParams(), [](const ParamDecl& p) -> Type { return p.getType(); });
    return &llvm::cast<FunctionType>(*FunctionType::get(getReturnType(), std::move(paramTypes)));
}

bool FunctionDecl::signatureMatches(const FunctionDecl& other, bool matchReceiver) const {
    if (getName() != other.getName()) return false;
    if (matchReceiver && getTypeDecl() != other.getTypeDecl()) return false;
    if (isMutating() != other.isMutating()) return false;
    if (getReturnType() != other.getReturnType()) return false;
    if (getParams() != other.getParams()) return false;
    return true;
}

MethodDecl::MethodDecl(DeclKind kind, FunctionProto proto, TypeDecl& typeDecl,
                       SourceLocation location)
: FunctionDecl(kind, std::move(proto), *typeDecl.getModule(), location), typeDecl(&typeDecl),
  mutating(false) {}

Type MethodDecl::getThisType() const {
    if (getTypeDecl()->passByValue() && !isMutating()) {
        return getTypeDecl()->getUnresolvedType(isMutating());
    } else {
        return PointerType::get(getTypeDecl()->getUnresolvedType(isMutating()), true);
    }
}

void TypeDecl::addField(FieldDecl&& field) {
    fields.emplace_back(std::move(field));
}

void TypeDecl::addMethod(std::unique_ptr<FunctionLikeDecl> decl) {
    ASSERT(decl->isMethodDecl() || decl->isInitDecl() || decl->isDeinitDecl());
    methods.emplace_back(std::move(decl));
}

DeinitDecl* TypeDecl::getDeinitializer() const {
    for (auto& decl : getMemberDecls()) {
        if (auto* deinitDecl = llvm::dyn_cast<DeinitDecl>(decl.get())) {
            return deinitDecl;
        }
    }
    return nullptr;
}

Type TypeDecl::getType(llvm::ArrayRef<Type> genericArgs, bool isMutable) const {
    ASSERT(genericArgs.size() == genericParams.size());
    return BasicType::get(name, genericArgs, isMutable);
}

Type TypeDecl::getUnresolvedType(bool isMutable) const {
    auto genericArgs = map(getGenericParams(),
                           [](const GenericParamDecl& p) { return BasicType::get(p.getName(), {}); });
    return BasicType::get(name, std::move(genericArgs), isMutable);
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
        if (p.value().getName() == fieldName) {
            return static_cast<unsigned>(p.index());
        }
    }
    fatalError("unknown field");
}

SourceLocation Decl::getLocation() const {
    switch (getKind()) {
        case DeclKind::ParamDecl: return llvm::cast<ParamDecl>(this)->getLocation();
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::InitDecl:
        case DeclKind::DeinitDecl: return llvm::cast<FunctionLikeDecl>(this)->getLocation();
        case DeclKind::GenericParamDecl: return llvm::cast<GenericParamDecl>(this)->getLocation();
        case DeclKind::TypeDecl: return llvm::cast<TypeDecl>(this)->getLocation();
        case DeclKind::VarDecl: return llvm::cast<VarDecl>(this)->getLocation();
        case DeclKind::FieldDecl: return llvm::cast<FieldDecl>(this)->getLocation();
        case DeclKind::ImportDecl: return llvm::cast<ImportDecl>(this)->getLocation();
    }
    llvm_unreachable("all cases handled");
}
