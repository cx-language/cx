#include "type.h"
#include <sstream>
#pragma warning(push, 0)
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "../support/utility.h"
#include "arena.h"
#include "decl.h"

using namespace cx;

static std::vector<TypeBase*> typeBases;

#define DEFINE_BUILTIN_TYPE_GET_AND_IS(TYPE, NAME) \
    Type Type::get##TYPE(Mutability mutability, Location location) { \
        return BasicType::get(#NAME, {}, mutability, location); \
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
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int128, int128)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt, uint)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt8, uint8)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt16, uint16)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt32, uint32)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt64, uint64)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt128, uint128)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float, float)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float16, float16)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float32, float32)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float64, float64)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float80, float80)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Char, char)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Null, null)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Undefined, undefined)
#undef DEFINE_BUILTIN_TYPE_GET_AND_IS

bool Type::isImplicitlyCopyable() const {
    switch (getKind()) {
    case TypeKind::BasicType:
        return !getDecl() || getDecl()->passByValue();
    case TypeKind::ArrayType:
        return !isConstantArray() || getElementType().isImplicitlyCopyable();
    case TypeKind::TupleType:
        return llvm::all_of(llvm::cast<TupleType>(typeBase)->elements, [&](auto& element) { return element.type.isImplicitlyCopyable(); });
    case TypeKind::FunctionType:
    case TypeKind::PointerType:
        return true;
    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }
    llvm_unreachable("all cases handled");
}

bool Type::isConstantArray() const {
    return isArrayType() && getArraySize() >= 0;
}

bool Type::isArrayRef() const {
    return isBasicType() && getName() == "ArrayRef";
}

bool Type::isUnsizedArrayPointer() const {
    return isArrayType() && getArraySize() == ArrayType::UnknownSize;
}

bool Type::isBuiltinScalar(llvm::StringRef typeName) {
    return llvm::StringSwitch<bool>(typeName)
        .Cases({"int", "int8", "int16", "int32", "int64", "int128"}, true)
        .Cases({"uint", "uint8", "uint16", "uint32", "uint64", "uint128"}, true)
        .Cases({"float", "float16", "float32", "float64", "float80", "bool", "char"}, true)
        .Default(false);
}

bool Type::isEnumType() const {
    if (auto* basicType = llvm::dyn_cast<BasicType>(typeBase)) {
        return basicType->decl && basicType->decl->isEnumDecl();
    }
    return false;
}

Type Type::resolve(const llvm::StringMap<Type>& replacements) const {
    if (!typeBase) return Type(nullptr, mutability, location);

    switch (getKind()) {
    case TypeKind::BasicType: {
        auto it = replacements.find(getName());
        if (it != replacements.end()) {
            // TODO: Handle generic arguments for type placeholders.
            Type resolved = it->second.withMutability(mutability);
            resolved.location = location;
            return resolved;
        }

        auto genericArgs = map(getGenericArgs(), [&](Type t) { return t.resolve(replacements); });
        return BasicType::get(getName(), std::move(genericArgs), mutability, location);
    }
    case TypeKind::ArrayType:
        return ArrayType::get(getElementType().resolve(replacements), getArraySize(), location);

    case TypeKind::TupleType: {
        auto elements = map(getTupleElements(), [&](auto& element) { return TupleElement{element.name, element.type.resolve(replacements)}; });
        return TupleType::get(std::move(elements), mutability, location);
    }
    case TypeKind::FunctionType: {
        auto paramTypes = map(getParamTypes(), [&](Type t) { return t.resolve(replacements); });
        return FunctionType::get(getReturnType().resolve(replacements), std::move(paramTypes), llvm::cast<FunctionType>(typeBase)->isVariadic, mutability,
                                 location);
    }
    case TypeKind::PointerType:
        return PointerType::get(getPointee().resolve(replacements), mutability, location);
    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }
    llvm_unreachable("all cases handled");
}

template<typename T> static Type getType(T&& typeBase, Mutability mutability, Location location) {
    Type newType(&typeBase, mutability, location);

    for (auto* existingTypeBase : typeBases) {
        Type existingType(existingTypeBase, mutability, location);
        if (existingType.equalsIgnoreTopLevelMutable(newType)) {
            return existingType;
        }
    }

    typeBases.push_back(makeAST<T>(std::forward<T>(typeBase)));
    return Type(typeBases.back(), mutability, location);
}

Type BasicType::get(llvm::StringRef name, llvm::ArrayRef<Type> genericArgs, Mutability mutability, Location location) {
    return getType(BasicType(name, genericArgs), mutability, location);
}

Type ArrayType::get(Type elementType, int64_t size, Location location) {
    return getType(ArrayType(elementType, size), elementType.mutability, location);
}

Type TupleType::get(std::vector<TupleElement>&& elements, Mutability mutability, Location location) {
    return getType(TupleType(std::move(elements)), mutability, location);
}

Type FunctionType::get(Type returnType, std::vector<Type>&& paramTypes, bool isVariadic, Mutability mutability, Location location) {
    return getType(FunctionType(returnType, std::move(paramTypes), isVariadic), mutability, location);
}

