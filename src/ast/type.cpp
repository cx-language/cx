#include "type.h"
#include <sstream>
#include <unordered_map>
#pragma warning(push, 0)
#include <llvm/ADT/Hashing.h>
#include <llvm/ADT/ScopeExit.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "../support/utility.h"
#include "arena.h"
#include "ast.h"
#include "decl.h"

using namespace cx;

static std::vector<TypeBase*> typeBases;

// Structural hash matching operator==: equal types hash equal. Locations,
// spellings, and decl pointers are ignored, so spelling twins share one bucket.
static llvm::hash_code hashTypeStructure(Type type) {
    switch (type.getKind()) {
    case TypeKind::BasicType: {
        llvm::hash_code hash = llvm::hash_value(type.getName());
        for (const GenericArg& arg : type.getGenericArgs()) {
            hash = llvm::hash_combine(hash, static_cast<int>(arg.kind));
            if (arg.isInt()) {
                hash = llvm::hash_combine(hash, arg.getInt());
            } else if (arg.isType()) {
                hash = llvm::hash_combine(hash, hashTypeStructure(arg.getType()));
            }
        }
        return llvm::hash_combine(type.getKind(), hash);
    }
    case TypeKind::ArrayPointerType:
        return llvm::hash_combine(type.getKind(), hashTypeStructure(type.getElementType()));
    case TypeKind::AnonymousStructType: {
        llvm::hash_code hash = llvm::hash_value(type.getAnonymousStructElements().size());
        for (const auto& element : type.getAnonymousStructElements()) {
            hash = llvm::hash_combine(hash, llvm::hash_value(element.name), hashTypeStructure(element.type));
        }
        return llvm::hash_combine(type.getKind(), hash);
    }
    case TypeKind::FunctionType: {
        llvm::hash_code hash = llvm::hash_combine(hashTypeStructure(type.getReturnType()), type.getParamTypes().size());
        for (Type paramType : type.getParamTypes()) {
            hash = llvm::hash_combine(hash, hashTypeStructure(paramType));
        }
        auto* functionType = llvm::cast<FunctionType>(type.typeBase);
        return llvm::hash_combine(type.getKind(), functionType->isVariadic, functionType->isExtern, hash);
    }
    case TypeKind::PointerType:
        return llvm::hash_combine(type.getKind(), static_cast<int>(type.getPointerKind()), hashTypeStructure(type.getPointee()));
    case TypeKind::UnresolvedType:
        return llvm::hash_combine(type.getKind());
    }
    llvm_unreachable("all cases handled");
}

struct StructuralTypeHash {
    size_t operator()(TypeBase* base) const { return hashTypeStructure(Type(base, Location(), Location())); }
};

struct StructuralTypeEq {
    bool operator()(TypeBase* a, TypeBase* b) const { return Type(a, Location(), Location()) == Type(b, Location(), Location()); }
};

// Creation-ordered structural twins per shape; the vector usually holds one base.
static std::unordered_map<TypeBase*, std::vector<TypeBase*>, StructuralTypeHash, StructuralTypeEq> typeIndex;

void cx::resetTypeInterning() {
    typeBases.clear();
    typeIndex.clear();
}

#define DEFINE_BUILTIN_TYPE_GET_AND_IS(TYPE, NAME) \
    Type Type::get##TYPE(Location location) { \
        return BasicType::get(#NAME, {}, location); \
    } \
    bool Type::is##TYPE() const { \
        return isBasicType() && getName() == #NAME; \
    }

DEFINE_BUILTIN_TYPE_GET_AND_IS(Void, void)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Bool, bool)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int8, int8)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int16, int16)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int32, int32)
DEFINE_BUILTIN_TYPE_GET_AND_IS(Int64, int64)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt8, uint8)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt16, uint16)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt32, uint32)
DEFINE_BUILTIN_TYPE_GET_AND_IS(UInt64, uint64)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CSizeT, c_size_t)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CSChar, c_schar)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CUChar, c_uchar)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CShort, c_short)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CUShort, c_ushort)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CInt, c_int)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CUInt, c_uint)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CLong, c_long)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CULong, c_ulong)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CLongLong, c_longlong)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CULongLong, c_ulonglong)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CFloat, c_float)
DEFINE_BUILTIN_TYPE_GET_AND_IS(CDouble, c_double)
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
        if (isFixedArray()) return !isConcreteArray() || getElementType().isImplicitlyCopyable();
        return !getDecl() || getDecl()->isStoredByValue();
    case TypeKind::ArrayPointerType:
        return !isConcreteArray() || getElementType().isImplicitlyCopyable();
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

