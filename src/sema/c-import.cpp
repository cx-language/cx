#include "c-import.h"

// Builds that don't link against Clang (such as the WebAssembly build used by
// the online playground) define CX_NO_C_IMPORT to exclude the Clang-based C
// header importer. Importing C headers then fails cleanly with an error.
#ifndef CX_NO_C_IMPORT
#include <memory>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>
#pragma warning(push, 0)
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/Mangle.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/Type.h>
#include <clang/Basic/Builtins.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/LiteralSupport.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/PreprocessorOptions.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/Sema.h>
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/TargetParser/Host.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/ast.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../ast/type.h"
#include "../driver/driver.h"
#include "../support/utility.h"
#include "typecheck.h"

using namespace cx;
using namespace llvm::sys;

namespace {

// Builds an integer literal; null when the value fits no 64-bit cx type.
IntLiteralExpr* makeIntLiteralExpr(const llvm::APSInt& value) {
    if (value.isSigned() ? !value.isSignedIntN(64) : !value.isIntN(64)) return nullptr;
    uint64_t bits = value.isSigned() ? uint64_t(value.getSExtValue()) : value.getZExtValue();
    return makeAST<IntLiteralExpr>(bits, value.isSigned(), Location());
}

struct CToCxConverter final : clang::ASTConsumer {
    CToCxConverter(Module& module, Typechecker& typechecker, clang::TargetInfo* targetInfo, clang::SourceManager& sourceManager, bool cxxMode,
                   clang::MangleContext* mangleContext)
    : module(module), typechecker(typechecker), targetInfo(targetInfo), sourceManager(sourceManager), cxxMode(cxxMode), mangleContext(mangleContext) {}

    void Initialize(clang::ASTContext& context) override { astContext = &context; }

    Type toCx(const clang::BuiltinType& type) {
        switch (type.getKind()) {
        case clang::BuiltinType::Void:
            return Type::getVoid();
        case clang::BuiltinType::Bool:
            return Type::getBool();
        case clang::BuiltinType::Char_S:
        case clang::BuiltinType::Char_U:
            return Type::getChar();
        // Varying-width C integers keep their identity (instead of mapping by
        // width) so they stay ABI-compatible and the C backend spells
        // header-compatible types.
        case clang::BuiltinType::SChar:
            return Type::getCSChar();
        case clang::BuiltinType::UChar:
            return Type::getCUChar();
        case clang::BuiltinType::Short:
            return Type::getCShort();
        case clang::BuiltinType::UShort:
            return Type::getCUShort();
        case clang::BuiltinType::Int:
            return Type::getCInt();
        case clang::BuiltinType::UInt:
            return Type::getCUInt();
        case clang::BuiltinType::Long:
            return Type::getCLong();
        case clang::BuiltinType::ULong:
            return Type::getCULong();
        case clang::BuiltinType::LongLong:
            return Type::getCLongLong();
        case clang::BuiltinType::ULongLong:
            return Type::getCULongLong();
        case clang::BuiltinType::Float16:
        case clang::BuiltinType::BFloat16:
            ASSERT(false); // Skipped before conversion; float32 as a fallback.
            return Type::getFloat32();
        case clang::BuiltinType::Int128:
        case clang::BuiltinType::UInt128:
            ASSERT(false); // Skipped before conversion; int32 as a fallback.
            return Type::getInt32();
        case clang::BuiltinType::Float:
            return Type::getFloat32();
        case clang::BuiltinType::Double:
            return Type::getFloat64();
        case clang::BuiltinType::LongDouble:
            return Type::getFloat80();
        default:
            auto name = type.getName(clang::PrintingPolicy({}));
            WARN(Location(), "unknown C built-in type '" << name << "', defaulting to 'int32'");
            return Type::getInt32();
        }
    }

    static llvm::StringRef getName(const clang::TagDecl& decl) {
        if (!decl.getName().empty()) {
            return decl.getName();
        } else if (auto typedefNameDecl = decl.getTypedefNameForAnonDecl()) {
            return typedefNameDecl->getName();
        }
        return "";
    }

    Type toCx(clang::QualType qualType) {
        auto& type = *qualType.getTypePtr();

        switch (type.getTypeClass()) {
        case clang::Type::Pointer: {
            auto pointeeType = llvm::cast<clang::PointerType>(type).getPointeeType();
            if (pointeeType->isFunctionType()) {
                return OptionalType::get(toCx(pointeeType));
            }
            return OptionalType::get(PointerType::get(toCx(pointeeType), PointerKind::Pointer));
        }
        case clang::Type::Builtin:
            return toCx(llvm::cast<clang::BuiltinType>(type));
        case clang::Type::Typedef: {
            auto& typedefType = llvm::cast<clang::TypedefType>(type);
            return toCx(typedefType.desugar());
        }
        case clang::Type::PredefinedSugar: {
            return toCx(llvm::cast<clang::PredefinedSugarType>(type).desugar());
        }
        case clang::Type::Record: {
            auto& recordType = llvm::cast<clang::RecordType>(type);
            auto* recordDecl = recordType.getDecl();
            auto* typeDecl = toCx(*recordDecl);
            // Spell anonymous records with their generated name: an empty spelling never compares equal, not even to itself.
            llvm::StringRef name = typeDecl ? typeDecl->getName() : getName(*recordDecl);
            auto cxType = BasicType::get(name, {});
            if (typeDecl) bindTypeSpelling(cxType, *typeDecl);
            return cxType;
        }
        case clang::Type::Paren:
            return toCx(llvm::cast<clang::ParenType>(type).getInnerType());
        case clang::Type::FunctionProto: {
            auto& functionProtoType = llvm::cast<clang::FunctionProtoType>(type);
            auto paramTypes = mapAst(functionProtoType.getParamTypes(), [&](clang::QualType paramType) { return toCx(paramType); });
            return FunctionType::get(toCx(functionProtoType.getReturnType()), std::move(paramTypes), functionProtoType.isVariadic());
        }
        case clang::Type::FunctionNoProto: {
            auto& functionNoProtoType = llvm::cast<clang::FunctionNoProtoType>(type);
            return FunctionType::get(toCx(functionNoProtoType.getReturnType()), {}, true);
        }
        case clang::Type::ConstantArray: {
            auto& constantArrayType = llvm::cast<clang::ConstantArrayType>(type);
            if (!constantArrayType.getSize().isIntN(64)) {
                ERROR(Location(), "array is too large");
            }
            std::vector<GenericArg> args;
            args.emplace_back(toCx(constantArrayType.getElementType()));
            args.push_back(GenericArg::fromInt(constantArrayType.getSize().getLimitedValue(), Location()));
            return BasicType::get("Array", args);
        }
        case clang::Type::IncompleteArray: {
            auto& incompleteArrayType = llvm::cast<clang::IncompleteArrayType>(type);
            return ArrayPointerType::get(toCx(incompleteArrayType.getElementType()));
        }
        case clang::Type::Attributed:
            return toCx(llvm::cast<clang::AttributedType>(type).getEquivalentType());
        case clang::Type::Decayed:
            return toCx(llvm::cast<clang::DecayedType>(type).getDecayedType());
        case clang::Type::Enum: {
            auto& enumType = llvm::cast<clang::EnumType>(type);
            auto name = getName(*enumType.getDecl());

            if (name.empty()) {
                return toCx(enumType.getDecl()->getIntegerType());
            } else {
                return BasicType::get(name, {});
            }
        }
        case clang::Type::Vector: {
            auto& vectorType = llvm::cast<clang::VectorType>(type);
            std::vector<GenericArg> args;
            args.emplace_back(toCx(vectorType.getElementType()));
            args.push_back(GenericArg::fromInt(vectorType.getNumElements(), Location()));
            return BasicType::get("Array", args);
        }
        case clang::Type::LValueReference:
        case clang::Type::RValueReference: {
            auto pointeeType = llvm::cast<clang::ReferenceType>(type).getPointeeType();
            return PointerType::get(toCx(pointeeType), PointerKind::Reference);
        }
        case clang::Type::SubstTemplateTypeParm:
            return toCx(llvm::cast<clang::SubstTemplateTypeParmType>(type).desugar());
        default:
            if (cxxMode) {
                // A wrongly-typed field or parameter would corrupt the ABI, so C++ declarations
                // using unhandled types are skipped (counted in the end-of-import summary).
                conversionFailed = true;
                return Type::getInt32();
            }
            WARN(Location(), "unhandled type class '" << type.getTypeClassName() << "' (importing type '" << qualType.getAsString() << "')");
            return Type::getInt32();
        }
    }

    std::optional<FieldDecl> toCx(const clang::FieldDecl& decl, TypeDecl& typeDecl) {
        if (decl.getName().empty()) return std::nullopt;
        return FieldDecl(toCx(decl.getType()), decl.getName(), nullptr, typeDecl, AccessLevel::Default, toCx(decl.getLocation()));
    }

    // Collects the field names an imported record will have after anonymous
    // struct members are promoted, so generated field names can avoid them.
    void collectPromotedNames(const clang::RecordDecl& def, bool parentIsUnion, std::unordered_set<std::string>& names) {
        for (auto* field : def.fields()) {
            if (field->isAnonymousStructOrUnion()) {
                auto* anonRecord = field->getType()->getAsRecordDecl();
                const clang::RecordDecl* anonDef = anonRecord ? anonRecord->getDefinition() : nullptr;
                if (anonDef && !anonDef->isUnion() && !parentIsUnion) {
                    collectPromotedNames(*anonDef, false, names);
                }
            } else if (!field->getName().empty()) {
                names.emplace(field->getName());
            }
        }
    }

