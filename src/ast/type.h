#pragma once

#include <ostream>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#pragma warning(pop)
#include "../support/utility.h"

namespace cx {

struct ParamDecl;
struct TypeDecl;
struct DestructorDecl;
struct TupleElement;

enum class Mutability { Mutable, Const };

enum class TypeKind {
    BasicType,
    ArrayType,
    TupleType,
    FunctionType,
    PointerType,
    UnresolvedType, // Placeholder for unresolved generic parameters
};

struct TypeBase {
    virtual ~TypeBase() = 0;

    const TypeKind kind;

protected:
    TypeBase(TypeKind kind) : kind(kind) {}
};

inline TypeBase::~TypeBase() {}

struct Type {
    TypeBase& operator*() const { return *typeBase; }
    explicit operator bool() const { return typeBase != nullptr; }
    Type withLocation(Location location) const { return Type(typeBase, mutability, location); }

    // TODO: Remove 'Type' suffix from these methods
    bool isBasicType() const { return getKind() == TypeKind::BasicType; }
    bool isArrayType() const { return getKind() == TypeKind::ArrayType; }
    bool isRangeType() const { return isBasicType() && (getName() == "Range" || getName() == "ClosedRange"); }
    bool isTupleType() const { return getKind() == TypeKind::TupleType; }
    bool isFunctionType() const { return getKind() == TypeKind::FunctionType; }
    bool isPointerType() const { return getKind() == TypeKind::PointerType; }
    bool isImplementedAsPointer() const;
    bool isUnresolvedType() const { return getKind() == TypeKind::UnresolvedType; }
    bool isOptionalType() const { return isBasicType() && getName() == "Optional"; }
    bool isBuiltinType() const { return (isBasicType() && isBuiltinScalar(getName())) || isPointerType() || isNull() || isVoid(); }
    bool isImplicitlyCopyable() const;
    bool isConstantArray() const;
    bool isArrayRef() const;
    bool isUnsizedArrayPointer() const;
    bool isFloatingPoint() const { return isFloat() || isFloat16() || isFloat32() || isFloat64() || isFloat80(); }
    bool isEnumType() const;
    bool isIterable() const { return isRangeType(); }
    bool isIncrementable() const { return isInteger() || isFloatingPoint() || isUnsizedArrayPointer(); }
    bool isDecrementable() const { return isInteger() || isFloatingPoint() || isUnsizedArrayPointer(); }
    bool isVoid() const;
    bool isBool() const;
    bool isInt() const;
    bool isInt8() const;
    bool isInt16() const;
    bool isInt32() const;
    bool isInt64() const;
    bool isInt128() const;
    bool isUInt() const;
    bool isUInt8() const;
    bool isUInt16() const;
    bool isUInt32() const;
    bool isUInt64() const;
    bool isUInt128() const;
    bool isFloat() const;
    bool isFloat16() const;
    bool isFloat32() const;
    bool isFloat64() const;
    bool isFloat80() const;
    bool isChar() const;
    bool isNull() const;
    bool isUndefined() const;
    bool isNeverType() const { return isBasicType() && getName() == "never"; }

    Type resolve(const llvm::StringMap<Type>& replacements) const;
    bool isInteger() const;
    bool isSigned() const;
    bool isUnsigned() const;
    bool isSignedInteger() const { return isInteger() && isSigned(); }
    bool isUnsignedInteger() const { return isInteger() && isUnsigned(); }
    int getIntegerBitWidth() const;
    bool isMutable() const { return mutability == Mutability::Mutable; }
    Type withMutability(Mutability m) const { return Type(typeBase, m, location); }
    Type getPointerTo() const;
    Type removePointer() const { return isPointerType() ? getPointee() : *this; }
    Type removeOptional() const { return isOptionalType() ? getWrappedType() : *this; }
    TypeKind getKind() const { return typeBase->kind; }
    TypeDecl* getDecl() const;
    DestructorDecl* getDestructor() const;
    bool equalsIgnoreTopLevelMutable(Type) const;
    bool containsUnresolvedPlaceholder() const;
    void printTo(std::ostream& stream) const;
    std::string toString() const;

    llvm::StringRef getName() const;
    std::string getQualifiedTypeName() const;
    Type getElementType() const;
    int64_t getArraySize() const;
    llvm::ArrayRef<TupleElement> getTupleElements() const;
    llvm::ArrayRef<Type> getGenericArgs() const;
    Type getReturnType() const;
    llvm::ArrayRef<Type> getParamTypes() const;
    Type getPointee() const;
    Type getWrappedType() const;

