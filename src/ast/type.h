#pragma once

#include <vector>
#include <string>
#include <ostream>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>

namespace delta {

enum class TypeKind {
    BasicType,
    ArrayType,
    TupleType,
    FuncType,
    PtrType,
};

class TypeBase {
public:
    virtual ~TypeBase() = 0;
    TypeKind getKind() const { return kind; }

protected:
    TypeBase(TypeKind kind) : kind(kind) { }
    
private:
    const TypeKind kind;
};

inline TypeBase::~TypeBase() { }

struct Type {
public:
    Type() = default;
    Type(const TypeBase* typeBase, bool isMutable = false)
    : typeBase(typeBase), mutableFlag(isMutable) { }
    const TypeBase& operator*() const { return *typeBase; }
    explicit operator bool() const { return typeBase != nullptr; }
    const TypeBase* get() const { return typeBase; }

    bool isBasicType() const { return getKind() == TypeKind::BasicType; }
    bool isArrayType() const { return getKind() == TypeKind::ArrayType; }
    bool isTupleType() const { return getKind() == TypeKind::TupleType; }
    bool isFuncType() const { return getKind() == TypeKind::FuncType; }
    bool isPtrType() const { return getKind() == TypeKind::PtrType; }
    bool isFloatingPoint() const {
        return isFloat() || isFloat32() || isFloat64() || isFloat80();
    }
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
    bool isChar() const;
    bool isNull() const;

    void appendType(Type);
    bool isImplicitlyConvertibleTo(Type) const;
    bool isInteger() const { return isSigned() || isUnsigned(); }
    bool isSigned() const;
    bool isUnsigned() const;
    bool isMutable() const { return mutableFlag; }
    void setMutable(bool m) { mutableFlag = m; }
    TypeKind getKind() const { return typeBase->getKind(); }
    void printTo(std::ostream& stream, bool omitTopLevelMutable) const;
    std::string toString() const;

    llvm::StringRef getName() const;
    Type getElementType() const;
    int getArraySize() const;
    llvm::ArrayRef<Type> getSubtypes() const;
    Type getReturnType() const;
    llvm::ArrayRef<Type> getParamTypes() const;
    Type getPointee() const;
    Type getReferee() const;
    bool isRef() const;

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
    static Type getChar(bool isMutable = false);
    static Type getNull(bool isMutable = false);

    static bool isBuiltinScalar(llvm::StringRef typeName);

private:
    const TypeBase* typeBase;
    bool mutableFlag;
};

class BasicType : public TypeBase {
public:
    std::string name;
    static Type get(llvm::StringRef name, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::BasicType; }

private:
    friend Type;
    BasicType(llvm::StringRef name)
    : TypeBase(TypeKind::BasicType), name(name) { }
};

class ArrayType : public TypeBase {
public:
    Type elementType;
    int64_t size;
    static Type get(Type type, int64_t size, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::ArrayType; }

private:
    ArrayType(Type type, int64_t size)
    : TypeBase(TypeKind::ArrayType), elementType(type), size(size) { }
};

class TupleType : public TypeBase {
public:
    std::vector<Type> subtypes;
    static Type get(std::vector<Type>&& subtypes, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::TupleType; }

private:
    TupleType(std::vector<Type>&& subtypes)
    : TypeBase(TypeKind::TupleType), subtypes(std::move(subtypes)) {
        assert(this->subtypes.size() != 1);
    }
};

class FuncType : public TypeBase {
public:
    Type returnType;
    std::vector<Type> paramTypes;
    static Type get(Type returnType, std::vector<Type>&& paramTypes, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::FuncType; }

private:
    FuncType(Type returnType, std::vector<Type>&& paramTypes)
    : TypeBase(TypeKind::FuncType), returnType(returnType), paramTypes(std::move(paramTypes)) { }
};

class PtrType : public TypeBase {
public:
    Type pointeeType;
    bool ref;
    static Type get(Type pointeeType, bool isReference, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::PtrType; }

private:
    PtrType(Type type, bool ref) : TypeBase(TypeKind::PtrType), pointeeType(type), ref(ref) { }
};

bool operator==(Type, Type);
bool operator!=(Type, Type);
std::ostream& operator<<(std::ostream&, Type);

}
