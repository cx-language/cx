#pragma once

#include <cstdint>
#include <optional>
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
#include "arena.h"

namespace cx {

struct ParamDecl;
struct Module;
struct TypeDecl;
struct DestructorDecl;
struct AnonymousStructElement;
struct GenericArg;
struct Expr;

enum class PointerKind {
    Pointer, // T*: an explicit, storable pointer. Formed with '&'.
    Reference, // T&: an implicit, non-storable borrow. Only valid as a function parameter type.
};

enum class TypeKind {
    BasicType,
    ArrayPointerType,
    AnonymousStructType,
    FunctionType,
    PointerType,
    UnresolvedType, // Placeholder for unresolved generic parameters
};

struct TypeBase {
    virtual ~TypeBase() = 0;

    const TypeKind kind;
    // Earliest-created base with equal structure ignoring spelling (null when none).
    // Spelling twins share one identity; see Type::canonicalTwin().
    TypeBase* firstTwin = nullptr;
    // Creation-order index, unique per base. Only mangling uses it, to tell
    // apart anonymous C types, which have no name to mangle.
    size_t identityIndex = 0;

protected:
    TypeBase(TypeKind kind) : kind(kind) {}
};

inline TypeBase::~TypeBase() {}

struct Type {
    TypeBase& operator*() const { return *typeBase; }
    explicit operator bool() const { return typeBase != nullptr; }
    // Relocating drops the old span unless the caller passes the new end: a stale
    // end at a new start would underline garbage, while an invalid end degrades
    // to today's single caret.
    Type withLocation(Location location, Location endLocation = Location()) const { return Type(typeBase, location, endLocation, aliasSpelling); }

    // TODO: Remove 'Type' suffix from these methods
    bool isBasicType() const { return getKind() == TypeKind::BasicType; }
    // Matches the stdlib Array declaration by name; user code must not declare its own Array type.
    bool isFixedArray() const { return isBasicType() && getName() == "Array" && getGenericArgs().size() == 2; }
    bool isArrayType() const { return getKind() == TypeKind::ArrayPointerType || isFixedArray(); }
    bool isRangeType() const { return isBasicType() && (getName() == "Range" || getName() == "ClosedRange"); }
    bool isAnonymousStructType() const { return getKind() == TypeKind::AnonymousStructType; }
    bool isFunctionType() const { return getKind() == TypeKind::FunctionType; }
    bool isPointerType() const { return getKind() == TypeKind::PointerType; }
    // Raw pointers, borrows, and indexable views.
    bool isPointerOrArrayPointer() const { return isPointerType() || isArrayPointer(); }
    bool isReferenceType() const;
    bool isImplementedAsPointer() const;
    bool isUnresolvedType() const { return getKind() == TypeKind::UnresolvedType; }
    bool isOptionalType() const { return isBasicType() && getName() == "Optional"; }
    bool isBuiltinType() const { return (isBasicType() && isBuiltinScalar(getName())) || isPointerType() || isNull() || isVoid(); }
    bool isImplicitlyCopyable() const;
    bool needsDestruction() const;
    bool isConcreteArray() const;
    bool isSlice() const;
    bool isArrayPointer() const;
    bool isFloatingPoint() const { return isFloat32() || isFloat64() || isFloat80() || isCFloat() || isCDouble(); }
    bool isEnumType() const;
    bool isIterable() const { return isRangeType(); }
    bool isIncrementable() const { return isInteger() || isFloatingPoint() || isArrayPointer(); }
    bool isDecrementable() const { return isInteger() || isFloatingPoint() || isArrayPointer(); }
    bool isVoid() const;
    bool isBool() const;
    bool isInt8() const;
    bool isInt16() const;
    bool isInt32() const;
    bool isInt64() const;
    bool isUInt8() const;
    bool isUInt16() const;
    bool isUInt32() const;
    bool isUInt64() const;
    bool isCSizeT() const;
    bool isCSChar() const;
    bool isCUChar() const;
    bool isCShort() const;
    bool isCUShort() const;
    bool isCInt() const;
    bool isCUInt() const;
    bool isCLong() const;
    bool isCULong() const;
    bool isCLongLong() const;
    bool isCULongLong() const;
    bool isCFloat() const;
    bool isCDouble() const;
    bool isFloat32() const;
    bool isFloat64() const;
    bool isFloat80() const;
    bool isChar() const;
    bool isNull() const;
    bool isUndefined() const;
    bool isNeverType() const { return isBasicType() && getName() == "never"; }
    bool isString() const { return isBasicType() && getName() == "string"; }
    bool isBorrowOrOptionalBorrow() const { return isReferenceType() || (isOptionalType() && getWrappedType().isReferenceType()); }

