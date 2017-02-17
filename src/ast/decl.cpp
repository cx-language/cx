#include "decl.h"
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>

using namespace delta;

namespace {

std::vector<Type> mapToTypes(const std::vector<ParamDecl>& params) {
    std::vector<Type> paramTypes;
    paramTypes.reserve(params.size());
    for (const auto& param : params) paramTypes.emplace_back(param.type);
    return paramTypes;
}

} // anonymous namespace

FuncType FuncDecl::getFuncType() const {
    auto returnTypes = returnType.isTupleType()
        ? returnType.getTupleType().subtypes : std::vector<Type>{returnType};
    return FuncType{returnTypes, mapToTypes(params)};
}

Type TypeDecl::getType() const {
    return BasicType{name};
}

Type TypeDecl::getTypeForPassing() const {
    return tag == TypeTag::Struct ? getType() : PtrType{llvm::make_unique<Type>(getType()), true};
}

unsigned TypeDecl::getFieldIndex(llvm::StringRef fieldName) const {
    for (unsigned index = 0, size = fields.size(); index < size; ++index) {
        if (fields[index].name == fieldName) return index;
    }
    assert(false && "unknown field");
}

SrcLoc Decl::getSrcLoc() const {
    switch (getKind()) {
        case DeclKind::ParamDecl:  return getParamDecl().srcLoc;
        case DeclKind::FuncDecl:   return getFuncDecl().srcLoc;
        case DeclKind::InitDecl:   return getInitDecl().srcLoc;
        case DeclKind::DeinitDecl: return getDeinitDecl().srcLoc;
        case DeclKind::TypeDecl:   return getTypeDecl().srcLoc;
        case DeclKind::VarDecl:    return getVarDecl().srcLoc;
        case DeclKind::FieldDecl:  return getFieldDecl().srcLoc;
        case DeclKind::ImportDecl: return getImportDecl().srcLoc;
    }
}
