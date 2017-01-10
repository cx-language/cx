#include <boost/optional.hpp>
#include "type.h"

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
        case TypeKind::BasicType: firstType = std::move(getBasicType()); break;
        case TypeKind::TupleType: break;
        case TypeKind::FuncType: firstType = std::move(getFuncType()); break;
        case TypeKind::PtrType: firstType = std::move(getPtrType()); break;
    }
    if (firstType) {
        data = TupleType();
        getTupleType().subtypes.emplace_back(std::move(*firstType));
    }
    getTupleType().subtypes.emplace_back(std::move(type));
}

bool operator==(const Type& lhs, const Type& rhs) {
    switch (lhs.getKind()) {
        case TypeKind::BasicType:
            return rhs.getKind() == TypeKind::BasicType
            && lhs.getBasicType().name == rhs.getBasicType().name;
        case TypeKind::TupleType:
            return rhs.getKind() == TypeKind::TupleType
            && lhs.getTupleType().subtypes == rhs.getTupleType().subtypes;
        case TypeKind::FuncType:
            return rhs.getKind() == TypeKind::FuncType
            && lhs.getFuncType().returnTypes == rhs.getFuncType().returnTypes
            && lhs.getFuncType().paramTypes == rhs.getFuncType().paramTypes;
        case TypeKind::PtrType:
            return rhs.getKind() == TypeKind::PtrType
            && *lhs.getPtrType().pointeeType == *rhs.getPtrType().pointeeType;
    }
}

bool operator!=(const Type& lhs, const Type& rhs) {
    return !(lhs == rhs);
}

std::ostream& operator<<(std::ostream& out, const Type& type) {
    switch (type.getKind()) {
        case TypeKind::BasicType:
            return out << type.getBasicType().name;
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
            return out << *type.getPtrType().pointeeType << "*";
    }
}