    // Appends the conversion of one field of a C record definition to the cx
    // record. Anonymous structs nested in structs are promoted (they're laid
    // out in place, so this preserves the C layout); anything nested in a
    // union nests under a generated field name since union members overlap
    // rather than sequence. Returns false if the field can't be converted, in
    // which case the whole struct is dropped.
    bool appendField(const clang::FieldDecl& field, TypeDecl& typeDecl, bool& hasAnonymousMember, std::unordered_set<std::string>& usedNames,
                     unsigned& anonymousMemberCount) {
        if (field.isAnonymousStructOrUnion()) {
            auto* anonRecord = field.getType()->getAsRecordDecl();
            const clang::RecordDecl* anonDef = anonRecord ? anonRecord->getDefinition() : nullptr;
            if (!anonDef) return false;
            hasAnonymousMember = true;
            if (anonDef->isUnion() || typeDecl.isUnion()) {
                TypeDecl* nested = toCx(*anonDef);
                if (!nested) return false;
                std::string fieldName;
                do {
                    fieldName = "unnamed_" + std::to_string(anonymousMemberCount++);
                } while (!usedNames.insert(fieldName).second);
                auto nestedType = BasicType::get(nested->getName(), {});
                bindTypeSpelling(nestedType, *nested);
                typeDecl.fields.emplace_back(nestedType, fieldName, nullptr, typeDecl, AccessLevel::Default, Location());
                typeDecl.fields.back().isAnonymousMember = true;
            } else {
                for (auto* subfield : anonDef->fields()) {
                    if (!appendField(*subfield, typeDecl, hasAnonymousMember, usedNames, anonymousMemberCount)) return false;
                }
            }
            return true;
        }
        // Bit widths, field alignment, and [[no_unique_address]] change the layout in ways cx
        // fields cannot express, so records using them have no cx counterpart.
        if (cxxMode && (field.isBitField() || field.hasAttr<clang::AlignedAttr>() || field.hasAttr<clang::NoUniqueAddressAttr>())) return false;
        auto fieldDecl = toCx(field, typeDecl);
        if (!fieldDecl) return false;
        if (auto* fieldRecord = field.getType()->getAsRecordDecl()) {
            if (getName(*fieldRecord).empty()) hasAnonymousMember = true;
        }
        typeDecl.fields.emplace_back(std::move(*fieldDecl));
        return true;
    }

    TypeDecl* toCx(const clang::RecordDecl& recordDecl) {
        // Key by canonical declaration so that forward declarations, the
        // definition, and references to either all resolve to the same type.
        // References (e.g. `struct S*` inside an earlier typedef) may be
        // converted before the definition is seen, leaving an empty struct
        // that gets filled in below once the definition arrives.
        const clang::RecordDecl* canonical = llvm::cast<clang::RecordDecl>(recordDecl.getCanonicalDecl());
        auto it = importedRecordDecls.find(canonical);
        if (it == importedRecordDecls.end()) {
            auto tag = recordDecl.isUnion() ? TypeTag::Union : TypeTag::Struct;
            llvm::StringRef name = getName(recordDecl);
            std::string anonymousName;
            if (name.empty()) {
                // Anonymous records only occur nested in other records, so
                // they always have a definition; give them a generated name.
                do {
                    anonymousName = "AnonymousRecord" + std::to_string(anonymousRecordCount++);
                } while (!module.symbolTable.findInTopLevelScope(anonymousName).empty());
                name = anonymousName;
            }
            auto* typeDecl =
                makeAST<TypeDecl>(tag, name, AstVector<GenericArg>(), AstVector<Type>(), AccessLevel::Default, module, nullptr, toCx(recordDecl.getLocation()));
            // Records without a tag (including typedef-named ones) cannot be referenced
            // as `struct Name` from generated C, so backends define them under a mangled name.
            typeDecl->isAnonymousRecord = recordDecl.getName().empty();
            it = importedRecordDecls.emplace(canonical, typeDecl).first;

            // Add to symbol table before type-checking so that type-checker finds the struct decl.
            module.addToSymbolTable(typeDecl);
            module.sourceFiles.front().topLevelDecls.push_back(typeDecl);
        }

        // Fill in the fields from the definition once available. This runs at
        // most once per struct; structs without a definition (opaque types
        // used only through pointers) stay empty as before.
        if (const clang::RecordDecl* def = canonical->getDefinition()) {
            if (cxxMode) {
                if (auto* cxxDef = llvm::dyn_cast<clang::CXXRecordDecl>(def)) {
                    // Only standard-layout structs with a trivial destructor map to cx structs:
                    // bases would be missed by the field walk. User constructors are fine (cx
                    // initializes fields directly); by-value passing is gated separately on
                    // trivial copyability, since C++ passes non-trivial values indirectly.
                    if (cxxDef->getNumBases() > 0 || !cxxDef->isStandardLayout() || !cxxDef->hasTrivialDestructor()) {
                        conversionFailed = true;
                        return nullptr;
                    }
                }
            }
            if (completedRecordDecls.insert(canonical).second) {
                TypeDecl* typeDecl = it->second;
                typeDecl->packed = def->hasAttr<clang::PackedAttr>();

                bool hasAnonymousMember = false;
                unsigned anonymousMemberCount = 0;
                std::unordered_set<std::string> usedNames;
                collectPromotedNames(*def, typeDecl->isUnion(), usedNames);
                for (auto* field : def->fields()) {
                    if (!appendField(*field, *typeDecl, hasAnonymousMember, usedNames, anonymousMemberCount)) {
                        return nullptr;
                    }
                }
                // Fields are known now; re-bind in case the transiently empty
                // struct masked a collision with a same-named cx type.
                bindTypeSpelling(typeDecl->getType(), *typeDecl);

                // Structs with anonymous members keep the C layout but get no
                // autogenerated constructor: overlapping union members can't
                // be constructed by field, so none of these structs can.
                if (def->isStruct() && !hasAnonymousMember) {
                    // TODO: Add types in 'addAutogeneratedConstructor' so we don't need to type-check afterwards?
                    typeDecl->addAutogeneratedConstructor();
                    // Add types to the autogenerated constructor for IRGen:
                    llvm::SaveAndRestore setModule(typechecker.currentModule, &module);
                    llvm::SaveAndRestore setSourceFile(typechecker.currentSourceFile, &module.sourceFiles.front());
                    typechecker.typecheckTypeDecl(*typeDecl);
                }
            }
        }
        return it->second;
    }

    VarDecl* toCx(const clang::VarDecl& decl) {
        // C const is dropped like everywhere else: the value has storage but
        // isn't a compile-time constant, so the binding stays mutable.
        return makeAST<VarDecl>(toCx(decl.getType()), decl.getName(), nullptr, nullptr, AccessLevel::Default, module, toCx(decl.getLocation()));
    }

    void addConstantToSymbolTable(llvm::StringRef name, Expr* initializer, Type type) {
        initializer->type = type;
        auto* varDecl = makeAST<VarDecl>(type, name, initializer, nullptr, AccessLevel::Default, module, Location());
        varDecl->isConst = true;
        module.addToSymbolTable(varDecl);
        module.sourceFiles.front().topLevelDecls.push_back(varDecl);
    }

    void addIntegerConstantToSymbolTable(llvm::StringRef name, llvm::APSInt value, clang::QualType qualType) {
        auto* initializer = makeIntLiteralExpr(value);
        ASSERT(initializer);
        addConstantToSymbolTable(name, initializer, toCx(qualType));
    }

    void addFloatConstantToSymbolTable(llvm::StringRef name, llvm::APFloat value) {
        auto initializer = makeAST<FloatLiteralExpr>(std::move(value), Location());
        addConstantToSymbolTable(name, initializer, Type::getFloat64());
    }

    void addStringConstantToSymbolTable(llvm::StringRef name, llvm::StringRef value) {
        auto initializer = makeAST<StringLiteralExpr>(value, Location());
        addConstantToSymbolTable(name, initializer, BasicType::get("string", {}));
    }

    void addCharConstantToSymbolTable(llvm::StringRef name, char value) {
        auto initializer = makeAST<CharacterLiteralExpr>(value, Location());
        addConstantToSymbolTable(name, initializer, Type::getChar());
    }

    // Scalars with no cx counterpart. Mirrors toCx case for case in
    // findUnsupportedScalar, including pointed-to records: conversion fills
    // in record fields eagerly, so a pointer doesn't hide one.
    enum class UnsupportedScalar { Float16, Int128 };

    std::optional<UnsupportedScalar> findUnsupportedScalar(clang::QualType qualType, std::unordered_set<const clang::RecordDecl*>& visited) {
        auto& type = *qualType.getTypePtr();
        switch (type.getTypeClass()) {
        case clang::Type::Pointer:
            return findUnsupportedScalar(llvm::cast<clang::PointerType>(type).getPointeeType(), visited);
        case clang::Type::LValueReference:
        case clang::Type::RValueReference:
            return findUnsupportedScalar(llvm::cast<clang::ReferenceType>(type).getPointeeType(), visited);
        case clang::Type::SubstTemplateTypeParm:
            return findUnsupportedScalar(llvm::cast<clang::SubstTemplateTypeParmType>(type).desugar(), visited);
        case clang::Type::Builtin: {
            auto kind = llvm::cast<clang::BuiltinType>(type).getKind();
            if (kind == clang::BuiltinType::Float16 || kind == clang::BuiltinType::BFloat16) return UnsupportedScalar::Float16;
            if (kind == clang::BuiltinType::Int128 || kind == clang::BuiltinType::UInt128) return UnsupportedScalar::Int128;
            return std::nullopt;
        }
        case clang::Type::Typedef:
            return findUnsupportedScalar(llvm::cast<clang::TypedefType>(type).desugar(), visited);
        case clang::Type::PredefinedSugar:
            return findUnsupportedScalar(llvm::cast<clang::PredefinedSugarType>(type).desugar(), visited);
        case clang::Type::Record: {
            auto* def = llvm::cast<clang::RecordType>(type).getDecl()->getDefinition();
            if (!def) return std::nullopt;
            return findUnsupportedScalarInRecord(*def, visited);
        }
        case clang::Type::Paren:
            return findUnsupportedScalar(llvm::cast<clang::ParenType>(type).getInnerType(), visited);
        case clang::Type::FunctionProto: {
            auto& functionProtoType = llvm::cast<clang::FunctionProtoType>(type);
            if (auto unsupported = findUnsupportedScalar(functionProtoType.getReturnType(), visited)) return unsupported;
            for (clang::QualType paramType : functionProtoType.getParamTypes()) {
                if (auto unsupported = findUnsupportedScalar(paramType, visited)) return unsupported;
            }
            return std::nullopt;
        }
        case clang::Type::FunctionNoProto:
            return findUnsupportedScalar(llvm::cast<clang::FunctionNoProtoType>(type).getReturnType(), visited);
        case clang::Type::ConstantArray:
            return findUnsupportedScalar(llvm::cast<clang::ConstantArrayType>(type).getElementType(), visited);
        case clang::Type::IncompleteArray:
            return findUnsupportedScalar(llvm::cast<clang::IncompleteArrayType>(type).getElementType(), visited);
        case clang::Type::Attributed:
            return findUnsupportedScalar(llvm::cast<clang::AttributedType>(type).getEquivalentType(), visited);
        case clang::Type::Decayed:
            return findUnsupportedScalar(llvm::cast<clang::DecayedType>(type).getDecayedType(), visited);
        case clang::Type::Vector:
            return findUnsupportedScalar(llvm::cast<clang::VectorType>(type).getElementType(), visited);
        case clang::Type::Enum:
        default:
            return std::nullopt;
        }
    }