    Type resolve(const llvm::StringMap<GenericArg>& replacements) const;
    bool isInteger() const;
    bool isSigned() const;
    bool isUnsigned() const;
    bool isSignedInteger() const { return isInteger() && isSigned(); }
    bool isUnsignedInteger() const { return isInteger() && isUnsigned(); }
    int getIntegerBitWidth() const;
    // Size in bytes for types with target-independent layout, null otherwise.
    std::optional<uint64_t> getSizeInBytes() const;
    Type getPointerTo() const;
    Type removePointer() const { return isPointerType() ? getPointee() : *this; }
    Type removeReference() const { return isReferenceType() ? getPointee() : *this; }
    Type removeOptional() const { return isOptionalType() ? getWrappedType() : *this; }
    TypeKind getKind() const { return typeBase->kind; }
    bool isClosureType() const;
    llvm::ArrayRef<Type> getClosureParamTypes() const;
    Type getClosureReturnType() const;
    TypeDecl* getDecl() const;
    // Earliest-created structural twin with spelling dropped. Identity-keyed maps
    // and hashes must use this; all twins agree on the answer.
    Type canonicalTwin() const;
    DestructorDecl* getDestructor() const;
    bool containsUnresolvedPlaceholder() const;
    bool containsReference() const;
    bool storesBorrow() const;
    void printTo(std::ostream& stream, bool canonical = false) const;
    std::string toString() const;
    std::string toCanonicalString() const;

    llvm::StringRef getName() const;
    std::string getQualifiedTypeName() const;
    std::string getDisplayName() const;
    Type getElementType() const;
    int64_t getArraySize() const;
    llvm::StringRef getArraySizeParam() const;
    // A "sizeof(T)" array size is symbolic like a size parameter, but folds to an
    // integer once the operand type is known instead of substituting a value.
    // The marker name is unspellable (sizeof is a keyword), so user
    // declarations can never collide with it.
    bool isSizeofMarker() const;
    Type getSizeofOperand() const;
    bool hasSizeofArraySize() const;
    Type getSizeofArrayOperand() const;
    static Type getSizeofMarker(Type operand);
    bool hasDeferredArraySize() const;
    Expr* getDeferredArraySize() const;
    Module* getDeferredArraySizeHome() const;
    llvm::ArrayRef<AnonymousStructElement> getAnonymousStructElements() const;
    llvm::ArrayRef<GenericArg> getGenericArgs() const;
    Type getReturnType() const;
    llvm::ArrayRef<Type> getParamTypes() const;
    Type getPointee() const;
    PointerKind getPointerKind() const;
    Type getWrappedType() const;

    static Type getVoid(Location location = Location());
    static Type getBool(Location location = Location());
    static Type getInt8(Location location = Location());
    static Type getInt16(Location location = Location());
    static Type getInt32(Location location = Location());
    static Type getInt64(Location location = Location());
    static Type getUInt8(Location location = Location());
    static Type getUInt16(Location location = Location());
    static Type getUInt32(Location location = Location());
    static Type getUInt64(Location location = Location());
    static Type getCSizeT(Location location = Location());
    static Type getCSChar(Location location = Location());
    static Type getCUChar(Location location = Location());
    static Type getCShort(Location location = Location());
    static Type getCUShort(Location location = Location());
    static Type getCInt(Location location = Location());
    static Type getCUInt(Location location = Location());
    static Type getCLong(Location location = Location());
    static Type getCULong(Location location = Location());
    static Type getCLongLong(Location location = Location());
    static Type getCULongLong(Location location = Location());
    static Type getCFloat(Location location = Location());
    static Type getCDouble(Location location = Location());
    // TODO: Return correct uintptr type by checking target platform pointer size.
    static Type getUIntPtr(Location location = Location()) { return getUInt64(location); }
    static Type getFloat32(Location location = Location());
    static Type getFloat64(Location location = Location());
    static Type getFloat80(Location location = Location());
    static Type getChar(Location location = Location());
    static Type getNull(Location location = Location());
    static Type getUndefined(Location location = Location());

    static bool isBuiltinScalar(llvm::StringRef typeName);
    // Returns " (did you mean 'X'?)" for the closest builtin scalar type name,
    // or "" when nothing is close enough.
    static std::string didYouMeanBuiltin(llvm::StringRef typeName);

    TypeBase* typeBase = nullptr;
    // TODO: Add a dedicated class hierarchy for storing source locations with types, like TypeLoc in Clang and Swift.
    Location location;
    // End of this type's source spelling; invalid when synthesized or relocated.
    Location endLocation;
    // Alias name used at this type's source spelling, for diagnostics. Empty when spelled
    // canonically. Ignored by equality; identity strings use toCanonicalString() instead.
    llvm::StringRef aliasSpelling;
};

// A generic argument: either a type or an integer value.
struct GenericArg {
    enum class Kind { Null, Type, Int };