Type PointerType::get(Type pointeeType, Mutability mutability, Location location) {
    return getType(PointerType(pointeeType), mutability, location);
}

Type OptionalType::get(Type wrappedType, Mutability mutability, Location location) {
    return BasicType::get("Optional", wrappedType, mutability, location);
}

Type UnresolvedType::get(Mutability mutability, Location location) {
    return getType(UnresolvedType(), mutability, location);
}

bool cx::operator==(const TupleElement& a, const TupleElement& b) {
    return a.name == b.name && a.type == b.type;
}

void cx::appendGenericArgs(std::string& typeName, llvm::ArrayRef<Type> genericArgs) {
    if (genericArgs.empty()) return;

    typeName += '<';
    for (const Type& genericArg : genericArgs) {
        typeName += genericArg.toString();
        if (&genericArg != &genericArgs.back()) typeName += ", ";
    }
    typeName += '>';
}

std::string cx::getQualifiedTypeName(llvm::StringRef typeName, llvm::ArrayRef<Type> genericArgs) {
    std::string result = typeName.str();
    appendGenericArgs(result, genericArgs);
    return result;
}

std::vector<ParamDecl> FunctionType::getParamDecls(Location location) const {
    return map(paramTypes, [&](Type paramType) { return ParamDecl(paramType, "", false, location); });
}

constexpr auto signedInts = {"int", "int8", "int16", "int32", "int64"};
constexpr auto unsignedInts = {"uint", "uint8", "uint16", "uint32", "uint64"};

bool Type::isInteger() const {
    if (!isBasicType()) return false;
    return llvm::is_contained(signedInts, getName()) || llvm::is_contained(unsignedInts, getName());
}

bool Type::isSigned() const {
    ASSERT(isInteger());
    return llvm::is_contained(signedInts, getName());
}

bool Type::isUnsigned() const {
    ASSERT(isInteger());
    return llvm::is_contained(unsignedInts, getName());
}

int Type::getIntegerBitWidth() const {
    ASSERT(isInteger());
    return llvm::StringSwitch<int>(getName())
        .Cases({"int", "uint"}, 32)
        .Cases({"int8", "uint8"}, 8)
        .Cases({"int16", "uint16"}, 16)
        .Cases({"int32", "uint32"}, 32)
        .Cases({"int64", "uint64"}, 64);
}

Type Type::getPointerTo() const {
    return PointerType::get(*this);
}

llvm::StringRef Type::getName() const {
    return llvm::cast<BasicType>(typeBase)->name;
}

std::string Type::getQualifiedTypeName() const {
    return llvm::cast<BasicType>(typeBase)->getQualifiedName();
}

Type Type::getElementType() const {
    if (isArrayRef()) return getGenericArgs()[0];
    return llvm::cast<ArrayType>(typeBase)->elementType.withLocation(location);
}

int64_t Type::getArraySize() const {
    return llvm::cast<ArrayType>(typeBase)->size;
}

llvm::ArrayRef<TupleElement> Type::getTupleElements() const {
    return llvm::cast<TupleType>(typeBase)->elements;
}

llvm::ArrayRef<Type> Type::getGenericArgs() const {
    return llvm::cast<BasicType>(typeBase)->genericArgs;
}

Type Type::getReturnType() const {
    return llvm::cast<FunctionType>(typeBase)->returnType.withLocation(location);
}

llvm::ArrayRef<Type> Type::getParamTypes() const {
    return llvm::cast<FunctionType>(typeBase)->paramTypes;
}

Type Type::getPointee() const {
    return llvm::cast<PointerType>(typeBase)->pointeeType.withLocation(location);
}

bool Type::isImplementedAsPointer() const {
    auto unwrapped = removeOptional();
    return unwrapped.isPointerType() || unwrapped.isUnsizedArrayPointer() || unwrapped.isFunctionType();
}

Type Type::getWrappedType() const {
    ASSERT(isOptionalType());
    return getGenericArgs().front().withLocation(location);
}

bool cx::operator==(Type lhs, Type rhs) {
    if (lhs.isMutable() != rhs.isMutable()) return false;
    return lhs.equalsIgnoreTopLevelMutable(rhs);
}

bool Type::equalsIgnoreTopLevelMutable(Type other) const {
    switch (getKind()) {
    case TypeKind::BasicType:
        // HACK, FIXME: stop using equalsIgnoreTopLevelMutable to check if types are the same,
        // it doesn't make sense for anonymous types (e.g. ones embedded in structs imported from C).
        if (getName().empty()) return false;
        // TODO: Should probably compare the referenced decl instead of just the name.
        return other.isBasicType() && getName() == other.getName() && getGenericArgs() == other.getGenericArgs();
    case TypeKind::ArrayType:
        return other.isArrayType() && getElementType() == other.getElementType() && getArraySize() == other.getArraySize();
    case TypeKind::TupleType:
        return other.isTupleType() && getTupleElements() == other.getTupleElements();
    case TypeKind::FunctionType:
        return other.isFunctionType() && getReturnType() == other.getReturnType() && getParamTypes() == other.getParamTypes();
    case TypeKind::PointerType:
        return other.isPointerType() && getPointee() == other.getPointee();
    case TypeKind::UnresolvedType:
        return false;
    }
    llvm_unreachable("all cases handled");
}