    std::optional<UnsupportedScalar> findUnsupportedScalarInRecord(const clang::RecordDecl& recordDecl, std::unordered_set<const clang::RecordDecl*>& visited) {
        const clang::RecordDecl* def = recordDecl.getDefinition();
        if (!def) return std::nullopt;
        if (!visited.insert(llvm::cast<clang::RecordDecl>(def->getCanonicalDecl())).second) return std::nullopt;
        for (auto* field : def->fields()) {
            if (auto unsupported = findUnsupportedScalar(field->getType(), visited)) return unsupported;
        }
        return std::nullopt;
    }

    // Declarations using a scalar with no cx counterpart and no size- and
    // ABI-preserving mapping are skipped. The header still imports; using a
    // skipped name fails at the use site. The int128 skip is silent: system
    // headers use __int128 in declarations nobody imports for (e.g. Mach
    // thread states), so warning would be pure noise.
    bool skipIfUnsupportedScalar(std::optional<UnsupportedScalar> unsupported, Location location, llvm::StringRef name) {
        if (!unsupported) return false;
        if (*unsupported == UnsupportedScalar::Float16) {
            WARN(location, "skipping C declaration '" << name << "': 16-bit floating-point types are not supported");
        }
        return true;
    }

    bool skipIfUnsupportedScalar(clang::QualType type, llvm::StringRef name, Location location) {
        std::unordered_set<const clang::RecordDecl*> visited;
        return skipIfUnsupportedScalar(findUnsupportedScalar(type, visited), location, name);
    }

    bool skipIfUnsupportedScalar(const clang::RecordDecl& recordDecl) {
        std::unordered_set<const clang::RecordDecl*> visited;
        return skipIfUnsupportedScalar(findUnsupportedScalarInRecord(recordDecl, visited), toCx(recordDecl.getLocation()), getName(recordDecl));
    }

    static clang::QualType peelArrayTypes(clang::QualType type) {
        while (type->isArrayType())
            type = llvm::cast<clang::ArrayType>(type.getTypePtr())->getElementType();
        return type;
    }

    // True when the record is verifiably trivially copyable using only syntactic queries
    // (no instantiation, which is lazily unavailable during importing): a complete
    // definition, no virtuals, no virtual bases, no user-declared copy/move constructor
    // or destructor, and all value-position members and bases recursively trivial.
    // Anything unverifiable returns false (fail closed). Value-position type cycles are
    // ill-formed, so the recursion terminates without a visited set.
    static bool isVerifiablyTrivialRecord(const clang::CXXRecordDecl* record) {
        auto* def = record->getDefinition();
        if (!def) return false;
        record = def;
        if (record->isPolymorphic() || record->getNumVBases() != 0 || record->hasUserDeclaredCopyConstructor() || record->hasUserDeclaredMoveConstructor()
            || record->hasUserDeclaredDestructor()) {
            return false;
        }
        for (const auto& base : record->bases()) {
            auto* baseRecord = base.getType()->getAsCXXRecordDecl();
            if (!baseRecord || !isVerifiablyTrivialRecord(baseRecord)) return false;
        }
        for (const auto* field : record->fields()) {
            clang::QualType fieldType = peelArrayTypes(field->getType().getCanonicalType());
            if (fieldType->isPointerType() || fieldType->isReferenceType()) continue;
            if (auto* fieldRecord = fieldType->getAsCXXRecordDecl()) {
                if (!isVerifiablyTrivialRecord(fieldRecord)) return false;
            }
        }
        return true;
    }

    static bool isRecordPassedByValue(clang::QualType type) {
        type = type.getCanonicalType();
        if (type->isReferenceType() || type->isPointerType() || type->isArrayType() || type->isFunctionType()) return false;
        return type->getAsCXXRecordDecl() != nullptr;
    }

    // True when a record holds a float anywhere. The LLVM backend expands float-containing
    // aggregates element-wise, but the C++ ABI packs small ones into shared registers.
    // Value-position type cycles are ill-formed, so the recursion terminates without a visited set.
    static bool recordContainsFloat(const clang::RecordDecl* def) {
        for (const auto* field : def->fields()) {
            clang::QualType fieldType = peelArrayTypes(field->getType().getCanonicalType());
            if (fieldType->hasFloatingRepresentation()) return true;
            if (auto* fieldRecord = fieldType->getAsRecordDecl()) {
                if (auto* fieldDef = fieldRecord->getDefinition()) {
                    if (recordContainsFloat(fieldDef)) return true;
                }
            }
        }
        return false;
    }

    // True when a trivially copyable record crosses by value exactly: non-empty, at most 16
    // bytes, at most 8-aligned, and holding no floats. Larger or over-aligned aggregates cross
    // indirectly in cx but in memory or registers in C++.
    bool recordCrossesByValue(clang::QualType type, const clang::CXXRecordDecl* record) {
        auto* def = record->getDefinition();
        if (!def || def->field_empty()) return false;
        auto info = astContext->getTypeInfoInChars(type);
        if (info.Width.alignTo(info.Align).getQuantity() > 16 || info.Align.getQuantity() > 8) return false;
        return !recordContainsFloat(def);
    }

    bool recordParamIsUnsupported(clang::QualType type) {
        type = type.getCanonicalType();
        if (!isRecordPassedByValue(type)) return false;
        auto* record = type->getAsCXXRecordDecl();
        return !isVerifiablyTrivialRecord(record) || !recordCrossesByValue(type, record);
    }

    // True when a function signature crosses a record by value in a way the backends cannot
    // reproduce, so the declaration must be skipped: a non-trivial, large, over-aligned, or
    // float-containing record as a parameter, or any record as the return value (returns lack
    // the parameter integer-chunk coercion). Function pointers recurse on both sides: a cx
    // callback receives the same call, so its signature must cross too, and a returned function
    // pointer is only callable through cx with a crossing signature. Passing the record behind
    // a pointer or reference (or returning through an out-parameter) works.
    bool passesUnsupportedRecordByValue(clang::QualType type) {
        type = type.getCanonicalType();
        if (type->isReferenceType() || type->isPointerType()) {
            // Only function signatures matter through indirection; other pointees cross opaquely.
            clang::QualType pointee = type->getPointeeType().getCanonicalType();
            return pointee->isFunctionProtoType() && passesUnsupportedRecordByValue(pointee);
        }
        if (auto* proto = type->getAs<clang::FunctionProtoType>()) {
            clang::QualType returnType = proto->getReturnType();
            if (isRecordPassedByValue(returnType) || passesUnsupportedRecordByValue(returnType)) return true;
            for (clang::QualType paramType : proto->getParamTypes()) {
                if (recordParamIsUnsupported(paramType) || passesUnsupportedRecordByValue(paramType)) return true;
            }
        }
        return false;
    }

    // True for declarations directly in the global scope. Linkage specifications
    // (e.g. `extern "C"` blocks) don't count as scopes for this purpose.
    static bool isGlobalScope(const clang::Decl& decl) {
        auto* context = decl.getDeclContext();
        while (llvm::isa<clang::LinkageSpecDecl>(context))
            context = context->getParent();
        return context->isTranslationUnit();
    }

    // True when an identical function (same signature) was already imported,
    // so re-inclusion doesn't produce duplicate declarations. Differing
    // signatures are overloads, which are all imported.
    bool isAlreadyImported(const FunctionDecl& functionDecl) {
        for (auto* existing : module.symbolTable.findInTopLevelScope(functionDecl.getName())) {
            auto* existingFunction = llvm::dyn_cast<FunctionDecl>(existing);
            if (!existingFunction || existingFunction->isVariadic() != functionDecl.isVariadic()
                || existingFunction->getParams().size() != functionDecl.getParams().size()) {
                continue;
            }
            bool sameParams = true;
            for (size_t i = 0; i < existingFunction->getParams().size(); ++i) {
                if (!(existingFunction->getParams()[i].type == functionDecl.getParams()[i].type)) {
                    sameParams = false;
                    break;
                }
            }
            if (sameParams && existingFunction->getReturnType() == functionDecl.getReturnType()) return true;
        }
        return false;
    }