// In-progress needsDestruction queries. Infinite-size types are already an error, but
// checking continues after it, so field cycles must still terminate.
static thread_local std::vector<const TypeDecl*> destructionQueries;

bool Type::needsDestruction() const {
    if (getDestructor()) return true;
    if (isFixedArray()) return getElementType().needsDestruction();
    if (isAnonymousStructType()) {
        for (auto& element : getAnonymousStructElements()) {
            if (element.type.needsDestruction()) return true;
        }
        return false;
    }
    auto* typeDecl = getDecl();
    if (!typeDecl || llvm::is_contained(destructionQueries, typeDecl)) return false;
    destructionQueries.push_back(typeDecl);
    llvm::scope_exit pop([&] { destructionQueries.pop_back(); });
    if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(typeDecl)) return enumDecl->hasDestructiblePayload();
    for (auto& field : typeDecl->fields) {
        // Manually-destroyed fields never count: a struct holding only those
        // either declares a destructor (which runs) or is rejected in sema.
        if (field.isManuallyDestroy) continue;
        if (field.type.needsDestruction()) return true;
    }
    return false;
}

bool Type::isConcreteArray() const {
    return isFixedArray() && getGenericArgs()[1].isInt() && getArraySize() >= 0;
}

bool Type::isSlice() const {
    return isBasicType() && getName() == "Slice";
}

bool Type::isArrayPointer() const {
    return getKind() == TypeKind::ArrayPointerType;
}

constexpr llvm::StringRef builtinScalarNames[] = {
    "int8",  "int16",  "int32",  "int64",   "uint8",      "uint16",      "uint32",  "uint64",   "c_size_t", "c_schar", "c_uchar", "c_short", "c_ushort",
    "c_int", "c_uint", "c_long", "c_ulong", "c_longlong", "c_ulonglong", "c_float", "c_double", "float32",  "float64", "float80", "bool",    "char",
};

bool Type::isBuiltinScalar(llvm::StringRef typeName) {
    return llvm::is_contained(builtinScalarNames, typeName);
}

std::string Type::didYouMeanBuiltin(llvm::StringRef typeName) {
    llvm::StringRef best;
    unsigned bestDistance = 0;
    for (llvm::StringRef name : builtinScalarNames) {
        unsigned distance = typeName.edit_distance(name, true, 2);
        if (distance <= 2 && (best.empty() || distance < bestDistance)) {
            best = name;
            bestDistance = distance;
        }
    }
    // Scale the tolerance with the name length so short names like 'Foo'
    // don't match 'bool' (distance 2) while 'size_t' still matches 'c_size_t'.
    if (best.empty() || bestDistance * 3 > typeName.size()) return "";
    return (" (did you mean '" + best + "'?)").str();
}

bool Type::isEnumType() const {
    if (auto* typeDecl = getDecl()) {
        return typeDecl->isEnumDecl();
    }
    return false;
}

