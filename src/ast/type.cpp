#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include "type.h"
#include "../driver/utility.h"

using namespace delta;

ArrayType::ArrayType(const ArrayType& type) : elementType(new Type(*type.elementType)), size(type.size) { }

ArrayType& ArrayType::operator=(const ArrayType& type) {
    if (&type != this) {
        elementType.reset(new Type(*type.elementType));
        size = type.size;
    }
    return *this;
}

PtrType::PtrType(const PtrType& type) : pointeeType(new Type(*type.pointeeType)), ref(type.ref) { }

PtrType& PtrType::operator=(const PtrType& type) {
    if (&type != this) {
        pointeeType.reset(new Type(*type.pointeeType));
        ref = type.ref;
    }
    return *this;
}

void Type::appendType(Type type) {
    llvm::Optional<Type> firstType;
    switch (getKind()) {
        case TypeKind::BasicType: firstType = Type(std::move(getBasicType())); break;
        case TypeKind::ArrayType: firstType = Type(std::move(getArrayType())); break;
        case TypeKind::TupleType: break;
        case TypeKind::FuncType: firstType = Type(std::move(getFuncType())); break;
        case TypeKind::PtrType: firstType = Type(std::move(getPtrType())); break;
    }
    if (firstType) {
        data = TupleType();
        getTupleType().subtypes.emplace_back(std::move(*firstType));
    }
    getTupleType().subtypes.emplace_back(std::move(type));
}

bool Type::isImplicitlyConvertibleTo(const Type& type) const {
    switch (getKind()) {
        case TypeKind::BasicType:
            return type.isBasicType() && getName() == type.getName();
        case TypeKind::ArrayType:
            return type.isArrayType() && getArraySize() == type.getArraySize()
            && getElementType().isImplicitlyConvertibleTo(type.getElementType());
        case TypeKind::TupleType:
            return type.isTupleType() && getSubtypes() == type.getSubtypes();
        case TypeKind::FuncType:
            return type.isFuncType() && getReturnTypes() == type.getReturnTypes()
                                     && getParamTypes() == type.getParamTypes();
        case TypeKind::PtrType:
            return type.isPtrType()
            && (isRef() || !type.isRef())
            && (getPointee().isMutable() || !type.getPointee().isMutable())
            && getPointee().isImplicitlyConvertibleTo(type.getPointee());
    }
}

bool Type::isSigned() const {
    assert(isBasicType());
    llvm::StringRef name = getName();
    return name == "int" || name == "int8" || name == "int16" || name == "int32" || name == "int64";
}

bool delta::operator==(const Type& lhs, const Type& rhs) {
    if (lhs.isMutable() != rhs.isMutable()) return false;
    switch (lhs.getKind()) {
        case TypeKind::BasicType:
            return rhs.isBasicType() && lhs.getName() == rhs.getName();
        case TypeKind::ArrayType:
            return rhs.isArrayType() && lhs.getElementType() == rhs.getElementType();
        case TypeKind::TupleType:
            return rhs.isTupleType() && lhs.getSubtypes() == rhs.getSubtypes();
        case TypeKind::FuncType:
            return rhs.isFuncType() && lhs.getReturnTypes() == rhs.getReturnTypes()
                                    && lhs.getParamTypes() == rhs.getParamTypes();
        case TypeKind::PtrType:
            return rhs.isPtrType() && lhs.isRef() == rhs.isRef() && lhs.getPointee() == rhs.getPointee();
    }
}

bool delta::operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
}

void Type::printTo(std::ostream& stream, bool omitTopLevelMutable) const {
    switch (getKind()) {
        case TypeKind::BasicType:
            if (isMutable() && !omitTopLevelMutable) stream << "mutable ";
            stream << getName();
            break;
        case TypeKind::ArrayType:
            getElementType().printTo(stream, omitTopLevelMutable);
            stream << "[" << getArraySize() << "]";
            break;
        case TypeKind::TupleType:
            stream << "(";
            for (const Type& subtype : getSubtypes()) {
                subtype.printTo(stream, omitTopLevelMutable);
                if (&subtype != &getSubtypes().back()) stream << ", ";
            }
            stream << ")";
            break;
        case TypeKind::FuncType:
            stream << "func(";
            for (const Type& paramType : getParamTypes()) {
                stream << paramType;
                if (&paramType != &getParamTypes().back()) stream << ", ";
            }
            stream << ") -> ";
            for (const Type& returnType : getReturnTypes()) {
                stream << returnType;
                if (&returnType != &getReturnTypes().back()) stream << ", ";
            }
            break;
        case TypeKind::PtrType:
            if (isMutable() && !omitTopLevelMutable) {
                stream << "mutable(";
                getPointee().printTo(stream, false);
                stream << (isRef() ? '&' : '*') << ')';
            } else {
                getPointee().printTo(stream, false);
                stream << (isRef() ? '&' : '*');
            }
            break;
    }
}

std::ostream& delta::operator<<(std::ostream& stream, const Type& type) {
    type.printTo(stream, true);
    return stream;
}