    bool HandleTopLevelDecl(clang::DeclGroupRef declGroup) override {
        std::vector<clang::Decl*> pending(declGroup.begin(), declGroup.end());
        for (size_t i = 0; i < pending.size(); ++i) {
            clang::Decl* decl = pending[i];
            // Linkage specifications (e.g. `extern "C"` blocks) wrap their declarations; unwrap
            // them so the contents import like the surrounding declarations.
            if (auto* linkage = llvm::dyn_cast<clang::LinkageSpecDecl>(decl)) {
                pending.insert(pending.end(), linkage->decls_begin(), linkage->decls_end());
                continue;
            }
            conversionFailed = false;
            // Only declarations written in the imported header itself are imported; headers it includes
            // contribute nothing nameable from cx. (Everything is entered as C_System, so system-ness can't
            // tell them apart; the main file ID can.) Referenced types still convert on demand.
            if (cxxMode && sourceManager.getFileID(sourceManager.getExpansionLoc(decl->getLocation())) != sourceManager.getMainFileID()) continue;
            try {
                switch (decl->getKind()) {
                case clang::Decl::Function: {
                    auto& clangDecl = llvm::cast<clang::FunctionDecl>(*decl);
                    if (cxxMode && (!clangDecl.getDeclName().isIdentifier() || !isGlobalScope(clangDecl))) {
                        countSkippedCxxDecl(clangDecl);
                        break;
                    }
                    if (cxxMode && passesUnsupportedRecordByValue(clangDecl.getType())) {
                        countSkippedCxxDecl(clangDecl);
                        break;
                    }
                    if (skipIfUnsupportedScalar(clangDecl.getType(), clangDecl.getNameAsString(), toCx(clangDecl.getLocation()))) break;
                    auto functionDecl = toCx(clangDecl);
                    if (conversionFailed) {
                        countSkippedCxxDecl(clangDecl);
                        break;
                    }
                    if (cxxMode ? !isAlreadyImported(*functionDecl) : module.symbolTable.findInTopLevelScope(functionDecl->getName()).empty()) {
                        module.addToSymbolTable(functionDecl);
                        module.sourceFiles.front().topLevelDecls.push_back(functionDecl);
                    }
                    break;
                }
                case clang::Decl::Record:
                case clang::Decl::CXXRecord: {
                    auto& recordDecl = llvm::cast<clang::RecordDecl>(*decl);
                    // Convert definitions even when a forward declaration came
                    // first; toCx unifies them via the canonical declaration.
                    if (!decl->isFirstDecl() && !recordDecl.isCompleteDefinition()) break;
                    if (cxxMode && !isGlobalScope(recordDecl)) {
                        countSkippedCxxDecl(recordDecl);
                        break;
                    }
                    if (skipIfUnsupportedScalar(recordDecl)) break;
                    auto* converted = toCx(recordDecl);
                    if (cxxMode && !converted) countSkippedCxxDecl(recordDecl);
                    break;
                }
                case clang::Decl::Namespace:
                case clang::Decl::ClassTemplate:
                case clang::Decl::FunctionTemplate:
                    if (cxxMode) countSkippedCxxDecl(*decl);
                    break;
                case clang::Decl::Enum: {
                    auto& enumDecl = llvm::cast<clang::EnumDecl>(*decl);
                    // getIntegerType needs the definition; forward declarations have no cases to convert.
                    if (enumDecl.getDefinition() && skipIfUnsupportedScalar(enumDecl.getIntegerType(), getName(enumDecl), toCx(enumDecl.getLocation()))) {
                        break;
                    }
                    bool isAnonymous = getName(enumDecl).empty();
                    AstVector<EnumCase> cases;

                    for (clang::EnumConstantDecl* enumerator : enumDecl.enumerators()) {
                        auto enumeratorName = enumerator->getName();
                        auto value = enumerator->getInitVal();
                        auto* valueExpr = makeIntLiteralExpr(value);
                        if (!valueExpr) {
                            WARN(Location(), "skipping C enumerator '" << enumeratorName << "': value does not fit 64 bits");
                            continue;
                        }
                        cases.push_back(EnumCase(enumeratorName, valueExpr, Type(), AccessLevel::Default, Location()));
                        auto type = isAnonymous ? enumDecl.getIntegerType()
                                                : astContext->getTagType(clang::ElaboratedTypeKeyword::None, clang::NestedNameSpecifier(), &enumDecl, false);
                        addIntegerConstantToSymbolTable(enumeratorName, value, type);
                    }

                    auto* cxEnumDecl = makeAST<EnumDecl>(getName(enumDecl), std::move(cases), AstVector<Type>(), AccessLevel::Default, module, nullptr,
                                                         toCx(enumDecl.getLocation()));
                    module.addToSymbolTable(cxEnumDecl);
                    module.sourceFiles.front().topLevelDecls.push_back(cxEnumDecl);
                    break;
                }
                case clang::Decl::Var: {
                    auto& varDecl = llvm::cast<clang::VarDecl>(*decl);
                    if (varDecl.getLinkageInternal() != clang::Linkage::External) break;
                    if (cxxMode) {
                        // C++ globals have mangled names with no import support yet.
                        countSkippedCxxDecl(varDecl);
                        break;
                    }
                    if (skipIfUnsupportedScalar(varDecl.getType(), varDecl.getNameAsString(), toCx(varDecl.getLocation()))) break;
                    auto* cxVarDecl = toCx(varDecl);
                    module.addToSymbolTable(*cxVarDecl);
                    module.sourceFiles.front().topLevelDecls.push_back(cxVarDecl);
                    break;
                }
                case clang::Decl::Typedef: {
                    auto& typedefDecl = llvm::cast<clang::TypedefDecl>(*decl);
                    if (skipIfUnsupportedScalar(typedefDecl.getUnderlyingType(), typedefDecl.getNameAsString(), toCx(typedefDecl.getLocation()))) break;
                    auto underlyingType = toCx(typedefDecl.getUnderlyingType());
                    if (conversionFailed) {
                        countSkippedCxxDecl(typedefDecl);
                        break;
                    }
                    if (underlyingType.isBasicType()) {
                        // HACK: This defines a type alias in a hacky way
                        llvm::cast<BasicType>(BasicType::get(typedefDecl.getName(), {}).typeBase)->name = underlyingType.getName();
                    } else {
                        // TODO: Import non-BasicType typedefs from C headers.
                    }
                    break;
                }
                default:
                    break;
                }

                // Can't throw exceptions through the Clang API as it has exceptions disabled,
                // so need to handle them here.
            } catch (const CompileError& error) {
                CompileError augmentedError = error;
                augmentedError.message.insert(0, "encountered an internal compiler error in C import: ");
                augmentedError.reportAsWarning();
            } catch (...) {
                WARN(Location(), "Unhandled exception in C import");
            }
        }

        return true; // continue parsing
    }

    FunctionDecl* toCx(const clang::FunctionDecl& decl) {
        auto params =
            mapAst(decl.parameters(), [&](clang::ParmVarDecl* param) { return ParamDecl(toCx(param->getType()), param->getName(), false, Location()); });

        FunctionProto proto(decl.getName(), std::move(params), toCx(decl.getReturnType()), decl.isVariadic(), true);
        if (auto asmLabelAttr = decl.getAttr<clang::AsmLabelAttr>()) {
            proto.asmLabel = internString(asmLabelAttr->getLabel());
        } else if (cxxMode && decl.getLanguageLinkage() == clang::CXXLanguageLinkage && mangleContext->shouldMangleDeclName(&decl)) {
            std::string mangled;
            llvm::raw_string_ostream stream(mangled);
            mangleContext->mangleName(&decl, stream);
            // The \01 marker bypasses LLVM's target symbol prefix, so add the Mach-O/MinGW '_' explicitly.
            if (targetInfo->getTriple().isOSBinFormatMachO() || targetInfo->getTriple().isOSCygMing()) mangled = "_" + mangled;
            proto.asmLabel = internString(mangled);
        }
        return makeAST<FunctionDecl>(std::move(proto), AstVector<GenericArg>(), AccessLevel::Default, module, toCx(decl.getLocation()));
    }

    Location toCx(clang::SourceLocation location) {
        if (location.isInvalid()) return Location();
        auto presumedLocation = sourceManager.getPresumedLoc(location);
        if (presumedLocation.isInvalid()) return Location();
        return Location(internString(presumedLocation.getFilename()).data(), presumedLocation.getLine(), presumedLocation.getColumn());
    }

private:
    Module& module;
    Typechecker& typechecker;
    clang::TargetInfo* targetInfo;
    clang::SourceManager& sourceManager;
    clang::ASTContext* astContext = nullptr;
    std::unordered_map<const clang::RecordDecl*, TypeDecl*> importedRecordDecls;
    std::unordered_set<const clang::RecordDecl*> completedRecordDecls;
    static inline unsigned anonymousRecordCount = 0;
    const bool cxxMode;
    clang::MangleContext* mangleContext;
    // Set when a C++ declaration uses a type with no cx counterpart; the declaration is skipped.
    bool conversionFailed = false;
    // Skipped C++ declarations written in the imported header itself, reported once per import.
    unsigned skippedCxxDecls = 0;

    // Counts a skipped C++ declaration, but only ones written in the imported header
    // itself are reported; skipping standard-library internals is expected, not newsworthy.
    void countSkippedCxxDecl(const clang::Decl& decl) {
        if (sourceManager.getFileID(sourceManager.getExpansionLoc(decl.getLocation())) == sourceManager.getMainFileID()) skippedCxxDecls++;
    }

public:
    unsigned getSkippedCxxDecls() const { return skippedCxxDecls; }
    static void resetAnonymousRecordCount() { anonymousRecordCount = 0; }
};

// Value of a macro body that evaluated to a number: either an integer at its C
// type's width or a double. Integer results carry their C type so the imported
// constant gets the same type the macro has in C.
struct EvaluatedConstant {
    static EvaluatedConstant fromInt(llvm::APSInt value, clang::QualType type) {
        EvaluatedConstant result;
        result.intValue = std::move(value);
        result.intType = type;
        return result;
    }
    static EvaluatedConstant fromFloat(llvm::APFloat value) {
        EvaluatedConstant result;
        result.isFloat = true;
        result.floatValue = value;
        return result;
    }