Type Type::resolve(const llvm::StringMap<GenericArg>& replacements) const {
    if (!typeBase) return Type(nullptr, location);

    // Substitution rebuilds the type; keep this use site's spelling when it has one.
    // A bare placeholder has none, so replacement spellings survive that path.
    auto preserveSpelling = [this](Type resolved) {
        if (!aliasSpelling.empty()) resolved.aliasSpelling = aliasSpelling;
        return resolved;
    };

    switch (getKind()) {
    case TypeKind::BasicType: {
        auto it = replacements.find(getName());
        if (it != replacements.end() && it->second.isType()) {
            // TODO: Handle generic arguments for type placeholders.
            Type resolved = it->second.getType();
            resolved.location = location;
            resolved.endLocation = endLocation;
            return preserveSpelling(resolved);
        }
        // An integer parameter reference isn't a type; leave it for the use site to diagnose.

        auto genericArgs = mapAst(getGenericArgs(), [&](GenericArg arg) { return arg.resolve(replacements); });
        return preserveSpelling(BasicType::get(getName(), genericArgs, location, endLocation));
    }
    case TypeKind::ArrayPointerType: {
        auto* base = llvm::cast<ArrayPointerType>(typeBase);
        return preserveSpelling(ArrayPointerType::get(base->elementType.resolve(replacements), location, endLocation));
    }

    case TypeKind::AnonymousStructType: {
        auto elements =
            mapAst(getAnonymousStructElements(), [&](auto& element) { return AnonymousStructElement{element.name, element.type.resolve(replacements)}; });
        return preserveSpelling(AnonymousStructType::get(std::move(elements), location, endLocation));
    }
    case TypeKind::FunctionType: {
        auto paramTypes = mapAst(getParamTypes(), [&](Type t) { return t.resolve(replacements); });
        auto* functionType = llvm::cast<FunctionType>(typeBase);
        return preserveSpelling(FunctionType::get(getReturnType().resolve(replacements), std::move(paramTypes), functionType->isVariadic, location, endLocation,
                                                  functionType->isExtern));
    }
    case TypeKind::PointerType: {
        auto* base = llvm::cast<PointerType>(typeBase);
        return preserveSpelling(PointerType::get(base->pointeeType.resolve(replacements), base->pointerKind, location, endLocation));
    }
    case TypeKind::UnresolvedType: {
        // A deferred size substitutes like any other expression, so generic
        // value parameters keep their usual textual-substitution meaning.
        // (Folding to an integer happens in GenericArg::resolve, which is the
        // only caller that can change the argument kind.)
        auto* unresolved = llvm::cast<UnresolvedType>(typeBase);
        if (!unresolved->deferredSize) llvm_unreachable("invalid unresolved type");
        return preserveSpelling(
            UnresolvedType::getDeferredSize(unresolved->deferredSize->instantiate(replacements), unresolved->homeModule, location, endLocation));
    }
    }
    llvm_unreachable("all cases handled");
}

// Spellings are per-use display names stored in nested Type values. Interning must not
// merge types that differ only in spelling; otherwise, first-creator spelling would leak
// into unrelated diagnostics. Shapes are known to match (checked structurally first).
static bool spellingsEqual(Type a, Type b) {
    if (a.aliasSpelling != b.aliasSpelling) return false;
    switch (a.getKind()) {
    case TypeKind::BasicType: {
        auto argsA = a.getGenericArgs(), argsB = b.getGenericArgs();
        for (size_t i = 0; i < argsA.size(); ++i) {
            if (argsA[i].kind != argsB[i].kind) return false;
            if (argsA[i].isInt()) {
                if (argsA[i].getInt() != argsB[i].getInt()) return false;
            } else if (argsA[i].isType() && !spellingsEqual(argsA[i].getType(), argsB[i].getType())) {
                return false;
            }
        }
        return true;
    }
    case TypeKind::ArrayPointerType:
        return spellingsEqual(llvm::cast<ArrayPointerType>(a.typeBase)->elementType, llvm::cast<ArrayPointerType>(b.typeBase)->elementType);
    case TypeKind::AnonymousStructType: {
        auto elementsA = a.getAnonymousStructElements(), elementsB = b.getAnonymousStructElements();
        for (size_t i = 0; i < elementsA.size(); ++i) {
            if (!spellingsEqual(elementsA[i].type, elementsB[i].type)) return false;
        }
        return true;
    }
    case TypeKind::FunctionType: {
        auto paramsA = a.getParamTypes(), paramsB = b.getParamTypes();
        for (size_t i = 0; i < paramsA.size(); ++i) {
            if (!spellingsEqual(paramsA[i], paramsB[i])) return false;
        }
        return spellingsEqual(a.getReturnType(), b.getReturnType());
    }
    case TypeKind::PointerType:
        return spellingsEqual(a.getPointee(), b.getPointee());
    case TypeKind::UnresolvedType:
        return true;
    }
    llvm_unreachable("all cases handled");
}

