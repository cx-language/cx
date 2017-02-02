#include "decl.h"
#include <llvm/ADT/StringRef.h>

namespace {

std::vector<Type> mapToTypes(const std::vector<ParamDecl>& params) {
    std::vector<Type> paramTypes;
    paramTypes.reserve(params.size());
    for (const auto& param : params) paramTypes.emplace_back(param.type);
    return paramTypes;
}

} // anonymous namespace

FuncType FuncDecl::getFuncType() const {
    auto returnTypes = returnType.getKind() == TypeKind::TupleType
        ? returnType.getTupleType().subtypes : std::vector<Type>{returnType};
    return FuncType{returnTypes, mapToTypes(params)};
}

Type TypeDecl::getType() const {
    return BasicType{name};
}

unsigned TypeDecl::getFieldIndex(llvm::StringRef fieldName) const {
    for (unsigned index = 0, size = fields.size(); index < size; ++index) {
        if (fields[index].name == fieldName) return index;
    }
    assert(false && "unknown field");
}
