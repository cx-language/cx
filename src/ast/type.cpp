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
DEFINE_BUILTIN_TYPE_GET_AND_IS(CSizeT, c_size_t)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Byte, byte)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Float, float)
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
        if (isBasicArrayType()) return !isConstantArray() || getElementType().isImplicitlyCopyable();
        return !getDecl() || getDecl()->isStoredByValue();
    case TypeKind::ArrayPointerType:
        return !isConstantArray() || getElementType().isImplicitlyCopyable();
    case TypeKind::AnonymousStructType:
        return llvm::all_of(llvm::cast<AnonymousStructType>(typeBase)->elements, [&](auto& element) { return element.type.isImplicitlyCopyable(); });
    case TypeKind::FunctionType:
    case TypeKind::PointerType:
        return true;
    case TypeKind::UnresolvedType:
        llvm_unreachable("invalid unresolved type");
    }
    llvm_unreachable("all cases handled");
}

bool Type::isConstantArray() const {
    return isBasicArrayType() && getGenericArgs()[1].isInt() && getArraySize() >= 0;
}

bool Type::isSlice() const {
    return isBasicType() && getName() == "Slice";
}

bool Type::isUnsizedArrayPointer() const {
    return getKind() == TypeKind::ArrayPointerType && getArraySize() == ArrayPointerType::UnknownSize;
}

bool Type::isBuiltinScalar(llvm::StringRef typeName) {
    return llvm::StringSwitch<bool>(typeName)
        .Cases({"int", "int8", "int16", "int32", "int64", "int128"}, true)
        .Cases({"uint", "uint8", "uint16", "uint32", "uint64", "uint128", "byte", "c_size_t"}, true)
        .Cases({"float", "float32", "float64", "float80", "bool", "char"}, true)
        .Default(false);
}

bool Type::isEnumType() const {
    if (auto* basicType = llvm::dyn_cast<BasicType>(typeBase)) {
        return basicType->decl && basicType->decl->isEnumDecl();
    }
    return false;
}