template<typename T> static Type getType(T&& typeBase, Location location, Location endLocation = Location()) {
    Type newType(&typeBase, location, endLocation);

    // Unresolved types and anonymous C types never compare equal, so each
    // occurrence keeps its own base without polluting the index.
    bool indexable = newType.getKind() != TypeKind::UnresolvedType && !(newType.isBasicType() && newType.getName().empty());
    std::vector<TypeBase*>* twins = nullptr;
    if (indexable) {
        if (auto it = typeIndex.find(&typeBase); it != typeIndex.end()) {
            twins = &it->second;
            // Twins are creation-ordered, so the first spelling match is the earliest twin.
            for (auto* existingTypeBase : *twins) {
                Type existingType(existingTypeBase, location, endLocation);
                if (spellingsEqual(existingType, newType)) return existingType;
            }
        }
    }

    TypeBase* firstTwin = twins && !twins->empty() ? twins->front() : nullptr;
    typeBases.push_back(makeAST<T>(std::forward<T>(typeBase)));
    typeBases.back()->firstTwin = firstTwin;
    typeBases.back()->identityIndex = typeBases.size() - 1;
    // Share already-registered declarations so getDecl() usually hits without
    // scanning; its lazy search covers twins created before registration.
    if (firstTwin) {
        if (auto* freshBasic = llvm::dyn_cast<BasicType>(typeBases.back())) {
            freshBasic->decl = llvm::cast<BasicType>(firstTwin)->decl;
        }
    }
    if (indexable) {
        if (twins) {
            twins->push_back(typeBases.back());
        } else {
            typeIndex.emplace(typeBases.back(), std::vector<TypeBase*>{typeBases.back()});
        }
    }
    return Type(typeBases.back(), location, endLocation);
}

Type BasicType::get(llvm::StringRef name, llvm::ArrayRef<GenericArg> genericArgs, Location location, Location endLocation) {
    return getType(BasicType(name, genericArgs), location, endLocation);
}

Type BasicType::getArray(Type elementType, int64_t size, Location location, Location endLocation) {
    std::vector<GenericArg> args;
    args.emplace_back(elementType);
    args.push_back(GenericArg::fromInt(size, location));
    return BasicType::get("Array", args, location, endLocation);
}

Type Type::getSizeofMarker(Type operand) {
    return BasicType::get("sizeof", {GenericArg(operand)}, operand.location);
}

bool Type::isSizeofMarker() const {
    return isBasicType() && getName() == "sizeof" && getGenericArgs().size() == 1 && getGenericArgs()[0].isType();
}

Type Type::getSizeofOperand() const {
    ASSERT(isSizeofMarker());
    return getGenericArgs()[0].getType();
}

bool Type::hasSizeofArraySize() const {
    return isFixedArray() && getGenericArgs()[1].isType() && getGenericArgs()[1].getType().isSizeofMarker();
}

Type Type::getSizeofArrayOperand() const {
    ASSERT(hasSizeofArraySize());
    return getGenericArgs()[1].getType().getSizeofOperand();
}

Type ArrayPointerType::get(Type elementType, Location location, Location endLocation) {
    return getType(ArrayPointerType(elementType), location, endLocation);
}

Type AnonymousStructType::get(AstVector<AnonymousStructElement>&& elements, Location location, Location endLocation) {
    return getType(AnonymousStructType(std::move(elements)), location, endLocation);
}

Type FunctionType::get(Type returnType, AstVector<Type>&& paramTypes, bool isVariadic, Location location, Location endLocation, bool isExtern) {
    return getType(FunctionType(returnType, std::move(paramTypes), isVariadic, isExtern), location, endLocation);
}

Type PointerType::get(Type pointeeType, PointerKind kind, Location location, Location endLocation) {
    return getType(PointerType(pointeeType, kind), location, endLocation);
}

