#include "type-resolver.h"
#include "type.h"

using namespace delta;

Type TypeResolver::resolve(Type type) const {
    switch (type.getKind()) {
        case TypeKind::BasicType:
            if (Type resolvedType = resolveTypePlaceholder(type.getName())) {
                return resolvedType.asMutable(type.isMutable());
            }
            return type;
        case TypeKind::PointerType:
            return PointerType::get(resolve(type.getPointee()), type.isReference(), type.isMutable());
        case TypeKind::ArrayType:
            return ArrayType::get(resolve(type.getElementType()), type.getArraySize(), type.isMutable());
        case TypeKind::RangeType:
            return RangeType::get(resolve(type.getIterableElementType()),
                                  llvm::cast<RangeType>(*type).isExclusive(), type.isMutable());
        case TypeKind::FunctionType: {
            auto resolvedParamTypes = map(type.getParamTypes(), [&](Type type) { return resolve(type); });
            return FunctionType::get(resolve(type.getReturnType()), std::move(resolvedParamTypes),
                                     type.isMutable());
        }
        default:
            fatalError(("resolve() not implemented for type '" + type.toString() + "'").c_str());
    }
}
