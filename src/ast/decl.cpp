#include "decl.h"

static std::vector<Type> mapToTypes(const std::vector<ParamDecl>& params) {
    std::vector<Type> paramTypes;
    paramTypes.reserve(params.size());
    for (const auto& param : params) paramTypes.emplace_back(param.type);
    return paramTypes;
}

FuncType FuncDecl::getFuncType() const {
    auto returnTypes = returnType.getKind() == TypeKind::TupleType
        ? returnType.getTupleType().subtypes : std::vector<Type>{returnType};
    return FuncType{returnTypes, mapToTypes(params)};
}