Type OptionalType::get(Type wrappedType, Location location) {
    return BasicType::get("Optional", GenericArg(wrappedType), location);
}

Type UnresolvedType::get(Location location) {
    return getType(UnresolvedType(), location);
}

Type UnresolvedType::getDeferredSize(Expr* sizeExpr, Module* homeModule, Location location, Location endLocation) {
    UnresolvedType base;
    base.deferredSize = sizeExpr;
    base.homeModule = homeModule;
    return getType(std::move(base), location, endLocation);
}

bool Type::hasDeferredArraySize() const {
    return isFixedArray() && getGenericArgs()[1].isType() && getGenericArgs()[1].getType().isUnresolvedType()
        && llvm::cast<UnresolvedType>(getGenericArgs()[1].getType().typeBase)->deferredSize;
}

Expr* Type::getDeferredArraySize() const {
    ASSERT(hasDeferredArraySize());
    return llvm::cast<UnresolvedType>(getGenericArgs()[1].getType().typeBase)->deferredSize;
}

Module* Type::getDeferredArraySizeHome() const {
    ASSERT(hasDeferredArraySize());
    return llvm::cast<UnresolvedType>(getGenericArgs()[1].getType().typeBase)->homeModule;
}

bool cx::operator==(const AnonymousStructElement& a, const AnonymousStructElement& b) {
    return a.name == b.name && a.type == b.type;
}

bool cx::operator==(const GenericArg& a, const GenericArg& b) {
    if (a.kind != b.kind) return false;
    switch (a.kind) {
    case GenericArg::Kind::Type:
        return a.getType() == b.getType();
    case GenericArg::Kind::Int:
        return a.getInt() == b.getInt();
    case GenericArg::Kind::Null:
        return true;
    }
    llvm_unreachable("all cases handled");
}

std::string GenericArg::toString() const {
    if (isInt()) return std::to_string(getInt());
    if (isType()) return getType().toString();
    return "NULL";
}

std::string GenericArg::toCanonicalString() const {
    if (isInt()) return std::to_string(getInt());
    if (isType()) return getType().toCanonicalString();
    return "NULL";
}

GenericArg GenericArg::resolve(const llvm::StringMap<GenericArg>& replacements) const {
    if (!isType()) return *this;
    Type type = getType();
    if (type.isBasicType()) {
        if (auto it = replacements.find(type.getName()); it != replacements.end()) {
            return it->second;
        }
    }
    if (type.isUnresolvedType()) {
        // Deferred sizes only occur in size position; fold when substitution
        // leaves nothing name-like, so per-instantiation checks see concrete
        // sizes. Anything else (globals, errors) is sema's job at the use
        // site. Never throws: substitution also runs speculatively during
        // overload matching.
        auto* unresolved = llvm::cast<UnresolvedType>(type.typeBase);
        if (unresolved->deferredSize) {
            Expr* substituted = unresolved->deferredSize->instantiate(replacements);
            try {
                if (substituted->isFoldableIntConstant()) {
                    checkArraySizeDivisors(*substituted);
                    llvm::APSInt size = substituted->getConstantIntegerValue();
                    if (!size.isNegative() && size.getActiveBits() <= 63) {
                        return GenericArg::fromInt(size.getSExtValue(), location);
                    }
                }
            } catch (const CompileError&) {
            }
            GenericArg result = *this;
            result.type = UnresolvedType::getDeferredSize(substituted, unresolved->homeModule, location, endLocation);
            return result;
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
        typeName += genericArg.toCanonicalString();
        if (&genericArg != &genericArgs.back()) typeName += ", ";
    }
    typeName += '>';
}

std::string cx::getQualifiedTypeName(llvm::StringRef typeName, llvm::ArrayRef<GenericArg> genericArgs) {
    std::string result = typeName.str();
    appendGenericArgs(result, genericArgs);
    return result;
}

std::string cx::getDisplayTypeName(llvm::StringRef typeName, llvm::ArrayRef<GenericArg> genericArgs) {
    std::string result = typeName.str();
    if (!genericArgs.empty()) {
        result += '<';
        for (const GenericArg& genericArg : genericArgs) {
            result += genericArg.toString();
            if (&genericArg != &genericArgs.back()) result += ", ";
        }
        result += '>';
    }
    return result;
}

std::vector<ParamDecl> FunctionType::getParamDecls(Location location) const {
    return map(paramTypes, [&](Type paramType) { return ParamDecl(paramType, "", false, location); });
}

constexpr auto signedInts = {"int8", "int16", "int32", "int64", "c_schar", "c_short", "c_int", "c_long", "c_longlong"};
constexpr auto unsignedInts = {"uint8", "uint16", "uint32", "uint64", "c_size_t", "c_uchar", "c_ushort", "c_uint", "c_ulong", "c_ulonglong"};

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
    // c_long matches C's long: 32-bit on Windows and wasm32, 64-bit on the
    // 64-bit Unix targets. The host long has the target width (no cross-compilation).
    if (isCLong() || isCULong()) return static_cast<int>(sizeof(long) * 8);
    return llvm::StringSwitch<int>(getName())
        .Cases({"int8", "uint8", "c_schar", "c_uchar"}, 8)
        .Cases({"int16", "uint16", "c_short", "c_ushort"}, 16)
        .Cases({"int32", "uint32", "c_int", "c_uint"}, 32)
        .Cases({"int64", "uint64", "c_longlong", "c_ulonglong"}, 64);
}