    bool isFloat = false;
    llvm::APSInt intValue{32, true};
    clang::QualType intType;
    llvm::APFloat floatValue{0.0};
};

// Picks the C type of an integer literal from its suffixes and value, or null
// when it fits no standard integer type.
static clang::QualType cTypeForIntegerValue(const llvm::APInt& rawValue, bool isUnsigned, bool isLong, bool isLongLong, unsigned radix,
                                            clang::ASTContext& context) {
    auto fitsSigned = [&](unsigned width) { return rawValue.isSignedIntN(width); };
    auto fitsUnsigned = [&](unsigned width) { return rawValue.isIntN(width); };
    unsigned intWidth = context.getTypeSize(context.IntTy), longWidth = context.getTypeSize(context.LongTy),
             longLongWidth = context.getTypeSize(context.LongLongTy);
    bool hexOrOctal = radix != 10;
    clang::QualType type;
    if (!isUnsigned && !isLong && !isLongLong) {
        if (fitsSigned(intWidth))
            type = context.IntTy;
        else if (hexOrOctal && fitsUnsigned(intWidth))
            type = context.UnsignedIntTy;
        else if (fitsSigned(longWidth))
            type = context.LongTy;
        else if (hexOrOctal && fitsUnsigned(longWidth))
            type = context.UnsignedLongTy;
        else if (fitsSigned(longLongWidth))
            type = context.LongLongTy;
        else if (fitsUnsigned(longLongWidth))
            type = context.UnsignedLongLongTy;
    } else if (isUnsigned && !isLong && !isLongLong) {
        if (fitsUnsigned(intWidth))
            type = context.UnsignedIntTy;
        else if (fitsUnsigned(longWidth))
            type = context.UnsignedLongTy;
        else if (fitsUnsigned(longLongWidth))
            type = context.UnsignedLongLongTy;
    } else if (!isUnsigned && isLong) {
        if (fitsSigned(longWidth))
            type = context.LongTy;
        else if (hexOrOctal && fitsUnsigned(longWidth))
            type = context.UnsignedLongTy;
        else if (fitsSigned(longLongWidth))
            type = context.LongLongTy;
        else if (hexOrOctal && fitsUnsigned(longLongWidth))
            type = context.UnsignedLongLongTy;
    } else if (isUnsigned && isLong) {
        if (fitsUnsigned(longWidth))
            type = context.UnsignedLongTy;
        else if (fitsUnsigned(longLongWidth))
            type = context.UnsignedLongLongTy;
    } else if (!isUnsigned && isLongLong) {
        if (fitsSigned(longLongWidth))
            type = context.LongLongTy;
        else if (hexOrOctal && fitsUnsigned(longLongWidth))
            type = context.UnsignedLongLongTy;
    } else if (fitsUnsigned(longLongWidth)) {
        type = context.UnsignedLongLongTy;
    }
    return type;
}

// Integer ranks for usual arithmetic conversions: int, unsigned, long,
// unsigned long, long long, unsigned long long.
static int intRank(clang::QualType type) {
    switch (llvm::cast<clang::BuiltinType>(type.getCanonicalType())->getKind()) {
    case clang::BuiltinType::Int:
        return 0;
    case clang::BuiltinType::UInt:
        return 1;
    case clang::BuiltinType::Long:
        return 2;
    case clang::BuiltinType::ULong:
        return 3;
    case clang::BuiltinType::LongLong:
        return 4;
    case clang::BuiltinType::ULongLong:
        return 5;
    default:
        llvm_unreachable("integer ranks cover only the standard integer types");
    }
}

static clang::QualType intTypeForRank(int rank, clang::ASTContext& context) {
    switch (rank) {
    case 0:
        return context.IntTy;
    case 1:
        return context.UnsignedIntTy;
    case 2:
        return context.LongTy;
    case 3:
        return context.UnsignedLongTy;
    case 4:
        return context.LongLongTy;
    default:
        return context.UnsignedLongLongTy;
    }
}

// Usual arithmetic conversions (C11 6.3.1.8) for two integer ranks.
static int commonIntRank(int a, int b, clang::ASTContext& context) {
    if (a == b) return a;
    unsigned widthA = context.getTypeSize(intTypeForRank(a, context)), widthB = context.getTypeSize(intTypeForRank(b, context));
    bool signedA = (a % 2 == 0), signedB = (b % 2 == 0);
    if (signedA == signedB) {
        if (widthA != widthB) return widthA > widthB ? a : b;
        return a > b ? a : b;
    }
    int unsignedRank = signedA ? b : a, signedRank = signedA ? a : b;
    unsigned unsignedWidth = signedA ? widthB : widthA, signedWidth = signedA ? widthA : widthB;
    if (unsignedWidth > signedWidth) return unsignedRank;
    if (signedWidth > unsignedWidth) return signedRank;
    return signedRank + 1; // Same width: the unsigned counterpart of the signed type.
}

// Converts an integer value to the given width and signedness, preserving the
// bit pattern like C conversions do (out-of-range values wrap).
static llvm::APSInt convertInt(llvm::APSInt value, unsigned width, bool isUnsigned) {
    value = value.extOrTrunc(width);
    value.setIsUnsigned(isUnsigned);
    return value;
}

struct MacroImporter final : clang::PPCallbacks {
    MacroImporter(Module& module, CToCxConverter& cToCxConverter, clang::CompilerInstance& compilerInstance, Typechecker& typechecker)
    : module(module), cToCxConverter(cToCxConverter), compilerInstance(compilerInstance), typechecker(typechecker) {}

    void MacroDefined(const clang::Token& name, const clang::MacroDirective* macro) override {
        auto* info = macro->getMacroInfo();
        if (info->isFunctionLike()) return;
        llvm::StringRef macroName = name.getIdentifierInfo()->getName();
        llvm::ArrayRef<clang::Token> tokens = info->tokens();
        if (tokens.size() == 1) {
            auto& token = tokens[0];
            switch (token.getKind()) {
            case clang::tok::identifier: {
                llvm::StringRef target = token.getIdentifierInfo()->getName();
                module.addIdentifierReplacement(macroName, target);
                // Aliases of known constants are constants themselves, so later
                // expressions can use them (`#define B A` then `#define C (B+1)`).
                if (auto it = importedConstants.find(target); it != importedConstants.end()) importedConstants[macroName] = it->second;
                break;
            }
            case clang::tok::numeric_constant:
                importMacroConstant(macroName, token);
                break;
            case clang::tok::char_constant:
                importCharConstant(macroName, token);
                break;
            case clang::tok::string_literal:
                importStringConstant(macroName, tokens);
                break;
            default:
                break;
            }
            return;
        }
        if (!tokens.empty() && llvm::all_of(tokens, [](auto& token) { return token.getKind() == clang::tok::string_literal; })) {
            importStringConstant(macroName, tokens); // Implicitly concatenated strings import as one.
            return;
        }
        // Compound literals (`(Color){ 245, ... }`) import as constructor calls.
        if (tryImportCompoundLiteral(macroName, tokens)) return;
        // Anything else imports only if it evaluates to a number. Failures skip
        // silently: most multi-token macros aren't constants (`#define BEGIN {`).
        if (auto value = evaluateMacroExpression(tokens)) importEvaluatedConstant(macroName, *value);
    }

    // Imports stashed compound literals as constructor calls. Unknown types,
    // designators, partial inits, and types without an autogenerated constructor
    // all skip silently, like the scalar evaluator's failures. Runs after parsing
    // so structs resolve wherever they are defined; compound-referencing-compound
    // resolves in definition order.
    void flushPendingCompounds() {
        llvm::SaveAndRestore setModule(typechecker.currentModule, &module);
        llvm::SaveAndRestore setSourceFile(typechecker.currentSourceFile, &module.sourceFiles.front());
        for (auto& pending : pendingCompounds) {
            TypeDecl* typeDecl = resolveCompoundType(pending.typeName);
            if (!typeDecl || !checkCompoundArity(*typeDecl, pending.args)) continue;
            auto* call = makeAST<CallExpr>(makeAST<VarExpr>(typeDecl->getName(), Location()), std::move(pending.args), AstVector<GenericArg>(), Location());
            // Cannot throw through the Clang API; conversion failures skip.
            try {
                typechecker.typecheckExpr(*call);
            } catch (...) {
                continue;
            }
            if (!typechecker.isSupportedConstInitializer(*call)) continue;
            cToCxConverter.addConstantToSymbolTable(pending.name, call, typeDecl->getType());
        }
        pendingCompounds.clear();
    }

private:
    // Integer literal and its C type; the type is null when the value fits no C integer type.
    struct ParsedIntegerLiteral {
        llvm::APInt rawValue{128, 0};
        clang::QualType type;
    };

    // Parses an integer literal token; nullopt unless it is a well-formed integer literal.
    std::optional<ParsedIntegerLiteral> parseIntegerLiteral(const clang::Token& token) {
        // Parsed with NumericLiteralParser instead of Sema::ActOnNumericConstant: the latter
        // crashes in Sema::Diag when the literal needs a diagnostic (e.g. out of range).
        auto spelling = clang::Lexer::getSpelling(token, compilerInstance.getSourceManager(), compilerInstance.getLangOpts());
        clang::NumericLiteralParser parser(spelling, token.getLocation(), compilerInstance.getSourceManager(), compilerInstance.getLangOpts(),
                                           compilerInstance.getTarget(), compilerInstance.getDiagnostics());
        if (parser.hadError || parser.hasUDSuffix() || !parser.isIntegerLiteral()) return std::nullopt;
        if (parser.isBitInt || parser.isSizeT) return std::nullopt;
        llvm::APInt rawValue(128, 0);
        if (parser.GetIntegerValue(rawValue)) return std::nullopt;
        clang::QualType type =
            cTypeForIntegerValue(rawValue, parser.isUnsigned, parser.isLong, parser.isLongLong, parser.getRadix(), compilerInstance.getASTContext());
        return ParsedIntegerLiteral{std::move(rawValue), type};
    }

    // Parses a float literal token to double, honoring f/l suffixes; nullopt unless well-formed and representable.
    std::optional<llvm::APFloat> parseFloatLiteral(const clang::Token& token) {
        auto spelling = clang::Lexer::getSpelling(token, compilerInstance.getSourceManager(), compilerInstance.getLangOpts());
        clang::NumericLiteralParser parser(spelling, token.getLocation(), compilerInstance.getSourceManager(), compilerInstance.getLangOpts(),
                                           compilerInstance.getTarget(), compilerInstance.getDiagnostics());
        if (parser.hadError || parser.hasUDSuffix() || !parser.isFloatingLiteral()) return std::nullopt;
        auto& context = compilerInstance.getASTContext();
        const llvm::fltSemantics* semantics = &context.getFloatTypeSemantics(context.DoubleTy);
        // Only a suffix ends a float spelling in a letter (exponents always end in digits).
        char last = spelling.back();
        if (last == 'f' || last == 'F')
            semantics = &context.getFloatTypeSemantics(context.FloatTy);
        else if (last == 'l' || last == 'L')
            semantics = &context.getFloatTypeSemantics(context.LongDoubleTy);
        llvm::APFloat value(*semantics);
        auto status = parser.GetFloatValue(value, llvm::RoundingMode::NearestTiesToEven);
        if (status != llvm::APFloat::opOK && status != llvm::APFloat::opInexact) return std::nullopt;
        // The constant's type is float64, so narrow suffix-precision values to double.
        bool losesInfo = false;
        value.convert(llvm::APFloat::IEEEdouble(), llvm::RoundingMode::NearestTiesToEven, &losesInfo);
        if (!value.isFinite()) return std::nullopt;
        return value;
    }