bool cx::operator!=(Type lhs, Type rhs) {
    return !(lhs == rhs);
}

bool Type::containsUnresolvedPlaceholder() const {
    switch (getKind()) {
    case TypeKind::BasicType:
        for (Type genericArg : getGenericArgs()) {
            if (genericArg.containsUnresolvedPlaceholder()) {
                return true;
            }
        }
        return false;

    case TypeKind::ArrayType:
        return getElementType().containsUnresolvedPlaceholder();

    case TypeKind::TupleType:
        for (auto& element : getTupleElements()) {
            if (element.type.containsUnresolvedPlaceholder()) {
                return true;
            }
        }
        return false;

    case TypeKind::FunctionType:
        for (Type paramType : getParamTypes()) {
            if (paramType.containsUnresolvedPlaceholder()) {
                return true;
            }
        }
        return getReturnType().containsUnresolvedPlaceholder();

    case TypeKind::PointerType:
        return getPointee().containsUnresolvedPlaceholder();

    case TypeKind::UnresolvedType:
        return true;
    }

    llvm_unreachable("all cases handled");
}

bool Type::isClosureType() const {
    auto* typeDecl = getDecl();
    return typeDecl && typeDecl->isClosure();
}

// Closure structs hold the function pointer in field 0, followed by one field per capture.
// The function takes the captures as hidden leading parameters; the user-visible signature skips them.
llvm::ArrayRef<Type> Type::getClosureParamTypes() const {
    ASSERT(isClosureType());
    auto* closureDecl = getDecl();
    Type functionType = closureDecl->fields.front().type;
    ASSERT(functionType.isFunctionType());
    return functionType.getParamTypes().drop_front(closureDecl->fields.size() - 1);
}

Type Type::getClosureReturnType() const {
    ASSERT(isClosureType());
    Type functionType = getDecl()->fields.front().type;
    ASSERT(functionType.isFunctionType());
    return functionType.getReturnType();
}

TypeDecl* Type::getDecl() const {
    auto* basicType = llvm::dyn_cast<BasicType>(typeBase);
    return basicType ? basicType->decl : nullptr;
}

DestructorDecl* Type::getDestructor() const {
    auto* typeDecl = getDecl();
    return typeDecl ? typeDecl->getDestructor() : nullptr;
}

void Type::printTo(std::ostream& stream) const {
    if (!typeBase) {
        stream << "NULL";
        return;
    }

    switch (typeBase->kind) {
    case TypeKind::BasicType: {
        if (isClosureType()) {
            stream << "(";
            for (const Type& paramType : getClosureParamTypes()) {
                stream << paramType;
                if (&paramType != &getClosureParamTypes().back()) stream << ", ";
            }
            stream << ") -> ";
            getClosureReturnType().printTo(stream);
            break;
        }

        if (isOptionalType()) {
            getWrappedType().printTo(stream);
            if (!isMutable()) stream << " const";
            stream << '?';
            break;
        }

        if (!isMutable()) stream << "const ";
        stream << getName();

        auto genericArgs = llvm::cast<BasicType>(typeBase)->genericArgs;
        if (!genericArgs.empty()) {
            stream << "<";
            for (auto& type : genericArgs) {
                type.printTo(stream);
                if (&type != &genericArgs.back()) stream << ", ";
            }
            stream << ">";
        }

        break;
    }
    case TypeKind::ArrayType:
        getElementType().printTo(stream);
        stream << "[";
        switch (getArraySize()) {
        case ArrayType::UnknownSize:
            stream << "*";
            break;
        default:
            stream << getArraySize();
            break;
        }
        stream << "]";
        break;
    case TypeKind::TupleType:
        stream << "(";
        for (auto& element : getTupleElements()) {
            element.type.printTo(stream);
            stream << " " << element.name;
            if (&element != &getTupleElements().back()) stream << ", ";
        }
        stream << ")";
        break;
    case TypeKind::FunctionType:
        getReturnType().printTo(stream);
        stream << "(";
        for (const Type& paramType : getParamTypes()) {
            stream << paramType;
            if (&paramType != &getParamTypes().back()) stream << ", ";
        }
        stream << ")";
        break;
    case TypeKind::PointerType:
        getPointee().printTo(stream);
        if (!isMutable()) stream << " const";
        stream << '*';
        break;
    case TypeKind::UnresolvedType:
        stream << "<UNRESOLVED>";
        break;
    }
}

std::string Type::toString() const {
    std::ostringstream stream;
    printTo(stream);
    return stream.str();
}

std::ostream& cx::operator<<(std::ostream& stream, Type type) {
    type.printTo(stream);
    return stream;
}

llvm::raw_ostream& cx::operator<<(llvm::raw_ostream& stream, Type type) {
    std::ostringstream stringstream;
    type.printTo(stringstream);
    return stream << stringstream.str();
}