std::optional<uint64_t> Type::getSizeInBytes() const {
    // Only types whose lowering is fixed across targets. Pointers, aggregates,
    // and float80 depend on the target data layout, which sema cannot see.
    if (isInteger()) return getIntegerBitWidth() / 8;
    if (isChar() || isBool()) return 1;
    if (isFloat32() || isCFloat()) return 4;
    if (isFloat64() || isCDouble()) return 8;
    return std::nullopt;
}

Type Type::getPointerTo() const {
    return PointerType::get(*this);
}

llvm::StringRef Type::getName() const {
    return llvm::cast<BasicType>(typeBase)->name;
}

std::string Type::getQualifiedTypeName() const {
    if (!isBasicType()) return toCanonicalString();
    return llvm::cast<BasicType>(typeBase)->getQualifiedName();
}

std::string Type::getDisplayName() const {
    if (!aliasSpelling.empty()) return toString();
    if (!isBasicType()) return toString();
    auto* basicType = llvm::cast<BasicType>(typeBase);
    return getDisplayTypeName(basicType->name, basicType->genericArgs);
}

Type Type::getElementType() const {
    if (isSlice()) return getGenericArgs()[0].getType();
    if (isFixedArray()) return getGenericArgs()[0].getType().withLocation(location, endLocation);
    ASSERT(getKind() == TypeKind::ArrayPointerType);
    return llvm::cast<ArrayPointerType>(typeBase)->elementType.withLocation(location, endLocation);
}

int64_t Type::getArraySize() const {
    if (isFixedArray()) {
        auto& sizeArg = getGenericArgs()[1];
        if (sizeArg.isInt()) return sizeArg.getInt();
        // Symbolic size: no concrete size yet; callers check getArraySizeParam first.
        return 0;
    }
    ASSERT(getKind() == TypeKind::ArrayPointerType);
    return ArrayPointerType::UnknownSize;
}

llvm::StringRef Type::getArraySizeParam() const {
    if (isFixedArray()) {
        auto& sizeArg = getGenericArgs()[1];
        if (sizeArg.isType() && sizeArg.type.isBasicType()) return sizeArg.type.getName();
        return llvm::StringRef();
    }
    ASSERT(getKind() == TypeKind::ArrayPointerType);
    return llvm::StringRef();
}

llvm::ArrayRef<AnonymousStructElement> Type::getAnonymousStructElements() const {
    return llvm::cast<AnonymousStructType>(typeBase)->elements;
}

llvm::ArrayRef<GenericArg> Type::getGenericArgs() const {
    return llvm::cast<BasicType>(typeBase)->genericArgs;
}

Type Type::getReturnType() const {
    return llvm::cast<FunctionType>(typeBase)->returnType.withLocation(location, endLocation);
}