    void importMacroConstant(llvm::StringRef name, const clang::Token& token) {
        if (auto parsed = parseIntegerLiteral(token)) {
            if (parsed->type.isNull()) {
                WARN(Location(), "skipping C integer constant '" << name << "': value does not fit 64 bits");
                return;
            }
            llvm::APSInt value(parsed->rawValue, parsed->type->isUnsignedIntegerType());
            cToCxConverter.addIntegerConstantToSymbolTable(name, value, parsed->type);
            auto& context = compilerInstance.getASTContext();
            importedConstants[name] =
                EvaluatedConstant::fromInt(convertInt(value, context.getTypeSize(parsed->type), parsed->type->isUnsignedIntegerType()), parsed->type);
            return;
        }
        if (auto value = parseFloatLiteral(token)) {
            cToCxConverter.addFloatConstantToSymbolTable(name, *value);
            importedConstants[name] = EvaluatedConstant::fromFloat(*value);
        }
    }

    void importCharConstant(llvm::StringRef name, const clang::Token& token) {
        auto parsed = parseCharLiteral(token);
        auto value = parsed ? charValueOf(*parsed) : std::nullopt;
        if (!value) return;
        cToCxConverter.addCharConstantToSymbolTable(name, *value);
        importedConstants[name] = *parsed;
    }

    // The character value when the evaluated constant fits a char.
    static std::optional<char> charValueOf(const EvaluatedConstant& value) {
        if (value.isFloat || !value.intValue.isIntN(8)) return std::nullopt;
        return char(value.intValue.getZExtValue());
    }

    void importStringConstant(llvm::StringRef name, llvm::ArrayRef<clang::Token> tokens) {
        // Null diagnostics: malformed strings in skipped macros stay silent.
        clang::StringLiteralParser parser(tokens, compilerInstance.getSourceManager(), compilerInstance.getLangOpts(), compilerInstance.getTarget(), nullptr);
        if (parser.hadError || !parser.isOrdinary() || !parser.getUDSuffix().empty()) return;
        cToCxConverter.addStringConstantToSymbolTable(name, parser.GetString());
    }

    void importEvaluatedConstant(llvm::StringRef name, const EvaluatedConstant& value) {
        if (value.isFloat) {
            cToCxConverter.addFloatConstantToSymbolTable(name, value.floatValue);
        } else {
            cToCxConverter.addIntegerConstantToSymbolTable(name, value.intValue, value.intType);
        }
        importedConstants[name] = value;
    }

    // Stashes `(Type){ ... }` for import after parsing; false when the tokens are no
    // compound literal. Stashing must defer: MacroDefined fires before the
    // preceding declaration converts, so the struct may not exist yet. Scalar
    // elements evaluate now (textual order, like the scalar evaluator); type
    // resolution and checking wait for flushPendingCompounds.
    bool tryImportCompoundLiteral(llvm::StringRef macroName, llvm::ArrayRef<clang::Token> tokens) {
        size_t pos = 0;
        auto typeName = parseCompoundTypeName(tokens, pos);
        if (!typeName) return false;
        auto args = parseCompoundLiteralArgs(tokens, pos);
        if (!args || pos != tokens.size()) return false;
        pendingCompounds.push_back({macroName.str(), std::move(*typeName), std::move(*args)});
        return true;
    }

    // Parses `(Ident)` or `(struct Ident)` to the type name.
    static std::optional<std::string> parseCompoundTypeName(llvm::ArrayRef<clang::Token> tokens, size_t& pos) {
        if (pos >= tokens.size() || tokens[pos].getKind() != clang::tok::l_paren) return std::nullopt;
        size_t p = pos + 1;
        if (p < tokens.size() && tokens[p].getKind() == clang::tok::kw_struct) ++p;
        if (p >= tokens.size() || tokens[p].getKind() != clang::tok::identifier) return std::nullopt;
        std::string name = tokens[p].getIdentifierInfo()->getName().str();
        ++p;
        if (p >= tokens.size() || tokens[p].getKind() != clang::tok::r_paren) return std::nullopt;
        pos = p + 1;
        return name;
    }

    static ConstructorDecl* findAutogeneratedConstructor(TypeDecl& typeDecl) {
        for (auto* ctor : typeDecl.getConstructors()) {
            if (ctor->isAutogenerated) return ctor;
        }
        return nullptr;
    }

    // Resolves a compound type name to an imported struct with an autogenerated constructor.
    TypeDecl* resolveCompoundType(llvm::StringRef name) {
        for (Decl* decl : module.symbolTable.findInTopLevelScope(name)) {
            if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                if (!typeDecl->isStruct()) return nullptr;
                return findAutogeneratedConstructor(*typeDecl) ? typeDecl : nullptr;
            }
        }
        return nullptr;
    }

    // Argument counts must be pre-validated: arity errors report (print) instead of
    // throwing, so they cannot skip silently. Nested literals validate recursively.
    bool checkCompoundArity(TypeDecl& typeDecl, const AstVector<NamedValue>& args) {
        auto* constructor = findAutogeneratedConstructor(typeDecl);
        if (!constructor) return false;
        size_t required = 0;
        for (auto& param : constructor->getParams()) {
            if (!param.defaultValue) ++required;
        }
        if (args.size() < required || args.size() > constructor->getParams().size()) return false;
        for (auto& arg : args) {
            auto* nested = llvm::dyn_cast<CallExpr>(arg.value);
            auto* callee = nested ? llvm::dyn_cast<VarExpr>(nested->callee) : nullptr;
            if (callee) {
                TypeDecl* nestedType = resolveCompoundType(callee->identifier);
                if (!nestedType || !checkCompoundArity(*nestedType, nested->args)) return false;
            }
        }
        return true;
    }

    // Parses `{ a, b, ... }` into constructor arguments. A trailing comma is allowed.
    std::optional<AstVector<NamedValue>> parseCompoundLiteralArgs(llvm::ArrayRef<clang::Token> tokens, size_t& pos) {
        if (pos >= tokens.size() || tokens[pos].getKind() != clang::tok::l_brace) return std::nullopt;
        ++pos;
        AstVector<NamedValue> args;
        auto pushElement = [&](size_t start, size_t end) {
            auto element = parseCompoundElement(tokens.slice(start, end - start));
            if (!element) return false;
            args.emplace_back(*element);
            return true;
        };
        size_t elementStart = pos;
        unsigned depth = 0;
        while (pos < tokens.size()) {
            auto kind = tokens[pos].getKind();
            if (kind == clang::tok::l_brace || kind == clang::tok::l_paren || kind == clang::tok::l_square) {
                ++depth;
            } else if (kind == clang::tok::r_brace || kind == clang::tok::r_paren || kind == clang::tok::r_square) {
                if (depth == 0) break;
                --depth;
            } else if (kind == clang::tok::comma && depth == 0) {
                if (!pushElement(elementStart, pos)) return std::nullopt;
                elementStart = pos + 1;
            }
            ++pos;
        }
        if (pos >= tokens.size() || tokens[pos].getKind() != clang::tok::r_brace) return std::nullopt;
        if (elementStart != pos && !pushElement(elementStart, pos)) return std::nullopt;
        ++pos;
        return args;
    }

    // One initializer element: a nested compound literal, a scalar constant
    // expression, or a lone identifier resolving at flush (a compound imported above).
    std::optional<Expr*> parseCompoundElement(llvm::ArrayRef<clang::Token> tokens) {
        size_t pos = 0;
        if (auto typeName = parseCompoundTypeName(tokens, pos)) {
            auto args = parseCompoundLiteralArgs(tokens, pos);
            if (!args || pos != tokens.size()) return std::nullopt;
            return makeAST<CallExpr>(makeAST<VarExpr>(*typeName, Location()), std::move(*args), AstVector<GenericArg>(), Location());
        }
        if (tokens.size() == 1 && tokens.front().getKind() == clang::tok::char_constant) {
            auto parsed = parseCharLiteral(tokens.front());
            auto value = parsed ? charValueOf(*parsed) : std::nullopt;
            if (!value) return std::nullopt;
            return makeAST<CharacterLiteralExpr>(*value, Location());
        }
        if (auto value = evaluateMacroExpression(tokens)) {
            if (Expr* literal = makeElementLiteral(*value)) return literal;
            return std::nullopt;
        }
        if (tokens.size() == 1 && tokens.front().getKind() == clang::tok::identifier) {
            return makeAST<VarExpr>(tokens.front().getIdentifierInfo()->getName(), Location());
        }
        return std::nullopt;
    }

    // Converts an evaluated element to a literal; null when the integer fits no 64-bit cx type.
    static Expr* makeElementLiteral(const EvaluatedConstant& value) {
        if (value.isFloat) return makeAST<FloatLiteralExpr>(value.floatValue, Location());
        return makeIntLiteralExpr(value.intValue);
    }

    std::optional<EvaluatedConstant> evaluateMacroExpression(llvm::ArrayRef<clang::Token> tokens) {
        size_t pos = 0;
        auto result = parseBinaryExpr(tokens, pos, 1);
        if (!result || pos != tokens.size()) return std::nullopt;
        return result;
    }

