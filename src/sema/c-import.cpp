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
            auto cxType = BasicType::get(getName(*recordDecl), {});
            if (auto* typeDecl = toCx(*recordDecl)) bindTypeSpelling(cxType, *typeDecl);
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
            typeDecl->isAnonymousRecord = !anonymousName.empty();
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
                if (!def->getName().empty() && def->isStruct() && !hasAnonymousMember) {
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
        auto initializer = makeAST<IntLiteralExpr>(std::move(value), Location());
        addConstantToSymbolTable(name, initializer, toCx(qualType));
    }

    void addFloatConstantToSymbolTable(llvm::StringRef name, llvm::APFloat value) {
        auto initializer = makeAST<FloatLiteralExpr>(std::move(value), Location());
        addConstantToSymbolTable(name, initializer, Type::getFloat64());
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
                        auto valueExpr = makeAST<IntLiteralExpr>(value, Location());
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

struct MacroImporter final : clang::PPCallbacks {
    MacroImporter(Module& module, CToCxConverter& cToCxConverter, clang::CompilerInstance& compilerInstance)
    : module(module), cToCxConverter(cToCxConverter), compilerInstance(compilerInstance) {}

    void MacroDefined(const clang::Token& name, const clang::MacroDirective* macro) override {
        if (macro->getMacroInfo()->getNumTokens() != 1) return;
        auto& token = macro->getMacroInfo()->getReplacementToken(0);

        switch (token.getKind()) {
        case clang::tok::identifier:
            module.addIdentifierReplacement(name.getIdentifierInfo()->getName(), token.getIdentifierInfo()->getName());
            break;
        case clang::tok::numeric_constant:
            importMacroConstant(name.getIdentifierInfo()->getName(), token);
            break;
        default:
            break;
        }
    }

private:
    void importMacroConstant(llvm::StringRef name, const clang::Token& token) {
        // Parsed with NumericLiteralParser instead of Sema::ActOnNumericConstant: the latter
        // crashes in Sema::Diag when the literal needs a diagnostic (e.g. out of range).
        auto spelling = clang::Lexer::getSpelling(token, compilerInstance.getSourceManager(), compilerInstance.getLangOpts());
        clang::NumericLiteralParser parser(spelling, token.getLocation(), compilerInstance.getSourceManager(), compilerInstance.getLangOpts(),
                                           compilerInstance.getTarget(), compilerInstance.getDiagnostics());
        if (parser.hadError || parser.hasUDSuffix()) return;

        if (parser.isIntegerLiteral()) {
            if (parser.isBitInt || parser.isSizeT) return;
            llvm::APInt rawValue(128, 0);
            if (parser.GetIntegerValue(rawValue)) return;
            auto& context = compilerInstance.getASTContext();
            auto fitsSigned = [&](unsigned width) { return rawValue.isSignedIntN(width); };
            auto fitsUnsigned = [&](unsigned width) { return rawValue.isIntN(width); };
            unsigned intWidth = context.getTypeSize(context.IntTy), longWidth = context.getTypeSize(context.LongTy),
                     longLongWidth = context.getTypeSize(context.LongLongTy);
            bool hexOrOctal = parser.getRadix() != 10;
            clang::QualType type;
            if (!parser.isUnsigned && !parser.isLong && !parser.isLongLong) {
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
            } else if (parser.isUnsigned && !parser.isLong && !parser.isLongLong) {
                if (fitsUnsigned(intWidth))
                    type = context.UnsignedIntTy;
                else if (fitsUnsigned(longWidth))
                    type = context.UnsignedLongTy;
                else if (fitsUnsigned(longLongWidth))
                    type = context.UnsignedLongLongTy;
            } else if (!parser.isUnsigned && parser.isLong) {
                if (fitsSigned(longWidth))
                    type = context.LongTy;
                else if (hexOrOctal && fitsUnsigned(longWidth))
                    type = context.UnsignedLongTy;
                else if (fitsSigned(longLongWidth))
                    type = context.LongLongTy;
                else if (hexOrOctal && fitsUnsigned(longLongWidth))
                    type = context.UnsignedLongLongTy;
            } else if (parser.isUnsigned && parser.isLong) {
                if (fitsUnsigned(longWidth))
                    type = context.UnsignedLongTy;
                else if (fitsUnsigned(longLongWidth))
                    type = context.UnsignedLongLongTy;
            } else if (!parser.isUnsigned && parser.isLongLong) {
                if (fitsSigned(longLongWidth))
                    type = context.LongLongTy;
                else if (hexOrOctal && fitsUnsigned(longLongWidth))
                    type = context.UnsignedLongLongTy;
            } else if (fitsUnsigned(longLongWidth)) {
                type = context.UnsignedLongLongTy;
            }
            if (type.isNull()) {
                WARN(Location(), "skipping C integer constant '" << name << "': value does not fit 64 bits");
                return;
            }
            cToCxConverter.addIntegerConstantToSymbolTable(name, llvm::APSInt(rawValue, type->isUnsignedIntegerType()), type);
        } else if (parser.isFloatingLiteral()) {
            auto& context = compilerInstance.getASTContext();
            const llvm::fltSemantics* semantics = &context.getFloatTypeSemantics(context.DoubleTy);
            // Suffix-less floats always end in a digit, so a trailing letter is a suffix.
            char last = spelling.back();
            if (last == 'f' || last == 'F')
                semantics = &context.getFloatTypeSemantics(context.FloatTy);
            else if (last == 'l' || last == 'L')
                semantics = &context.getFloatTypeSemantics(context.LongDoubleTy);
            llvm::APFloat value(*semantics);
            auto status = parser.GetFloatValue(value, llvm::RoundingMode::NearestTiesToEven);
            if (status != llvm::APFloat::opOK && status != llvm::APFloat::opInexact) return;
            cToCxConverter.addFloatConstantToSymbolTable(name, value);
        }
    }

private:
    Module& module;
    CToCxConverter& cToCxConverter;
    clang::CompilerInstance& compilerInstance;
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
    pp.addPPCallbacks(std::make_unique<MacroImporter>(*module, *cToCxConverter, ci));

    // Treating all imported C headers as system code for now, since we have no proper way to differentiate them from normal user code.
    auto fileID = ci.getSourceManager().createFileID(*fileEntry, clang::SourceLocation(), clang::SrcMgr::C_System);
    ci.getSourceManager().setMainFileID(fileID);
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext(), false, clang::TU_Complete, nullptr, /*SkipFunctionBodies*/ true);
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