llvm::ArrayRef<Type> Type::getParamTypes() const {
    return llvm::cast<FunctionType>(typeBase)->paramTypes;
}

bool Type::isExternFunctionType() const {
    return isFunctionType() && llvm::cast<FunctionType>(typeBase)->isExtern;
}

Type Type::getPointee() const {
    return llvm::cast<PointerType>(typeBase)->pointeeType.withLocation(location, endLocation);
}

bool Type::isReferenceType() const {
    return isPointerType() && llvm::cast<PointerType>(typeBase)->pointerKind == PointerKind::Reference;
}

PointerKind Type::getPointerKind() const {
    return llvm::cast<PointerType>(typeBase)->pointerKind;
}

bool Type::containsSlice() const {
    if (isSlice()) return true;
    switch (getKind()) {
    case TypeKind::BasicType:
        return llvm::any_of(getGenericArgs(), [](GenericArg arg) { return arg.isType() && arg.getType().containsSlice(); });
    case TypeKind::ArrayPointerType:
        return getElementType().containsSlice();
    case TypeKind::AnonymousStructType:
        return llvm::any_of(getAnonymousStructElements(), [](auto& element) { return element.type.containsSlice(); });
    case TypeKind::FunctionType:
        return llvm::any_of(getParamTypes(), [](Type param) { return param.containsSlice(); }) || getReturnType().containsSlice();
    case TypeKind::PointerType:
        return getPointee().containsSlice();
    case TypeKind::UnresolvedType:
        return false;
    }
    llvm_unreachable("all cases handled");
}

