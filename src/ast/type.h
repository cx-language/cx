#pragma once

#include <vector>
#include <string>
#include <ostream>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)
#include "../support/utility.h"

namespace delta {

class ParamDecl;
class TypeDecl;
class DeinitDecl;
class TupleElement;

enum class TypeKind {
    BasicType,
    ArrayType,
    TupleType,
    FunctionType,
    PointerType,
    OptionalType
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
    Type(TypeBase* typeBase = nullptr, bool isMutable = false)
    : typeBase(typeBase), mutableFlag(isMutable) {}
    TypeBase& operator*() const { return *typeBase; }
    explicit operator bool() const { return typeBase != nullptr; }
    TypeBase* getBase() const { return typeBase; }

    bool isBasicType() const { return getKind() == TypeKind::BasicType; }
    bool isArrayType() const { return getKind() == TypeKind::ArrayType; }
    bool isRangeType() const { return isBasicType() && (getName() == "Range" || getName() == "ClosedRange"); }
    bool isTupleType() const { return getKind() == TypeKind::TupleType; }
    bool isFunctionType() const { return getKind() == TypeKind::FunctionType; }
    bool isPointerType() const { return getKind() == TypeKind::PointerType; }
    bool isOptionalType() const { return getKind() == TypeKind::OptionalType; }
    bool isBuiltinType() const {
        return (isBasicType() && isBuiltinScalar(getName())) || isPointerType() || isOptionalType()
            || isNull() || isVoid();
    }
    bool isArrayWithConstantSize() const;
    bool isArrayWithRuntimeSize() const;
    bool isArrayWithUnknownSize() const;
    bool isFloatingPoint() const {
        return isFloat() || isFloat32() || isFloat64() || isFloat80();
    }
    bool isEnumType() const;
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

    Type resolve(const llvm::StringMap<Type>& replacements) const;
    bool isInteger() const { return isBasicType() && (isSigned() || isUnsigned()); }
    bool isSigned() const;
    bool isUnsigned() const;
    bool isMutable() const { return mutableFlag; }
    void setMutable(bool isMutable);
    Type asMutable(bool isMutable = true) const { return Type(typeBase, isMutable); }
    Type asImmutable() const { return asMutable(false); }
    Type removePointer() const { return isPointerType() ? getPointee() : *this; }
    Type removeOptional() const { return isOptionalType() ? getWrappedType() : *this; }
    TypeKind getKind() const { return typeBase->getKind(); }
    TypeDecl* getDecl() const;
    DeinitDecl* getDeinitializer() const;
    void printTo(std::ostream& stream, bool omitTopLevelMutable) const;
    std::string toString(bool omitTopLevelMutable = false) const;

    llvm::StringRef getName() const;
    Type getElementType() const;
    int64_t getArraySize() const;
    llvm::ArrayRef<TupleElement> getTupleElements() const;
    llvm::ArrayRef<Type> getGenericArgs() const;
    Type getReturnType() const;
    llvm::ArrayRef<Type> getParamTypes() const;
    Type getPointee() const;
    Type getWrappedType() const;

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
    // TODO: Return correct uintptr type by checking target platform pointer size.
    static Type getUIntPtr(bool isMutable = false) { return getUInt64(isMutable); }
    static Type getFloat(bool isMutable = false);
    static Type getFloat32(bool isMutable = false);
    static Type getFloat64(bool isMutable = false);
    static Type getFloat80(bool isMutable = false);
    static Type getString(bool isMutable = false);
    static Type getChar(bool isMutable = false);
    static Type getNull(bool isMutable = false);

    static bool isBuiltinScalar(llvm::StringRef typeName);

private:
    TypeBase* typeBase;
    bool mutableFlag;
};

class BasicType : public TypeBase {
public:
    llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }
    llvm::StringRef getName() const { return name; }
    TypeDecl* getDecl() const { return decl; }
    void setDecl(TypeDecl* decl) { this->decl = decl; }
    static Type get(llvm::StringRef name, llvm::ArrayRef<Type> genericArgs, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::BasicType; }

private:
    friend Type;
    BasicType(llvm::StringRef name, std::vector<Type>&& genericArgs)
    : TypeBase(TypeKind::BasicType), name(name), genericArgs(std::move(genericArgs)), decl(nullptr) {}

private:
    std::string name;
    std::vector<Type> genericArgs;
    TypeDecl* decl;
};

class ArrayType : public TypeBase {
public:
    Type getElementType() const { return elementType; }
    int64_t getSize() const { return size; }
    bool hasRuntimeSize() const { return size == runtimeSize; }
    bool hasUnknownSize() const { return size == unknownSize; }
    static Type getIndexType() { return Type::getUInt(); }
    static const int64_t runtimeSize = -1;
    static const int64_t unknownSize = -2;
    static Type get(Type type, int64_t size, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::ArrayType; }

private:
    ArrayType(Type type, int64_t size)
    : TypeBase(TypeKind::ArrayType), elementType(type), size(size) {}

private:
    Type elementType;
    int64_t size; ///< Equal to ArrayType::runtimeSize if values of this type know their size at
                  ///< runtime (i.e. this is an ArrayRef), or ArrayType::unknownSize if values of
                  /// this type never know their size.
};

class TupleElement {
public:
    std::string name;
    Type type;
};

bool operator==(const TupleElement&, const TupleElement&);

class TupleType : public TypeBase {
public:
    llvm::ArrayRef<TupleElement> getElements() const { return elements; }
    static Type get(std::vector<TupleElement>&& elements, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::TupleType; }

private:
    TupleType(std::vector<TupleElement>&& elements)
    : TypeBase(TypeKind::TupleType), elements(std::move(elements)) {}

private:
    std::vector<TupleElement> elements;
};

class FunctionType : public TypeBase {
public:
    Type getReturnType() const { return returnType; }
    llvm::ArrayRef<Type> getParamTypes() const { return paramTypes; }
    std::vector<ParamDecl> getParamDecls(SourceLocation location = SourceLocation::invalid()) const;
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
    static Type get(Type pointeeType, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::PointerType; }

private:
    PointerType(Type pointeeType)
    : TypeBase(TypeKind::PointerType), pointeeType(pointeeType) {}

private:
    Type pointeeType;
};

class OptionalType : public TypeBase {
public:
    Type getWrappedType() const { return wrappedType; }
    static Type get(Type wrappedType, bool isMutable = false);
    static bool classof(const TypeBase* t) { return t->getKind() == TypeKind::OptionalType; }

private:
    OptionalType(Type wrappedType)
    : TypeBase(TypeKind::OptionalType), wrappedType(wrappedType) {}

private:
    Type wrappedType;
};

bool operator==(Type, Type);
bool operator!=(Type, Type);
std::ostream& operator<<(std::ostream&, Type);
llvm::raw_ostream& operator<<(llvm::raw_ostream&, Type);

}