    // Precedence-climbing parser for constant expressions; levels follow C, all left-associative.
    std::optional<EvaluatedConstant> parseBinaryExpr(llvm::ArrayRef<clang::Token> tokens, size_t& pos, unsigned minPrec) {
        auto lhs = parseUnaryExpr(tokens, pos);
        if (!lhs) return std::nullopt;
        while (pos < tokens.size()) {
            unsigned prec = binaryPrecedence(tokens[pos].getKind());
            if (prec < minPrec) break;
            auto op = tokens[pos++].getKind();
            auto rhs = parseBinaryExpr(tokens, pos, prec + 1);
            if (!rhs) return std::nullopt;
            lhs = applyBinaryOp(op, *lhs, *rhs);
            if (!lhs) return std::nullopt;
        }
        return lhs;
    }

    static unsigned binaryPrecedence(clang::tok::TokenKind kind) {
        switch (kind) {
        case clang::tok::pipepipe:
            return 1;
        case clang::tok::ampamp:
            return 2;
        case clang::tok::pipe:
            return 3;
        case clang::tok::caret:
            return 4;
        case clang::tok::amp:
            return 5;
        case clang::tok::equalequal:
        case clang::tok::exclaimequal:
            return 6;
        case clang::tok::less:
        case clang::tok::lessequal:
        case clang::tok::greater:
        case clang::tok::greaterequal:
            return 7;
        case clang::tok::lessless:
        case clang::tok::greatergreater:
            return 8;
        case clang::tok::plus:
        case clang::tok::minus:
            return 9;
        case clang::tok::star:
        case clang::tok::slash:
        case clang::tok::percent:
            return 10;
        default:
            return 0;
        }
    }

    std::optional<EvaluatedConstant> parseUnaryExpr(llvm::ArrayRef<clang::Token> tokens, size_t& pos) {
        if (pos < tokens.size()) {
            auto kind = tokens[pos].getKind();
            if (kind == clang::tok::plus || kind == clang::tok::minus || kind == clang::tok::tilde || kind == clang::tok::exclaim) {
                ++pos;
                auto operand = parseUnaryExpr(tokens, pos);
                if (!operand) return std::nullopt;
                return applyUnaryOp(kind, *operand);
            }
        }
        return parsePrimaryExpr(tokens, pos);
    }

    std::optional<EvaluatedConstant> parsePrimaryExpr(llvm::ArrayRef<clang::Token> tokens, size_t& pos) {
        if (pos >= tokens.size()) return std::nullopt;
        auto& token = tokens[pos];
        switch (token.getKind()) {
        case clang::tok::numeric_constant:
            ++pos;
            return parseNumericLiteral(token);
        case clang::tok::char_constant:
            ++pos;
            return parseCharLiteral(token);
        case clang::tok::identifier: {
            llvm::StringRef ident = token.getIdentifierInfo()->getName();
            ++pos;
            // Infinity and NaN builtins, which HUGE_VAL, INFINITY, and NAN expand to.
            if (ident == "__builtin_huge_val" || ident == "__builtin_huge_valf" || ident == "__builtin_huge_vall" || ident == "__builtin_inf"
                || ident == "__builtin_inff" || ident == "__builtin_infl") {
                if (!skipBalancedParens(tokens, pos)) return std::nullopt;
                return EvaluatedConstant::fromFloat(llvm::APFloat::getInf(llvm::APFloat::IEEEdouble()));
            }
            if (ident == "__builtin_nan" || ident == "__builtin_nanf" || ident == "__builtin_nanl") {
                if (!skipBalancedParens(tokens, pos)) return std::nullopt;
                return EvaluatedConstant::fromFloat(llvm::APFloat::getNaN(llvm::APFloat::IEEEdouble()));
            }
            auto it = importedConstants.find(ident);
            if (it == importedConstants.end()) return std::nullopt;
            return it->second;
        }
        case clang::tok::l_paren: {
            ++pos;
            auto inner = parseBinaryExpr(tokens, pos, 1);
            if (!inner || pos >= tokens.size() || tokens[pos].getKind() != clang::tok::r_paren) return std::nullopt;
            ++pos;
            return inner;
        }
        default:
            return std::nullopt;
        }
    }

    // Skips one balanced paren group (builtin call arguments); false when unbalanced.
    static bool skipBalancedParens(llvm::ArrayRef<clang::Token> tokens, size_t& pos) {
        if (pos >= tokens.size() || tokens[pos].getKind() != clang::tok::l_paren) return false;
        unsigned depth = 0;
        while (pos < tokens.size()) {
            auto kind = tokens[pos++].getKind();
            if (kind == clang::tok::l_paren)
                ++depth;
            else if (kind == clang::tok::r_paren && --depth == 0)
                return true;
        }
        return false;
    }

    std::optional<EvaluatedConstant> parseNumericLiteral(const clang::Token& token) {
        if (auto parsed = parseIntegerLiteral(token)) {
            if (parsed->type.isNull()) return std::nullopt;
            auto& context = compilerInstance.getASTContext();
            return EvaluatedConstant::fromInt(convertInt(llvm::APSInt(parsed->rawValue, parsed->type->isUnsignedIntegerType()),
                                                         context.getTypeSize(parsed->type), parsed->type->isUnsignedIntegerType()),
                                              parsed->type);
        }
        if (auto value = parseFloatLiteral(token)) return EvaluatedConstant::fromFloat(*value);
        return std::nullopt;
    }

    std::optional<EvaluatedConstant> parseCharLiteral(const clang::Token& token) {
        auto spelling = clang::Lexer::getSpelling(token, compilerInstance.getSourceManager(), compilerInstance.getLangOpts());
        clang::CharLiteralParser parser(spelling.data(), spelling.data() + spelling.size(), token.getLocation(), compilerInstance.getPreprocessor(),
                                        token.getKind());
        if (parser.hadError() || !parser.isOrdinary() || parser.isMultiChar() || !parser.getUDSuffix().empty()) return std::nullopt;
        // C character constants have type int.
        auto& context = compilerInstance.getASTContext();
        return EvaluatedConstant::fromInt(convertInt(llvm::APSInt(llvm::APInt(128, parser.getValue()), false), context.getTypeSize(context.IntTy), false),
                                          context.IntTy);
    }

    EvaluatedConstant makeBoolConstant(bool value) {
        auto& context = compilerInstance.getASTContext();
        return EvaluatedConstant::fromInt(llvm::APSInt(llvm::APInt(context.getTypeSize(context.IntTy), value ? 1 : 0), false), context.IntTy);
    }

    static llvm::APFloat toDouble(const EvaluatedConstant& value) {
        if (value.isFloat) return value.floatValue;
        llvm::APFloat result(0.0);
        result.convertFromAPInt(value.intValue, value.intValue.isSigned(), llvm::RoundingMode::NearestTiesToEven);
        return result;
    }

    std::optional<EvaluatedConstant> applyUnaryOp(clang::tok::TokenKind op, const EvaluatedConstant& operand) {
        if (operand.isFloat) {
            switch (op) {
            case clang::tok::plus:
                return operand;
            case clang::tok::minus: {
                llvm::APFloat value = operand.floatValue;
                value.changeSign();
                return EvaluatedConstant::fromFloat(value);
            }
            case clang::tok::exclaim:
                return makeBoolConstant(operand.floatValue.isZero());
            default:
                return std::nullopt; // No bitwise ops on floats.
            }
        }
        switch (op) {
        case clang::tok::plus:
            return operand;
        case clang::tok::minus:
            return EvaluatedConstant::fromInt(-operand.intValue, operand.intType);
        case clang::tok::tilde:
            return EvaluatedConstant::fromInt(~operand.intValue, operand.intType);
        case clang::tok::exclaim:
            return makeBoolConstant(operand.intValue.isZero());
        default:
            return std::nullopt;
        }
    }

    std::optional<EvaluatedConstant> applyBinaryOp(clang::tok::TokenKind op, const EvaluatedConstant& lhs, const EvaluatedConstant& rhs) {
        if (lhs.isFloat || rhs.isFloat) return applyFloatBinaryOp(op, toDouble(lhs), toDouble(rhs));
        auto& context = compilerInstance.getASTContext();
        // Shift counts keep their own type; every other operator converts both
        // sides to the common type first (C11 6.3.1.8, 6.5.7).
        int rank = (op == clang::tok::lessless || op == clang::tok::greatergreater) ? intRank(lhs.intType)
                                                                                    : commonIntRank(intRank(lhs.intType), intRank(rhs.intType), context);
        clang::QualType type = intTypeForRank(rank, context);
        unsigned width = context.getTypeSize(type);
        bool isSigned = !type->isUnsignedIntegerType();
        llvm::APSInt a = convertInt(lhs.intValue, width, !isSigned);
        llvm::APSInt b = convertInt(rhs.intValue, width, !isSigned);
        auto intResult = [&](llvm::APSInt value) { return EvaluatedConstant::fromInt(std::move(value), type); };
        switch (op) {
        case clang::tok::plus:
            return intResult(a + b);
        case clang::tok::minus:
            return intResult(a - b);
        case clang::tok::star:
            return intResult(a * b);
        case clang::tok::slash:
        case clang::tok::percent:
            if (b.isZero()) return std::nullopt;
            if (isSigned && b.isAllOnes() && a.isMinSignedValue()) return std::nullopt; // INT_MIN / -1 traps.
            return intResult(op == clang::tok::slash ? a / b : a % b);
        case clang::tok::lessless:
        case clang::tok::greatergreater: {
            if (rhs.intValue.isSigned() && rhs.intValue.isNegative()) return std::nullopt;
            uint64_t count = rhs.intValue.getZExtValue();
            if (count >= width) return std::nullopt;
            if (op == clang::tok::lessless) return intResult(a << (unsigned)count);
            return intResult(a >> (unsigned)count);
        }
        case clang::tok::amp:
            return intResult(a & b);
        case clang::tok::pipe:
            return intResult(a | b);
        case clang::tok::caret:
            return intResult(a ^ b);
        case clang::tok::less:
            return makeBoolConstant(a < b);
        case clang::tok::lessequal:
            return makeBoolConstant(a <= b);
        case clang::tok::greater:
            return makeBoolConstant(a > b);
        case clang::tok::greaterequal:
            return makeBoolConstant(a >= b);
        case clang::tok::equalequal:
            return makeBoolConstant(a == b);
        case clang::tok::exclaimequal:
            return makeBoolConstant(a != b);
        case clang::tok::ampamp:
            return makeBoolConstant(!a.isZero() && !b.isZero());
        case clang::tok::pipepipe:
            return makeBoolConstant(!a.isZero() || !b.isZero());
        default:
            return std::nullopt;
        }
    }