bool Type::containsReference() const {
    switch (getKind()) {
    case TypeKind::BasicType:
        return llvm::any_of(getGenericArgs(), [](GenericArg arg) { return arg.isType() && arg.getType().containsReference(); });
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
        return llvm::any_of(getGenericArgs(), [](GenericArg arg) { return arg.isType() && arg.getType().storesBorrow(); });
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
    return unwrapped.isPointerOrArrayPointer() || unwrapped.isFunctionType();
}

Type Type::getWrappedType() const {
    ASSERT(isOptionalType());
    return getGenericArgs().front().getType().withLocation(location, endLocation);
}

bool cx::operator==(Type lhs, Type rhs) {
    if (lhs.getKind() != rhs.getKind()) return false;
    switch (lhs.getKind()) {
    case TypeKind::BasicType:
        // HACK, FIXME: stop using operator== to check if types are the same,
        // it doesn't make sense for anonymous types (e.g. ones embedded in structs imported from C).
        if (lhs.getName().empty()) return false;
        // TODO: Should probably compare the referenced decl instead of just the name.
        return lhs.getName() == rhs.getName() && lhs.getGenericArgs() == rhs.getGenericArgs();
    case TypeKind::ArrayPointerType:
        return lhs.getElementType() == rhs.getElementType();
    case TypeKind::AnonymousStructType:
        return lhs.getAnonymousStructElements() == rhs.getAnonymousStructElements();
    case TypeKind::FunctionType:
        return lhs.getReturnType() == rhs.getReturnType() && lhs.getParamTypes() == rhs.getParamTypes()
            && llvm::cast<FunctionType>(lhs.typeBase)->isVariadic == llvm::cast<FunctionType>(rhs.typeBase)->isVariadic
            && llvm::cast<FunctionType>(lhs.typeBase)->isExtern == llvm::cast<FunctionType>(rhs.typeBase)->isExtern;
    case TypeKind::PointerType:
        return lhs.getPointerKind() == rhs.getPointerKind() && lhs.getPointee() == rhs.getPointee();
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
        // A symbolic array size (Array<T, N> with N a placeholder) is unresolved,
        // unless it is sizeof-computed, which backends fold with target layout.
        if (isFixedArray() && !getArraySizeParam().empty() && !hasSizeofArraySize()) return true;
        for (GenericArg genericArg : getGenericArgs()) {
            if (genericArg.isType() && genericArg.getType().containsUnresolvedPlaceholder()) {
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
    if (!basicType || basicType->decl) return basicType ? basicType->decl : nullptr;
    // Spelling twins share one declaration, but it may be registered on any of them
    // (e.g. an instantiation built from spelled inference args), so resolve lazily
    // and cache. A miss caches nothing, so later registrations are still found.
    // The index bucket holds exactly the structural twins, creation-ordered.
    if (auto it = typeIndex.find(typeBase); it != typeIndex.end()) {
        for (auto* twinBase : it->second) {
            if (auto* twin = llvm::dyn_cast<BasicType>(twinBase)) {
                if (twin->decl) {
                    basicType->decl = twin->decl;
                    break;
                }
            }
        }
    }
    return basicType->decl;
}

Type Type::canonicalTwin() const {
    if (!typeBase || !typeBase->firstTwin) return *this;
    return Type(typeBase->firstTwin, location);
}

DestructorDecl* Type::getDestructor() const {
    auto* typeDecl = getDecl();
    return typeDecl ? typeDecl->getDestructor() : nullptr;
}

void Type::printTo(std::ostream& stream, bool canonical) const {
    if (!typeBase) {
        stream << "NULL";
        return;
    }
    if (!canonical && !aliasSpelling.empty()) {
        stream << aliasSpelling;
        return;
    }

    switch (typeBase->kind) {
    case TypeKind::BasicType: {
        if (isFixedArray()) {
            // Fixed arrays are represented as BasicType("Array", {T, N}), but
            // diagnostics keep the source-level T[N] spelling.
            getGenericArgs()[0].type.withLocation(location).printTo(stream, canonical);
            stream << "[";
            if (hasSizeofArraySize()) {
                stream << "sizeof(";
                getSizeofArrayOperand().printTo(stream, canonical);
                stream << ")";
            } else if (!getArraySizeParam().empty()) {
                stream << getArraySizeParam();
            } else {
                stream << getArraySize();
            }
            stream << "]";
            break;
        }
        if (isClosureType()) {
            if (canonical) {
                // Canonical strings key instantiations; distinct closures need distinct keys
                // even when their signatures match.
                stream << getName();
                break;
            }
            stream << "(";
            for (const Type& paramType : getClosureParamTypes()) {
                paramType.printTo(stream, canonical);
                if (&paramType != &getClosureParamTypes().back()) stream << ", ";
            }
            stream << ") => ";
            getClosureReturnType().printTo(stream, canonical);
            break;
        }

        if (isOptionalType()) {
            getWrappedType().printTo(stream, canonical);
            stream << '?';
            break;
        }

        if (isSlice()) {
            getElementType().printTo(stream, canonical);
            stream << "[]";
            break;
        }

        stream << getName();

        auto genericArgs = llvm::cast<BasicType>(typeBase)->genericArgs;
        if (!genericArgs.empty()) {
            stream << "<";
            for (auto& arg : genericArgs) {
                if (arg.isInt()) {
                    stream << arg.getInt();
                } else {
                    arg.getType().printTo(stream, canonical);
                }
                if (&arg != &genericArgs.back()) stream << ", ";
            }
            stream << ">";
        }

        break;
    }
    case TypeKind::ArrayPointerType:
        getElementType().printTo(stream, canonical);
        stream << "[*]";
        break;
    case TypeKind::AnonymousStructType:
        stream << "(";
        for (auto& element : getAnonymousStructElements()) {
            element.type.printTo(stream, canonical);
            stream << " " << element.name;
            if (&element != &getAnonymousStructElements().back()) stream << ", ";
        }
        stream << ")";
        break;
    case TypeKind::FunctionType:
        if (llvm::cast<FunctionType>(typeBase)->isExtern) stream << "extern ";
        getReturnType().printTo(stream, canonical);
        stream << "(";
        for (const Type& paramType : getParamTypes()) {
            paramType.printTo(stream, canonical);
            if (&paramType != &getParamTypes().back()) stream << ", ";
        }
        stream << ")";
        break;
    case TypeKind::PointerType:
        getPointee().printTo(stream, canonical);
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

std::string Type::toCanonicalString() const {
    std::ostringstream stream;
    printTo(stream, true);
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
