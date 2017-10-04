#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include "../support/utility.h"

namespace delta {

class TypeResolver;

enum class TypeKind {
    BasicType,
    ArrayType,
    TupleType,
    FunctionType,
    PointerType,
};

class TypeBase {
public:
    virtual ~TypeBase() = 0;
    TypeKind getKind() const { return kind; }

protected:
    TypeBase(TypeKind kind) : kind(kind) {}

private:
    const TypeKind kind;
};

inline TypeBase::~TypeBase() {}

struct Type {
public:
    Type(const TypeBase* typeBase = nullptr, bool isMutable = false)
    : typeBase(typeBase), mutableFlag(isMutable) {}
    const TypeBase& operator*() const { return *typeBase; }
    explicit operator bool() const { return typeBase != nullptr; }
    const TypeBase* get() const { return typeBase; }

    bool isBasicType() const { return getKind() == TypeKind::BasicType; }
    bool isArrayType() const { return getKind() == TypeKind::ArrayType; }
    bool isRangeType() const { return isBasicType() && (getName() == "Range" || getName() == "ClosedRange"); }
    bool isTupleType() const { return getKind() == TypeKind::TupleType; }
    bool isFunctionType() const { return getKind() == TypeKind::FunctionType; }
    bool isPointerType() const { return getKind() == TypeKind::PointerType; }
    bool isBuiltinType() const {
        return (isBasicType() && isBuiltinScalar(getName())) || isPointerType() || isNull();
    }
    bool isSizedArrayType() const;
    bool isUnsizedArrayType() const;
    bool isNullablePointer() const;
    bool isFloatingPoint() const {
        return isFloat() || isFloat32() || isFloat64() || isFloat80();
    }
    bool isIterable() const { return isRangeType(); }
    bool isVoid() const;
    bool isBool() const;
    bool isInt() const;
    bool isInt8() const;
    bool isInt16() const;
    bool isInt32() const;
    bool isInt64() const;
    bool isUInt() const;
    bool isUInt8() const;
    bool isUInt16() const;
    bool isUInt32() const;
    bool isUInt64() const;
    bool isFloat() const;
    bool isFloat32() const;
    bool isFloat64() const;
    bool isFloat80() const;
    bool isString() const;
    bool isChar() const;
    bool isNull() const;

    void appendType(Type);
    bool isImplicitlyConvertibleTo(Type) const;
    bool isInteger() const { return isSigned() || isUnsigned(); }
    bool isSigned() const;
    bool isUnsigned() const;
    bool isMutable() const { return mutableFlag; }
    void setMutable(bool isMutable);
    Type asMutable(bool isMutable = true) const { return Type(typeBase, isMutable); }
    Type asImmutable() const { return asMutable(false); }
    Type removePointer() const { return isPointerType() ? getPointee() : *this; }
    TypeKind getKind() const { return typeBase->getKind(); }
    void printTo(std::ostream& stream, bool omitTopLevelMutable) const;
    std::string toString() const;

    llvm::StringRef getName() const;
    Type getElementType() const;
    int64_t getArraySize() const;
    llvm::ArrayRef<Type> getSubtypes() const;
    llvm::ArrayRef<Type> getGenericArgs() const;
    Type getReturnType() const;
    llvm::ArrayRef<Type> getParamTypes() const;
    Type getPointee() const;
    Type getReferee() const;
    bool isReference() const;
    Type getIterableElementType() const;

    static Type getVoid(bool isMutable = false);
    static Type getBool(bool isMutable = false);
    static Type getInt(bool isMutable = false);
    static Type getInt8(bool isMutable = false);
    static Type getInt16(bool isMutable = false);
    static Type getInt32(bool isMutable = false);
    static Type getInt64(bool isMutable = false);
    static Type getUInt(bool isMutable = false);
    static Type getUInt8(bool isMutable = false);
    static Type getUInt16(bool isMutable = false);
    static Type getUInt32(bool isMutable = false);
    static Type getUInt64(bool isMutable = false);
    static Type getFloat(bool isMutable = false);
    static Type getFloat32(bool isMutable = false);
    static Type getFloat64(bool isMutable = false);
    static Type getFloat80(bool isMutable = false);
    static Type getString(bool isMutable = false);
    static Type getChar(bool isMutable = false);
    static Type getNull(bool isMutable = false);

    static bool isBuiltinScalar(llvm::StringRef typeName);

private:
    const TypeBase* typeBase;
    bool mutableFlag;
};

class BasicType : public TypeBase {
public:
    llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }
    llvm::StringRef getName() const { return name; }
    static Type get(llvm::StringRef name, llvm::ArrayRef<Type> genericArgs,
                    bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::BasicType; }

private:
    friend Type;
    BasicType(llvm::StringRef name, std::vector<Type>&& genericArgs)
    : TypeBase(TypeKind::BasicType), name(name), genericArgs(std::move(genericArgs)) {}

private:
    std::string name;
    std::vector<Type> genericArgs;
};

class ArrayType : public TypeBase {
public:
    Type getElementType() const { return elementType; }
    int64_t getSize() const { return size; }
    bool isUnsized() const { return size == unsized; }
    static const int64_t unsized = -1;
    static Type get(Type type, int64_t size, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::ArrayType; }

private:
    ArrayType(Type type, int64_t size)
    : TypeBase(TypeKind::ArrayType), elementType(type), size(size) {}

private:
    Type elementType;
    int64_t size; ///< Equal to ArrayType::unsized if this is an unsized array type.
};

class TupleType : public TypeBase {
public:
    llvm::ArrayRef<Type> getSubtypes() const { return subtypes; }
    static Type get(std::vector<Type>&& subtypes, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::TupleType; }

private:
    TupleType(std::vector<Type>&& subtypes)
    : TypeBase(TypeKind::TupleType), subtypes(std::move(subtypes)) {
        ASSERT(getSubtypes().size() != 1);
    }

private:
    std::vector<Type> subtypes;
};

class FunctionType : public TypeBase {
public:
    Type getReturnType() const { return returnType; }
    llvm::ArrayRef<Type> getParamTypes() const { return paramTypes; }
    static Type get(Type returnType, std::vector<Type>&& paramTypes, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::FunctionType; }

private:
    FunctionType(Type returnType, std::vector<Type>&& paramTypes)
    : TypeBase(TypeKind::FunctionType), returnType(returnType), paramTypes(std::move(paramTypes)) {}

private:
    Type returnType;
    std::vector<Type> paramTypes;
};

class PointerType : public TypeBase {
public:
    Type getPointeeType() const { return pointeeType; }
    bool isReference() const { return reference; }
    static Type get(Type pointeeType, bool isReference, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::PointerType; }

private:
    PointerType(Type pointeeType, bool isReference)
    : TypeBase(TypeKind::PointerType), pointeeType(pointeeType), reference(isReference) {}

private:
    Type pointeeType;
    bool reference;
};

bool operator==(Type, Type);
bool operator!=(Type, Type);
std::ostream& operator<<(std::ostream&, Type);
llvm::raw_ostream& operator<<(llvm::raw_ostream&, Type);

}
