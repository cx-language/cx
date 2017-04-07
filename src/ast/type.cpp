#include <sstream>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/STLExtras.h>
#include "type.h"
#include "../driver/utility.h"

using namespace delta;

#define DEFINE_BUILTIN_TYPE_GET_AND_IS(TYPE, NAME) \
Type Type::get##TYPE(bool isMutable) { \
    static BasicType type(#NAME); \
    return Type(&type, isMutable); \
} \
bool Type::is##TYPE() const { \
    return isBasicType() && getName() == #NAME; \
}

DEFINE_BUILTIN_TYPE_GET_AND_IS(Void, void)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Bool, bool)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int, int)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int8, int8)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int16, int16)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int32, int32)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int64, int64)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt, uint)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt8, uint8)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt16, uint16)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt32, uint32)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt64, uint64)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float, float)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float32, float32)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float64, float64)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float80, float80)
DEFINE_BUILTIN_TYPE_GET_AND_IS(String, string)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Char, char)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Null, null)
#undef DEFINE_BUILTIN_TYPE_GET_AND_IS

bool Type::isSizedArrayType() const {
    return isArrayType() && getArraySize() != ArrayType::unsized;
}

bool Type::isUnsizedArrayType() const {
    return isArrayType() && getArraySize() == ArrayType::unsized;
}

bool Type::isNullablePointer() const {
    return isPtrType() && !llvm::cast<PtrType>(typeBase)->ref;
}

bool Type::isBuiltinScalar(llvm::StringRef typeName) {
    static const char* const builtinTypeNames[] = {
        "int", "int8", "int16", "int32", "int64",
        "uint", "uint8", "uint16", "uint32", "uint64",
        "float", "float32", "float64", "float80", "bool", "char",
    };
    return std::find(std::begin(builtinTypeNames), std::end(builtinTypeNames), typeName)
        != std::end(builtinTypeNames);
}

void Type::appendType(Type type) {
    std::vector<Type> subtypes;
    if (!isTupleType())
        subtypes.push_back(*this);
    else
        subtypes = llvm::cast<TupleType>(*typeBase).subtypes;
    subtypes.push_back(type);
    typeBase = TupleType::get(std::move(subtypes)).get();
}

namespace {
    std::vector<std::unique_ptr<BasicType>> basicTypes;
    std::vector<std::unique_ptr<ArrayType>> arrayTypes;
    std::vector<std::unique_ptr<RangeType>> rangeTypes;
    std::vector<std::unique_ptr<TupleType>> tupleTypes;
    std::vector<std::unique_ptr<FuncType>> funcTypes;
    std::vector<std::unique_ptr<PtrType>> ptrTypes;
}

#define FETCH_AND_RETURN_TYPE(TYPE, CACHE, EQUALS, ...) \
    auto it = llvm::find_if(CACHE, [&](const std::unique_ptr<TYPE>& t) { return EQUALS; }); \
    if (it != CACHE.end()) return Type(it->get(), isMutable); \
    CACHE.emplace_back(new TYPE(__VA_ARGS__)); \
    return Type(CACHE.back().get(), isMutable);

Type BasicType::get(llvm::StringRef name, bool isMutable) {
    FETCH_AND_RETURN_TYPE(BasicType, basicTypes, t->name == name, name)
}

Type ArrayType::get(Type elementType, int64_t size, bool isMutable) {
    FETCH_AND_RETURN_TYPE(ArrayType, arrayTypes,
        t->elementType == elementType && t->size == size, elementType, size);
}

Type RangeType::get(Type elementType, bool isMutable) {
    FETCH_AND_RETURN_TYPE(RangeType, rangeTypes,
        t->elementType == elementType, elementType);
}

Type TupleType::get(std::vector<Type>&& subtypes, bool isMutable) {
    FETCH_AND_RETURN_TYPE(TupleType, tupleTypes,
        t->subtypes == subtypes, std::move(subtypes));
}

Type FuncType::get(Type returnType, std::vector<Type>&& paramTypes, bool isMutable) {
    FETCH_AND_RETURN_TYPE(FuncType, funcTypes,
        t->returnType == returnType && t->paramTypes == paramTypes,
        returnType, std::move(paramTypes));
}

Type PtrType::get(Type pointeeType, bool isReference, bool isMutable) {
    FETCH_AND_RETURN_TYPE(PtrType, ptrTypes,
        t->pointeeType == pointeeType && t->ref == isReference, pointeeType, isReference);
}

#undef FETCH_AND_RETURN_TYPE

