#include <boost/optional.hpp>
#include <llvm/ADT/StringRef.h>
#include "type.h"

ArrayType::ArrayType(const ArrayType& type) : elementType(new Type(*type.elementType)), size(type.size) { }

ArrayType& ArrayType::operator=(const ArrayType& type) {
    if (&type != this) {
        elementType.reset(new Type(*type.elementType));
        size = type.size;
    }
    return *this;
}

PtrType::PtrType(const PtrType& type) : pointeeType(new Type(*type.pointeeType)) { }

PtrType& PtrType::operator=(const PtrType& type) {
    if (&type != this) {
        pointeeType.reset(new Type(*type.pointeeType));
    }
    return *this;
}

void Type::appendType(Type type) {
    boost::optional<Type> firstType;
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
            return type.isBasicType() && getBasicType().name == type.getBasicType().name;
        case TypeKind::ArrayType:
            return type.isArrayType() && getArrayType().size == type.getArrayType().size
            && getArrayType().elementType->isImplicitlyConvertibleTo(*type.getArrayType().elementType);
        case TypeKind::TupleType:
            return type.isTupleType() && getTupleType().subtypes == type.getTupleType().subtypes;
        case TypeKind::FuncType:
            return type.isFuncType() && getFuncType().returnTypes == type.getFuncType().returnTypes
                                     && getFuncType().paramTypes == type.getFuncType().paramTypes;
        case TypeKind::PtrType:
            return type.isPtrType()
            && (getPtrType().pointeeType->isMutable() || !type.getPtrType().pointeeType->isMutable())
            && getPtrType().pointeeType->isImplicitlyConvertibleTo(*type.getPtrType().pointeeType);
    }
}

bool Type::isSigned() const {
    assert(isBasicType());
    llvm::StringRef name = getBasicType().name;
    return name == "int" || name == "int8" || name == "int16" || name == "int32" || name == "int64";
}

bool operator==(const Type& lhs, const Type& rhs) {
    if (lhs.isMutable() != rhs.isMutable()) return false;
    switch (lhs.getKind()) {
        case TypeKind::BasicType:
            return rhs.isBasicType() && lhs.getBasicType().name == rhs.getBasicType().name;
        case TypeKind::ArrayType:
            return rhs.isArrayType() && lhs.getArrayType().elementType == rhs.getArrayType().elementType;
        case TypeKind::TupleType:
            return rhs.isTupleType() && lhs.getTupleType().subtypes == rhs.getTupleType().subtypes;
        case TypeKind::FuncType:
            return rhs.isFuncType() && lhs.getFuncType().returnTypes == rhs.getFuncType().returnTypes
                                    && lhs.getFuncType().paramTypes == rhs.getFuncType().paramTypes;
        case TypeKind::PtrType:
            return rhs.isPtrType() && *lhs.getPtrType().pointeeType == *rhs.getPtrType().pointeeType;
    }
}

bool operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Type& type) {
    switch (type.getKind()) {
        case TypeKind::BasicType:
            if (type.isMutable()) out << "mutable ";
            return out << type.getBasicType().name;
        case TypeKind::ArrayType:
            return out << *type.getArrayType().elementType << "[" << type.getArrayType().size << "]";
        case TypeKind::TupleType:
            out << "(";
            for (const Type& subtype : type.getTupleType().subtypes) {
                out << subtype;
                if (&subtype != &type.getTupleType().subtypes.back()) out << ", ";
            }
            return out << ")";
        case TypeKind::FuncType:
            out << "func(";
            for (const Type& paramType : type.getFuncType().paramTypes) {
                out << paramType;
                if (&paramType != &type.getFuncType().paramTypes.back()) out << ", ";
            }
            out << ") -> ";
            for (const Type& returnType : type.getFuncType().returnTypes) {
                out << returnType;
                if (&returnType != &type.getFuncType().returnTypes.back()) out << ", ";
            }
            return out;
        case TypeKind::PtrType:
            if (type.isMutable()) return out << "mutable(" << *type.getPtrType().pointeeType << "*)";
            return out << *type.getPtrType().pointeeType << "*";
    }
}