    GenericArg() : kind(Kind::Null) {}
    GenericArg(Type t) : kind(Kind::Type), location(t.location), endLocation(t.endLocation) { type = t; }
    static GenericArg fromInt(int64_t intValue, Location location, Location endLocation = Location()) {
        GenericArg arg;
        arg.kind = Kind::Int;
        arg.intValue = intValue;
        arg.location = location;
        arg.endLocation = endLocation;
        return arg;
    }
    explicit operator bool() const { return kind != Kind::Null; }
    bool isType() const { return kind == Kind::Type; }
    bool isInt() const { return kind == Kind::Int; }
    Type& getType() {
        ASSERT(isType());
        return type;
    }
    const Type& getType() const {
        ASSERT(isType());
        return type;
    }
    int64_t getInt() const {
        ASSERT(isInt());
        return intValue;
    }
    std::string toString() const;
    std::string toCanonicalString() const;
    GenericArg resolve(const llvm::StringMap<GenericArg>& replacements) const;

    Kind kind = Kind::Null;
    union {
        Type type;
        int64_t intValue;
    };
    Location location;
    Location endLocation;
};

bool operator==(const GenericArg&, const GenericArg&);

void appendGenericArgs(std::string& typeName, llvm::ArrayRef<GenericArg> genericArgs);
std::string getQualifiedTypeName(llvm::StringRef typeName, llvm::ArrayRef<GenericArg> genericArgs);
// Display variant of getQualifiedTypeName using source spellings. Diagnostics and
// IDE hover only; never for lookup, mangling, or symbol keys.
std::string getDisplayTypeName(llvm::StringRef typeName, llvm::ArrayRef<GenericArg> genericArgs);

struct BasicType : TypeBase {
    std::string getQualifiedName() const { return getQualifiedTypeName(name, genericArgs); }
    static Type get(llvm::StringRef name, llvm::ArrayRef<GenericArg> genericArgs, Location location = Location(), Location endLocation = Location());
    static Type getArray(Type elementType, int64_t size, Location location = Location(), Location endLocation = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::BasicType; }

private:
    BasicType(llvm::StringRef name, std::vector<GenericArg>&& genericArgs)
    : TypeBase(TypeKind::BasicType), name(internString(name)), genericArgs(std::move(genericArgs)), decl(nullptr) {}

public:
    llvm::StringRef name; // Can be empty for anonymous types imported from C.
    std::vector<GenericArg> genericArgs;
    TypeDecl* decl;
};

struct ArrayPointerType : TypeBase {
    static const int64_t UnknownSize = -1;
    static Type get(Type elementType, Location location = Location(), Location endLocation = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::ArrayPointerType; }

private:
    explicit ArrayPointerType(Type elementType) : TypeBase(TypeKind::ArrayPointerType), elementType(elementType) {}

public:
    Type elementType;
};

struct AnonymousStructElement {
    AnonymousStructElement(llvm::StringRef name, Type type) : name(internString(name)), type(type) {}

    llvm::StringRef name;
    Type type;
};

bool operator==(const AnonymousStructElement&, const AnonymousStructElement&);

struct AnonymousStructType : TypeBase {
    static Type get(std::vector<AnonymousStructElement>&& elements, Location location = Location(), Location endLocation = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::AnonymousStructType; }

private:
    AnonymousStructType(std::vector<AnonymousStructElement>&& elements) : TypeBase(TypeKind::AnonymousStructType), elements(std::move(elements)) {}

public:
    std::vector<AnonymousStructElement> elements;
};

struct FunctionType : TypeBase {
    std::vector<ParamDecl> getParamDecls(Location location = Location()) const;
    static Type get(Type returnType, std::vector<Type>&& paramTypes, bool isVariadic, Location location = Location(), Location endLocation = Location());
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
    static Type get(Type pointeeType, PointerKind kind = PointerKind::Pointer, Location location = Location(), Location endLocation = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::PointerType; }

private:
    PointerType(Type pointeeType, PointerKind kind) : TypeBase(TypeKind::PointerType), pointeeType(pointeeType), pointerKind(kind) {}

public:
    Type pointeeType;
    PointerKind pointerKind;
};

namespace OptionalType {
Type get(Type wrappedType, Location location = Location());
};

struct UnresolvedType : TypeBase {
    static Type get(Location location = Location());
    static Type getDeferredSize(Expr* sizeExpr, Module* homeModule, Location location, Location endLocation = Location());
    static bool classof(const TypeBase* t) { return t->kind == TypeKind::UnresolvedType; }

    // Array-size expression awaiting sema folding (null for plain placeholders).
    // Unresolved types never intern together, so each occurrence keeps its own.
    Expr* deferredSize = nullptr;
    // Module whose globals the size expression resolves against. Folding can
    // run under another module (e.g. a stdlib instantiation triggered by this
    // type), so the scope travels with the node instead of the checker.
    Module* homeModule = nullptr;

private:
    UnresolvedType() : TypeBase(TypeKind::UnresolvedType) {}
};

bool operator==(Type, Type);
bool operator!=(Type, Type);
std::ostream& operator<<(std::ostream&, Type);
llvm::raw_ostream& operator<<(llvm::raw_ostream&, Type);
inline std::ostream& operator<<(std::ostream& stream, GenericArg arg) {
    return stream << arg.toString();
}
inline llvm::raw_ostream& operator<<(llvm::raw_ostream& stream, GenericArg arg) {
    return stream << arg.toString();
}

} // namespace cx