bool Type::isImplicitlyConvertibleTo(Type type) const {
    switch (typeBase->getKind()) {
        case TypeKind::BasicType:
            return type.isBasicType() && getName() == type.getName();
        case TypeKind::ArrayType:
            return type.isArrayType()
            && (getArraySize() == type.getArraySize() || type.isUnsizedArrayType())
            && getElementType().isImplicitlyConvertibleTo(type.getElementType());
        case TypeKind::RangeType:
            return type.isRangeType() && llvm::cast<RangeType>(typeBase)->elementType
                                      == llvm::cast<RangeType>(type.typeBase)->elementType;
        case TypeKind::TupleType:
            return type.isTupleType() && getSubtypes() == type.getSubtypes();
        case TypeKind::FuncType:
            return type.isFuncType() && getReturnType() == type.getReturnType()
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

bool Type::isUnsigned() const {
    assert(isBasicType());
    llvm::StringRef name = getName();
    return name == "uint" || name == "uint8" || name == "uint16" || name == "uint32" || name == "uint64";
}

llvm::StringRef Type::getName() const { return llvm::cast<BasicType>(typeBase)->name; }
Type Type::getElementType() const { return llvm::cast<ArrayType>(typeBase)->elementType; }
int Type::getArraySize() const { return llvm::cast<ArrayType>(typeBase)->size; }
llvm::ArrayRef<Type> Type::getSubtypes() const { return llvm::cast<TupleType>(typeBase)->subtypes; }
Type Type::getReturnType() const { return llvm::cast<FuncType>(typeBase)->returnType; }
llvm::ArrayRef<Type> Type::getParamTypes() const { return llvm::cast<FuncType>(typeBase)->paramTypes; }
Type Type::getPointee() const { return llvm::cast<PtrType>(typeBase)->pointeeType; }
Type Type::getReferee() const { assert(isRef()); return getPointee(); }
bool Type::isRef() const { return llvm::cast<PtrType>(typeBase)->ref; }

Type Type::getIterableElementType() const {
    assert(isIterable());
    assert(isRangeType() && "non-range iterables not supported yet");
    return llvm::cast<RangeType>(typeBase)->elementType;
}

bool delta::operator==(Type lhs, Type rhs) {
    if (lhs.isMutable() != rhs.isMutable()) return false;
    switch (lhs.getKind()) {
        case TypeKind::BasicType:
            return rhs.isBasicType() && lhs.getName() == rhs.getName();
        case TypeKind::ArrayType:
            return rhs.isArrayType() && lhs.getElementType() == rhs.getElementType()
                                     && lhs.getArraySize() == rhs.getArraySize();
        case TypeKind::RangeType:
            return rhs.isRangeType() && llvm::cast<RangeType>(lhs.get())->elementType
                                     == llvm::cast<RangeType>(rhs.get())->elementType;
        case TypeKind::TupleType:
            return rhs.isTupleType() && lhs.getSubtypes() == rhs.getSubtypes();
        case TypeKind::FuncType:
            return rhs.isFuncType() && lhs.getReturnType() == rhs.getReturnType()
                                    && lhs.getParamTypes() == rhs.getParamTypes();
        case TypeKind::PtrType:
            return rhs.isPtrType() && lhs.isRef() == rhs.isRef() && lhs.getPointee() == rhs.getPointee();
    }
}

bool delta::operator!=(Type lhs, Type rhs) {
    return !(lhs == rhs);
}

void Type::printTo(std::ostream& stream, bool omitTopLevelMutable) const {
    switch (typeBase->getKind()) {
        case TypeKind::BasicType:
            if (isMutable() && !omitTopLevelMutable) stream << "mutable ";
            stream << getName();
            break;
        case TypeKind::ArrayType:
            getElementType().printTo(stream, omitTopLevelMutable);
            stream << "[";
            if (!isUnsizedArrayType()) stream << getArraySize();
            stream << "]";
            break;
        case TypeKind::RangeType:
            stream << "Range<";
            llvm::cast<RangeType>(typeBase)->elementType.printTo(stream, true);
            stream << ">";
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
            getReturnType().printTo(stream, true);
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

std::string Type::toString() const {
    std::ostringstream stream;
    printTo(stream, false);
    return stream.str();
}

std::ostream& delta::operator<<(std::ostream& stream, Type type) {
    type.printTo(stream, true);
    return stream;
}

llvm::raw_ostream& delta::operator<<(llvm::raw_ostream& stream, Type type) {
    std::ostringstream stringstream;
    type.printTo(stringstream, true);
    return stream << stringstream.str();
}