Type Type::resolve(const llvm::StringMap<GenericArg>& replacements) const {
    if (!typeBase) return Type(nullptr, mutability, location);

    switch (getKind()) {
    case TypeKind::BasicType: {
        auto it = replacements.find(getName());
        if (it != replacements.end() && it->second.isType()) {
            // TODO: Handle generic arguments for type placeholders.
            Type resolved = it->second.type.withMutability(mutability);
            resolved.location = location;
            return resolved;
        }
        // An integer parameter reference isn't a type; leave it for the use site to diagnose.

        auto genericArgs = map(getGenericArgs(), [&](GenericArg arg) { return arg.resolve(replacements); });
        return BasicType::get(getName(), std::move(genericArgs), mutability, location);
    }
    case TypeKind::ArrayPointerType: {
        Type elementType = llvm::cast<ArrayPointerType>(typeBase)->elementType;
        if (elementType.isBasicType()) {
            if (auto it = replacements.find(elementType.getName()); it != replacements.end() && it->second.isType()) {
                elementType = it->second.type;
                if (!llvm::cast<ArrayPointerType>(typeBase)->elementType.isMutable()) {
                    elementType = elementType.withMutability(Mutability::Const);
                }
            } else {
                elementType = elementType.resolve(replacements);
            }
        } else {
            elementType = elementType.resolve(replacements);
        }
        if (llvm::StringRef sizeParam = getArraySizeParam(); !sizeParam.empty()) {
            // A missing or mistyped substitution leaves the size symbolic; the use site reports it.
            if (auto it = replacements.find(sizeParam); it != replacements.end() && it->second.isInt()) {
                return ArrayPointerType::get(elementType, it->second.getInt(), location);
            }
            return ArrayPointerType::get(elementType, sizeParam, location);
        }
        return ArrayPointerType::get(elementType, getArraySize(), location);
    }

    case TypeKind::AnonymousStructType: {
        auto elements =
            map(getAnonymousStructElements(), [&](auto& element) { return AnonymousStructElement{element.name, element.type.resolve(replacements)}; });
        return AnonymousStructType::get(std::move(elements), mutability, location);
    }
    case TypeKind::FunctionType: {
        auto paramTypes = map(getParamTypes(), [&](Type t) { return t.resolve(replacements); });
        return FunctionType::get(getReturnType().resolve(replacements), std::move(paramTypes), llvm::cast<FunctionType>(typeBase)->isVariadic, mutability,
                                 location);
    }
    case TypeKind::PointerType:
        return PointerType::get(getPointee().resolve(replacements), getPointerKind(), mutability, location);
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

Type BasicType::get(llvm::StringRef name, llvm::ArrayRef<GenericArg> genericArgs, Mutability mutability, Location location) {
    return getType(BasicType(name, genericArgs), mutability, location);
}

Type ArrayPointerType::get(Type elementType, int64_t size, Location location) {
    if (size == UnknownSize) return getType(ArrayPointerType(elementType, size), elementType.mutability, location);

    std::vector<GenericArg> args;
    args.emplace_back(elementType);
    args.push_back(GenericArg::fromInt(size, location));
    return BasicType::get("Array", args, elementType.mutability, location);
}

Type ArrayPointerType::get(Type elementType, llvm::StringRef sizeParam, Location location) {
    // A symbolic size names an integer generic parameter; store it as a type
    // placeholder resolved at instantiation (see Type::resolve above).
    std::vector<GenericArg> args;
    args.emplace_back(elementType);
    args.emplace_back(BasicType::get(sizeParam, {}, elementType.mutability, location));
    return BasicType::get("Array", args, elementType.mutability, location);
}

Type AnonymousStructType::get(std::vector<AnonymousStructElement>&& elements, Mutability mutability, Location location) {
    return getType(AnonymousStructType(std::move(elements)), mutability, location);
}

Type FunctionType::get(Type returnType, std::vector<Type>&& paramTypes, bool isVariadic, Mutability mutability, Location location) {
    return getType(FunctionType(returnType, std::move(paramTypes), isVariadic), mutability, location);
}

Type PointerType::get(Type pointeeType, PointerKind kind, Mutability mutability, Location location) {
    return getType(PointerType(pointeeType, kind), mutability, location);
}

Type OptionalType::get(Type wrappedType, Mutability mutability, Location location) {
    return BasicType::get("Optional", GenericArg(wrappedType), mutability, location);
}

Type UnresolvedType::get(Mutability mutability, Location location) {
    return getType(UnresolvedType(), mutability, location);
}

bool cx::operator==(const AnonymousStructElement& a, const AnonymousStructElement& b) {
    return a.name == b.name && a.type == b.type;
}

bool cx::operator==(const GenericArg& a, const GenericArg& b) {
    if (a.isInt() || b.isInt()) return a.intValue == b.intValue;
    return a.type == b.type;
}

std::string GenericArg::toString() const {
    if (isInt()) return std::to_string(getInt());
    return type.toString();
}

GenericArg GenericArg::resolve(const llvm::StringMap<GenericArg>& replacements) const {
    if (isInt()) return *this;
    if (type.isBasicType()) {
        if (auto it = replacements.find(type.getName()); it != replacements.end()) {
            return it->second;
        }
    }
    GenericArg result = *this;
    result.type = type.resolve(replacements);
    return result;
}

void cx::appendGenericArgs(std::string& typeName, llvm::ArrayRef<GenericArg> genericArgs) {
    if (genericArgs.empty()) return;

    typeName += '<';
    for (const GenericArg& genericArg : genericArgs) {
        typeName += genericArg.toString();
        if (&genericArg != &genericArgs.back()) typeName += ", ";
    }
    typeName += '>';
}

std::string cx::getQualifiedTypeName(llvm::StringRef typeName, llvm::ArrayRef<GenericArg> genericArgs) {
    std::string result = typeName.str();
    appendGenericArgs(result, genericArgs);
    return result;
}

Type cx::getArrayTypeForReceiver(Type type) {
    if (!type || !type.isBasicArrayType() || type.isMutable()) return type;

    auto genericArgs = std::vector<GenericArg>(type.getGenericArgs().begin(), type.getGenericArgs().end());
    genericArgs[0] = GenericArg(type.getElementType());
    return BasicType::get("Array", genericArgs, type.mutability, type.location);
}

std::vector<ParamDecl> FunctionType::getParamDecls(Location location) const {
    return map(paramTypes, [&](Type paramType) { return ParamDecl(paramType, "", false, location); });
}

constexpr auto signedInts = {"int", "int8", "int16", "int32", "int64"};
constexpr auto unsignedInts = {"uint", "uint8", "uint16", "uint32", "uint64", "byte", "c_size_t"};

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
    // c_size_t matches C's size_t, which is pointer-sized: 32-bit on wasm32,
    // 64-bit on the 64-bit native targets. The frontend runs on the same
    // width as its target (native host, or wasm32 under Emscripten), so the
    // host pointer width is the target width. (There is no cross-compilation.)
    if (isCSizeT()) return static_cast<int>(sizeof(void*) * 8);
    return llvm::StringSwitch<int>(getName())
        .Cases({"int", "uint"}, 32)
        .Cases({"int8", "uint8", "byte"}, 8)
        .Cases({"int16", "uint16"}, 16)
        .Cases({"int32", "uint32"}, 32)
        .Cases({"int64", "uint64"}, 64);
}

std::optional<uint64_t> Type::getSizeInBytes() const {
    // Only types whose lowering is fixed across targets. Pointers, aggregates,
    // and float80 depend on the target data layout, which sema cannot see.
    if (isInteger()) return getIntegerBitWidth() / 8;
    if (isInt128() || isUInt128()) return 16;
    if (isChar() || isBool()) return 1;
    if (isFloat() || isFloat32()) return 4;
    if (isFloat64()) return 8;
    return std::nullopt;
}

Type Type::getPointerTo() const {
    return PointerType::get(*this);
}

llvm::StringRef Type::getName() const {
    return llvm::cast<BasicType>(typeBase)->name;
}

std::string Type::getQualifiedTypeName() const {
    Type receiverType = getArrayTypeForReceiver(*this);
    if (!receiverType.isBasicType()) return receiverType.toString();
    return llvm::cast<BasicType>(receiverType.typeBase)->getQualifiedName();
}

Type Type::getElementType() const {
    if (isSlice()) return getGenericArgs()[0].type;
    if (isBasicArrayType()) {
        Type elementType = getGenericArgs()[0].type.withLocation(location);
        return isMutable() ? elementType : elementType.withMutability(Mutability::Const);
    }
    ASSERT(getKind() == TypeKind::ArrayPointerType);
    Type elementType = llvm::cast<ArrayPointerType>(typeBase)->elementType.withLocation(location);
    return isMutable() ? elementType : elementType.withMutability(Mutability::Const);
}

int64_t Type::getArraySize() const {
    if (isBasicArrayType()) {
        auto& sizeArg = getGenericArgs()[1];
        if (sizeArg.isInt()) return sizeArg.getInt();
        // Symbolic size: no concrete size yet; callers check getArraySizeParam first.
        return 0;
    }
    ASSERT(getKind() == TypeKind::ArrayPointerType);
    return llvm::cast<ArrayPointerType>(typeBase)->size;
}

llvm::StringRef Type::getArraySizeParam() const {
    if (isBasicArrayType()) {
        auto& sizeArg = getGenericArgs()[1];
        if (sizeArg.isType() && sizeArg.type.isBasicType()) return sizeArg.type.getName();
        return llvm::StringRef();
    }
    ASSERT(getKind() == TypeKind::ArrayPointerType);
    return llvm::cast<ArrayPointerType>(typeBase)->sizeParam;
}

llvm::ArrayRef<AnonymousStructElement> Type::getAnonymousStructElements() const {
    return llvm::cast<AnonymousStructType>(typeBase)->elements;
}

llvm::ArrayRef<GenericArg> Type::getGenericArgs() const {
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

bool Type::isReferenceType() const {
    return isPointerType() && llvm::cast<PointerType>(typeBase)->pointerKind == PointerKind::Reference;
}

PointerKind Type::getPointerKind() const {
    return llvm::cast<PointerType>(typeBase)->pointerKind;
}

bool Type::containsReference() const {
    switch (getKind()) {
    case TypeKind::BasicType:
        return llvm::any_of(getGenericArgs(), [](GenericArg arg) { return arg.isType() && arg.type.containsReference(); });
    case TypeKind::ArrayPointerType:
        return getElementType().containsReference();
    case TypeKind::AnonymousStructType:
        return llvm::any_of(getAnonymousStructElements(), [](auto& element) { return element.type.containsReference(); });
    case TypeKind::FunctionType:
        return llvm::any_of(getParamTypes(), [](Type param) { return param.containsReference(); }) || getReturnType().containsReference();
    case TypeKind::PointerType:
        return isReferenceType() || getPointee().containsReference();
    case TypeKind::UnresolvedType:
        return false;
    }
    llvm_unreachable("all cases handled");
}

// Whether storing a value of this type would retain a borrow: a reference in any position
// except a function parameter type (naming a function doesn't name its future arguments).
bool Type::storesBorrow() const {
    switch (getKind()) {
    case TypeKind::BasicType:
        return llvm::any_of(getGenericArgs(), [](GenericArg arg) { return arg.isType() && arg.type.storesBorrow(); });
    case TypeKind::ArrayPointerType:
        return getElementType().storesBorrow();
    case TypeKind::AnonymousStructType:
        return llvm::any_of(getAnonymousStructElements(), [](auto& element) { return element.type.storesBorrow(); });
    case TypeKind::FunctionType:
        return getReturnType().storesBorrow();
    case TypeKind::PointerType:
        return isReferenceType() || getPointee().storesBorrow();
    case TypeKind::UnresolvedType:
        return false;
    }
    llvm_unreachable("all cases handled");
}

bool Type::isImplementedAsPointer() const {
    auto unwrapped = removeOptional();
    return unwrapped.isPointerType() || unwrapped.isUnsizedArrayPointer() || unwrapped.isFunctionType();
}

Type Type::getWrappedType() const {
    ASSERT(isOptionalType());
    return getGenericArgs().front().type.withLocation(location);
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
    case TypeKind::ArrayPointerType:
        return other.getKind() == TypeKind::ArrayPointerType && getElementType() == other.getElementType() && getArraySize() == other.getArraySize()
            && getArraySizeParam() == other.getArraySizeParam();
    case TypeKind::AnonymousStructType:
        return other.isAnonymousStructType() && getAnonymousStructElements() == other.getAnonymousStructElements();
    case TypeKind::FunctionType:
        return other.isFunctionType() && getReturnType() == other.getReturnType() && getParamTypes() == other.getParamTypes();
    case TypeKind::PointerType:
        return other.isPointerType() && getPointerKind() == other.getPointerKind() && getPointee() == other.getPointee();
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
        // A symbolic array size (Array<T, N> with N a placeholder) is unresolved.
        if (isBasicArrayType() && !getArraySizeParam().empty()) return true;
        for (GenericArg genericArg : getGenericArgs()) {
            if (genericArg.isType() && genericArg.type.containsUnresolvedPlaceholder()) {
                return true;
            }
        }
        return false;

    case TypeKind::ArrayPointerType:
        return getElementType().containsUnresolvedPlaceholder();

    case TypeKind::AnonymousStructType:
        for (auto& element : getAnonymousStructElements()) {
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
        if (isBasicArrayType()) {
            // Fixed arrays are represented as BasicType("Array", {T, N}), but
            // diagnostics keep the source-level T[N] spelling. The array's
            // constness also constrains its elements, so print that qualifier once.
            Type elementType = getGenericArgs()[0].type.withLocation(location);
            if (!isMutable()) {
                stream << "const ";
                elementType = elementType.withMutability(Mutability::Mutable);
            }
            elementType.printTo(stream);
            stream << "[";
            if (!getArraySizeParam().empty()) {
                stream << getArraySizeParam();
            } else {
                stream << getArraySize();
            }
            stream << "]";
            break;
        }
        if (isClosureType()) {
            stream << "(";
            for (const Type& paramType : getClosureParamTypes()) {
                stream << paramType;
                if (&paramType != &getClosureParamTypes().back()) stream << ", ";
            }
            stream << ") => ";
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
            for (auto& arg : genericArgs) {
                if (arg.isInt()) {
                    stream << arg.getInt();
                } else {
                    arg.type.printTo(stream);
                }
                if (&arg != &genericArgs.back()) stream << ", ";
            }
            stream << ">";
        }

        break;
    }
    case TypeKind::ArrayPointerType:
        getElementType().printTo(stream);
        stream << "[";
        if (!getArraySizeParam().empty()) {
            stream << getArraySizeParam();
        } else if (getArraySize() == ArrayPointerType::UnknownSize) {
            stream << "*";
        } else {
            stream << getArraySize();
        }
        stream << "]";
        break;
    case TypeKind::AnonymousStructType:
        stream << "(";
        for (auto& element : getAnonymousStructElements()) {
            element.type.printTo(stream);
            stream << " " << element.name;
            if (&element != &getAnonymousStructElements().back()) stream << ", ";
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
        stream << (isReferenceType() ? '&' : '*');
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
