#include "type-resolver.h"
#include "type.h"

using namespace delta;

Type TypeResolver::resolve(Type type) const {
    if (!type) return type;

    switch (type.getKind()) {
        case TypeKind::BasicType: {
            if (Type resolvedType = resolveTypePlaceholder(type.getName())) {
                // TODO: Handle generic arguments for type placeholders.
                return resolvedType.asMutable(type.isMutable());
            }
            auto genericArgs = map(type.getGenericArgs(), [&](Type type) { return resolve(type); });
            return BasicType::get(type.getName(), std::move(genericArgs), type.isMutable());
        }
        case TypeKind::PointerType:
            return PointerType::get(resolve(type.getPointee()), type.isReference(), type.isMutable());
        case TypeKind::ArrayType:
            return ArrayType::get(resolve(type.getElementType()), type.getArraySize(), type.isMutable());
        case TypeKind::FunctionType: {
            auto resolvedParamTypes = map(type.getParamTypes(), [&](Type type) { return resolve(type); });
            return FunctionType::get(resolve(type.getReturnType()), std::move(resolvedParamTypes),
                                     type.isMutable());
        }
        default:
            fatalError(("resolve() not implemented for type '" + type.toString() + "'").c_str());
    }
}