    static Type getVoid(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getBool(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getInt(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getInt8(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getInt16(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getInt32(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getInt64(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getInt128(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getUInt(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getUInt8(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getUInt16(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getUInt32(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getUInt64(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getUInt128(Mutability mutability = Mutability::Mutable, Location location = Location());
    // TODO: Return correct uintptr type by checking target platform pointer size.
    static Type getUIntPtr(Mutability mutability = Mutability::Mutable, Location location = Location()) { return getUInt64(mutability, location); }
    static Type getFloat(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getFloat16(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getFloat32(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getFloat64(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getFloat80(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getChar(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getNull(Mutability mutability = Mutability::Mutable, Location location = Location());
    static Type getUndefined(Mutability mutability = Mutability::Mutable, Location location = Location());

    static bool isBuiltinScalar(llvm::StringRef typeName);

    TypeBase* typeBase = nullptr;
    Mutability mutability = Mutability::Mutable;
    // TODO: Add a dedicated class hierarchy for storing source locations with types, like TypeLoc in Clang and Swift.
    Location location;
};

void appendGenericArgs(std::string& typeName, llvm::ArrayRef<Type> genericArgs);
std::string getQualifiedTypeName(llvm::StringRef typeName, llvm::ArrayRef<Type> genericArgs);

struct BasicType : TypeBase {
    std::string getQualifiedName() const { return getQualifiedTypeName(name, genericArgs); }
    static Type get(llvm::StringRef name, llvm::ArrayRef<Type> genericArgs, Mutability mutability = Mutability::Mutable, Location location = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::BasicType; }

private:
    BasicType(llvm::StringRef name, std::vector<Type>&& genericArgs)
    : TypeBase(TypeKind::BasicType), name(name), genericArgs(std::move(genericArgs)), decl(nullptr) {}

public:
    std::string name; // Can be empty for anonymous types imported from C.
    std::vector<Type> genericArgs;
    TypeDecl* decl;
};

struct ArrayType : TypeBase {
    static Type getIndexType() { return Type::getInt(); }
    static const int64_t UnknownSize = -1;
    static Type get(Type type, int64_t size, Location location = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::ArrayType; }

private:
    ArrayType(Type type, int64_t size) : TypeBase(TypeKind::ArrayType), elementType(type), size(size) {}

public:
    Type elementType;
    int64_t size;
};

struct TupleElement {
    std::string name;
    Type type;
};

bool operator==(const TupleElement&, const TupleElement&);

struct TupleType : TypeBase {
    static Type get(std::vector<TupleElement>&& elements, Mutability mutability = Mutability::Mutable, Location location = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::TupleType; }

private:
    TupleType(std::vector<TupleElement>&& elements) : TypeBase(TypeKind::TupleType), elements(std::move(elements)) {}

public:
    std::vector<TupleElement> elements;
};

struct FunctionType : TypeBase {
    std::vector<ParamDecl> getParamDecls(Location location = Location()) const;
    static Type get(Type returnType, std::vector<Type>&& paramTypes, bool isVariadic, Mutability mutability = Mutability::Mutable,
                    Location location = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::FunctionType; }

private:
    FunctionType(Type returnType, std::vector<Type>&& paramTypes, bool isVariadic)
    : TypeBase(TypeKind::FunctionType), returnType(returnType), paramTypes(std::move(paramTypes)), isVariadic(isVariadic) {}

public:
    Type returnType;
    std::vector<Type> paramTypes;

public:
    bool isVariadic = false;
};

struct PointerType : TypeBase {
    static Type get(Type pointeeType, Mutability mutability = Mutability::Mutable, Location location = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::PointerType; }

private:
    PointerType(Type pointeeType) : TypeBase(TypeKind::PointerType), pointeeType(pointeeType) {}

public:
    Type pointeeType;
};

namespace OptionalType {
Type get(Type wrappedType, Mutability mutability = Mutability::Mutable, Location location = Location());
};

struct UnresolvedType : TypeBase {
    static Type get(Mutability mutability = Mutability::Mutable, Location location = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::UnresolvedType; }

private:
    UnresolvedType() : TypeBase(TypeKind::UnresolvedType) {}
};

bool operator==(Type, Type);
bool operator!=(Type, Type);
std::ostream& operator<<(std::ostream&, Type);
llvm::raw_ostream& operator<<(llvm::raw_ostream&, Type);

} // namespace cx