    std::optional<EvaluatedConstant> applyFloatBinaryOp(clang::tok::TokenKind op, llvm::APFloat lhs, llvm::APFloat rhs) {
        llvm::APFloat::opStatus status = llvm::APFloat::opOK;
        switch (op) {
        case clang::tok::plus:
            status = lhs.add(rhs, llvm::RoundingMode::NearestTiesToEven);
            break;
        case clang::tok::minus:
            status = lhs.subtract(rhs, llvm::RoundingMode::NearestTiesToEven);
            break;
        case clang::tok::star:
            status = lhs.multiply(rhs, llvm::RoundingMode::NearestTiesToEven);
            break;
        case clang::tok::slash:
            status = lhs.divide(rhs, llvm::RoundingMode::NearestTiesToEven);
            break;
        case clang::tok::less:
        case clang::tok::lessequal:
        case clang::tok::greater:
        case clang::tok::greaterequal:
        case clang::tok::equalequal:
        case clang::tok::exclaimequal: {
            auto cmp = lhs.compare(rhs);
            switch (op) {
            case clang::tok::less:
                return makeBoolConstant(cmp == llvm::APFloat::cmpLessThan);
            case clang::tok::lessequal:
                return makeBoolConstant(cmp == llvm::APFloat::cmpLessThan || cmp == llvm::APFloat::cmpEqual);
            case clang::tok::greater:
                return makeBoolConstant(cmp == llvm::APFloat::cmpGreaterThan);
            case clang::tok::greaterequal:
                return makeBoolConstant(cmp == llvm::APFloat::cmpGreaterThan || cmp == llvm::APFloat::cmpEqual);
            case clang::tok::equalequal:
                return makeBoolConstant(cmp == llvm::APFloat::cmpEqual);
            default:
                return makeBoolConstant(cmp != llvm::APFloat::cmpEqual);
            }
        }
        case clang::tok::ampamp:
            return makeBoolConstant(!lhs.isZero() && !rhs.isZero());
        case clang::tok::pipepipe:
            return makeBoolConstant(!lhs.isZero() || !rhs.isZero());
        default:
            return std::nullopt; // No % or bitwise ops on floats.
        }
        if (status & (llvm::APFloat::opInvalidOp | llvm::APFloat::opDivByZero | llvm::APFloat::opOverflow)) return std::nullopt;
        return EvaluatedConstant::fromFloat(lhs);
    }

private:
    struct PendingCompound {
        std::string name;
        std::string typeName;
        AstVector<NamedValue> args;
    };

    Module& module;
    CToCxConverter& cToCxConverter;
    clang::CompilerInstance& compilerInstance;
    Typechecker& typechecker;
    std::vector<PendingCompound> pendingCompounds;
    // Integer and float constants imported so far, for identifiers in later macro
    // bodies. Only macros defined above the use are visible, and #undef does not
    // remove them; both are rare enough in practice to leave unhandled.
    llvm::StringMap<EvaluatedConstant> importedConstants;
};

// Silently drops errors in system headers.
struct ErrorIgnoringTextDiagPrinter final : clang::TextDiagnosticPrinter {
    ErrorIgnoringTextDiagPrinter(llvm::raw_ostream& os, clang::DiagnosticOptions& diags, bool ownsOutputStream = false)
    : clang::TextDiagnosticPrinter(os, diags, ownsOutputStream), srcManager(nullptr) {}

    void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic& info) override {
        auto loc = info.getLocation();
        if (loc.isValid() && srcManager->isInSystemHeader(loc)) return;
        TextDiagnosticPrinter::HandleDiagnostic(level, info);
    }

    clang::SourceManager* srcManager;
};

} // namespace

bool cx::importCHeader(SourceFile& importer, ImportDecl& importDecl, Typechecker& typechecker) {
    llvm::StringRef headerName = importDecl.target;
    if (Module* cached = Module::findImportedModule(headerName)) {
        importer.addImportedModule(cached);
        return true;
    }

    clang::CompilerInstance ci;
    clang::DiagnosticOptions diagOpts;
    auto* diagClient = new ErrorIgnoringTextDiagPrinter(llvm::errs(), diagOpts);
    ci.createDiagnostics(diagClient);

    bool cxxMode = isCxxHeader(headerName);
    auto args = map(typechecker.options.cflags, [](auto& cflag) { return cflag.c_str(); });
    args.push_back("-fgnuc-version=4.2.1"); // Enable compatibility with GCC macros in imported headers.
#ifdef _WIN32
    args.push_back("-fms-extensions"); // Needed to parse MSVC system headers.
#endif
    if (cxxMode) {
        args.push_back("-x");
        args.push_back("c++");
        if (!llvm::any_of(typechecker.options.cflags, [](auto& cflag) { return cflag.starts_with("-std="); })) {
            args.push_back("-std=c++17");
        }
    }
    clang::CompilerInvocation::CreateFromArgs(ci.getInvocation(), args, ci.getDiagnostics());

    clang::TargetOptions pto;
    pto.Triple = llvm::sys::getDefaultTargetTriple();
    auto targetInfo = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    ci.setTarget(targetInfo);

    ci.createFileManager();
    ci.createSourceManager();
    diagClient->srcManager = &ci.getSourceManager();

    llvm::SmallString<256> importerDirectory;
    fs::real_path(importer.filePath, importerDirectory);
    ci.getHeaderSearchOpts().AddPath(path::parent_path(importerDirectory), clang::frontend::Quoted, false, true);

    auto addSystemPath = [&](llvm::StringRef includePath) {
        ci.getHeaderSearchOpts().AddPath(includePath, clang::frontend::System, false, true);
        ci.getHeaderSearchOpts().AddPath(includePath, clang::frontend::System, false, false);
    };
    for (llvm::StringRef includePath : llvm::concat<const std::string>(typechecker.options.importSearchPaths, getCCompilerSearchPaths()))
        addSystemPath(includePath);
    if (cxxMode)
        for (llvm::StringRef includePath : getCxxCompilerSearchPaths())
            addSystemPath(includePath);
    for (llvm::StringRef frameworkPath : typechecker.options.frameworkSearchPaths) {
        ci.getHeaderSearchOpts().AddPath(frameworkPath, clang::frontend::System, true, true);
        ci.getHeaderSearchOpts().AddPath(frameworkPath, clang::frontend::System, true, false);
    }
    for (llvm::StringRef define : typechecker.options.defines) {
        ci.getPreprocessorOpts().addMacroDef(define);
    }

    ci.createPreprocessor(clang::TU_Complete);
    auto& pp = ci.getPreprocessor();
    pp.getBuiltinInfo().initializeBuiltins(pp.getIdentifierTable(), pp.getLangOpts());

    clang::HeaderSearch& headerSearch = ci.getPreprocessor().getHeaderSearchInfo();
    auto fileEntry = headerSearch.LookupFile(headerName, {}, false, nullptr, nullptr, {}, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!fileEntry) {
        std::string searchDirs;
        for (auto& searchDir : headerSearch.search_dir_range()) {
            searchDirs += '\n';
            searchDirs += searchDir.getName();
        }
        auto language = cxxMode ? "C++" : "C";
        REPORT_ERROR(importDecl.location,
                     "couldn't find " << language << " header file '" << importDecl.target << "' in the following locations:" << searchDirs);
        return false;
    }

    auto headerPath = fileEntry->getFileEntry().tryGetRealPathName();
    if (headerPath.empty()) headerPath = headerName;
    importDecl.importedHeaderPath = internString(headerPath);

    std::string headerModuleName = headerName.str();
    llvm::replace(headerModuleName, '.', '_');
    auto ownedModule = std::make_unique<Module>(std::move(headerModuleName));
    Module* module = ownedModule.get();
    module->isCHeaderImport = true;
    module->isCxxHeaderImport = cxxMode;
    module->addSourceFile(SourceFile(headerPath.str(), module));

    ci.createASTContext();
    std::unique_ptr<clang::MangleContext> mangleContext;
    if (cxxMode) mangleContext.reset(ci.getASTContext().createMangleContext());
    auto cToCxConverter = new CToCxConverter(*module, typechecker, targetInfo, ci.getSourceManager(), cxxMode, mangleContext.get());
    ci.setASTConsumer(std::unique_ptr<CToCxConverter>(cToCxConverter));
    ci.createSema(clang::TU_Complete, nullptr);
    auto macroImporter = std::make_unique<MacroImporter>(*module, *cToCxConverter, ci, typechecker);
    MacroImporter* macroImporterPtr = macroImporter.get();
    pp.addPPCallbacks(std::move(macroImporter));

    // Treating all imported C headers as system code for now, since we have no proper way to differentiate them from normal user code.
    auto fileID = ci.getSourceManager().createFileID(*fileEntry, clang::SourceLocation(), clang::SrcMgr::C_System);
    ci.getSourceManager().setMainFileID(fileID);
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext(), false, clang::TU_Complete, nullptr, /*SkipFunctionBodies*/ true);
    macroImporterPtr->flushPendingCompounds();
    ci.getDiagnosticClient().EndSourceFile();

    if (ci.getDiagnosticClient().getNumErrors() > 0) {
        return false;
    }

    if (auto skipped = cToCxConverter->getSkippedCxxDecls()) {
        WARN(importDecl.location,
             "skipped " << skipped << " C++ declarations in '" << headerName << "' (namespaces, templates, globals, and non-POD types are not supported)");
    }

    importer.addImportedModule(module);
    Module::registerImportedModule(headerName, module);
    ownedModule.release();
    return true;
}

void cx::resetCImportState() {
    CToCxConverter::resetAnonymousRecordCount();
}

#else // CX_NO_C_IMPORT

bool cx::importCHeader(SourceFile&, ImportDecl&, Typechecker&) {
    // C header imports are not supported in builds without the Clang-based
    // importer. Returning false makes the caller report an error.
    return false;
}

void cx::resetCImportState() {}

#endif // CX_NO_C_IMPORT
