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
#include <clang/AST/DeclGroup.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/Type.h>
#include <clang/Basic/Builtins.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Lex/HeaderSearch.h>
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
    CToCxConverter(Module& module, Typechecker& typechecker, clang::TargetInfo* targetInfo, clang::SourceManager& sourceManager)
    : module(module), typechecker(typechecker), targetInfo(targetInfo), sourceManager(sourceManager) {}

    void Initialize(clang::ASTContext& context) override { astContext = &context; }

    Type getIntTypeByWidth(unsigned widthInBits, bool asSigned) {
        switch (widthInBits) {
        case 8:
            return asSigned ? Type::getInt8() : Type::getUInt8();
        case 16:
            return asSigned ? Type::getInt16() : Type::getUInt16();
        case 32:
            return asSigned ? Type::getInt32() : Type::getUInt32();
        case 64:
            return asSigned ? Type::getInt64() : Type::getUInt64();
        }
        llvm_unreachable("unsupported integer width");
    }

    Type toCx(const clang::BuiltinType& type) {
        switch (type.getKind()) {
        case clang::BuiltinType::Void:
            return Type::getVoid();
        case clang::BuiltinType::Bool:
            return Type::getBool();
        case clang::BuiltinType::Char_S:
        case clang::BuiltinType::Char_U:
            return Type::getChar();
        case clang::BuiltinType::SChar:
            return getIntTypeByWidth(targetInfo->getCharWidth(), true);
        case clang::BuiltinType::UChar:
            return getIntTypeByWidth(targetInfo->getCharWidth(), false);
        case clang::BuiltinType::Short:
            return getIntTypeByWidth(targetInfo->getShortWidth(), true);
        case clang::BuiltinType::UShort:
            return getIntTypeByWidth(targetInfo->getShortWidth(), false);
        case clang::BuiltinType::Int:
            return Type::getInt32();
        case clang::BuiltinType::UInt:
            return Type::getUInt32();
        case clang::BuiltinType::Long:
            return getIntTypeByWidth(targetInfo->getLongWidth(), true);
        case clang::BuiltinType::ULong:
            return getIntTypeByWidth(targetInfo->getLongWidth(), false);
        case clang::BuiltinType::LongLong:
            return getIntTypeByWidth(targetInfo->getLongLongWidth(), true);
        case clang::BuiltinType::ULongLong:
            return getIntTypeByWidth(targetInfo->getLongLongWidth(), false);
        case clang::BuiltinType::Float16:
        case clang::BuiltinType::BFloat16:
            ASSERT(false); // Skipped before conversion; float32 as a fallback.
            return Type::getFloat32();
        case clang::BuiltinType::Float:
            return Type::getFloat32();
        case clang::BuiltinType::Double:
            return Type::getFloat64();
        case clang::BuiltinType::LongDouble:
            return Type::getFloat80();
        case clang::BuiltinType::Int128:
            return Type::getInt128();
        case clang::BuiltinType::UInt128:
            return Type::getUInt128();
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
        auto mutability = qualType.isConstQualified() ? Mutability::Const : Mutability::Mutable;
        auto& type = *qualType.getTypePtr();

        switch (type.getTypeClass()) {
        case clang::Type::Pointer: {
            auto pointeeType = llvm::cast<clang::PointerType>(type).getPointeeType();
            if (pointeeType->isFunctionType()) {
                return OptionalType::get(toCx(pointeeType), mutability);
            }
            return OptionalType::get(PointerType::get(toCx(pointeeType), PointerKind::Pointer, Mutability::Mutable), mutability);
        }
        case clang::Type::Builtin:
            return toCx(llvm::cast<clang::BuiltinType>(type)).withMutability(mutability);
        case clang::Type::Typedef: {
            auto& typedefType = llvm::cast<clang::TypedefType>(type);
            auto desugared = typedefType.desugar();
            if (mutability == Mutability::Const) desugared.addConst();
            return toCx(desugared);
        }
        case clang::Type::PredefinedSugar: {
            auto desugared = llvm::cast<clang::PredefinedSugarType>(type).desugar();
            if (mutability == Mutability::Const) desugared.addConst();
            return toCx(desugared);
        }
        case clang::Type::Record: {
            auto& recordType = llvm::cast<clang::RecordType>(type);
            auto* recordDecl = recordType.getDecl();
            auto cxType = BasicType::get(getName(*recordDecl), {}, mutability);
            llvm::cast<BasicType>(cxType.typeBase)->decl = toCx(*recordDecl);
            return cxType;
        }
        case clang::Type::Paren:
            return toCx(llvm::cast<clang::ParenType>(type).getInnerType());
        case clang::Type::FunctionProto: {
            auto& functionProtoType = llvm::cast<clang::FunctionProtoType>(type);
            auto paramTypes = map(functionProtoType.getParamTypes(), [&](clang::QualType paramType) { return toCx(paramType); });
            return FunctionType::get(toCx(functionProtoType.getReturnType()), std::move(paramTypes), functionProtoType.isVariadic(), mutability);
        }
        case clang::Type::FunctionNoProto: {
            auto& functionNoProtoType = llvm::cast<clang::FunctionNoProtoType>(type);
            return FunctionType::get(toCx(functionNoProtoType.getReturnType()), {}, true, mutability);
        }
        case clang::Type::ConstantArray: {
            auto& constantArrayType = llvm::cast<clang::ConstantArrayType>(type);
            if (!constantArrayType.getSize().isIntN(64)) {
                ERROR(Location(), "array is too large");
            }
            std::vector<GenericArg> args;
            args.emplace_back(toCx(constantArrayType.getElementType()));
            args.push_back(GenericArg::fromInt(constantArrayType.getSize().getLimitedValue(), Location()));
            return BasicType::get("Array", args, mutability);
        }
        case clang::Type::IncompleteArray:
            return ArrayPointerType::get(toCx(llvm::cast<clang::IncompleteArrayType>(type).getElementType()));
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
                return BasicType::get(name, {}, mutability);
            }
        }
        case clang::Type::Vector: {
            auto& vectorType = llvm::cast<clang::VectorType>(type);
            std::vector<GenericArg> args;
            args.emplace_back(toCx(vectorType.getElementType()));
            args.push_back(GenericArg::fromInt(vectorType.getNumElements(), Location()));
            return BasicType::get("Array", args, mutability);
        }
        default:
            WARN(Location(), "unhandled type class '" << type.getTypeClassName() << "' (importing type '" << qualType.getAsString() << "')");
            return Type::getInt32();
        }
    }

    std::optional<FieldDecl> toCx(const clang::FieldDecl& decl, TypeDecl& typeDecl) {
        if (decl.getName().empty()) return std::nullopt;
        return FieldDecl(toCx(decl.getType()), decl.getName(), nullptr, typeDecl, AccessLevel::Default, Location());
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
            auto* typeDecl =
                makeAST<TypeDecl>(tag, getName(recordDecl), std::vector<GenericArg>(), std::vector<Type>(), AccessLevel::Default, module, nullptr, Location());
            it = importedRecordDecls.emplace(canonical, typeDecl).first;

            // Add to symbol table before type-checking so that type-checker finds the struct decl.
            module.addToSymbolTable(typeDecl);
            module.sourceFiles.front().topLevelDecls.push_back(typeDecl);
        }

        // Fill in the fields from the definition once available. This runs at
        // most once per struct; structs without a definition (opaque types
        // used only through pointers) stay empty as before.
        if (const clang::RecordDecl* def = canonical->getDefinition()) {
            if (completedRecordDecls.insert(canonical).second) {
                TypeDecl* typeDecl = it->second;
                typeDecl->packed = def->hasAttr<clang::PackedAttr>();

                bool hasFieldWithAnonymousType = false;
                for (auto* field : def->fields()) {
                    if (auto fieldDecl = toCx(*field, *typeDecl)) {
                        if (fieldDecl->type.isBasicType() && fieldDecl->type.getName().empty()) {
                            hasFieldWithAnonymousType = true;
                        }
                        typeDecl->fields.emplace_back(std::move(*fieldDecl));
                    } else {
                        return nullptr;
                    }
                }

                // Fields with unnameable types not yet supported.
                if (!def->getName().empty() && def->isStruct() && !hasFieldWithAnonymousType) {
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
        return makeAST<VarDecl>(toCx(decl.getType()), decl.getName(), nullptr, nullptr, AccessLevel::Default, module, toCx(decl.getLocation()));
    }

    void addIntegerConstantToSymbolTable(llvm::StringRef name, llvm::APSInt value, clang::QualType qualType) {
        auto initializer = makeAST<IntLiteralExpr>(std::move(value), Location());
        auto type = toCx(qualType).withMutability(Mutability::Const);
        initializer->type = type;
        auto* varDecl = makeAST<VarDecl>(type, name, initializer, nullptr, AccessLevel::Default, module, Location());
        module.addToSymbolTable(varDecl);
        module.sourceFiles.front().topLevelDecls.push_back(varDecl);
    }

    void addFloatConstantToSymbolTable(llvm::StringRef name, llvm::APFloat value) {
        auto initializer = makeAST<FloatLiteralExpr>(std::move(value), Location());
        auto type = Type::getFloat64(Mutability::Const);
        initializer->type = type;
        auto* varDecl = makeAST<VarDecl>(type, name, initializer, nullptr, AccessLevel::Default, module, Location());
        module.addToSymbolTable(varDecl);
        module.sourceFiles.front().topLevelDecls.push_back(varDecl);
    }

    // True when converting this type would reach a 16-bit float. Mirrors toCx
    // case for case, including pointed-to records: conversion fills in record
    // fields eagerly, so a pointer doesn't hide float16.
    bool typeUsesFloat16(clang::QualType qualType, std::unordered_set<const clang::RecordDecl*>& visited) {
        auto& type = *qualType.getTypePtr();
        switch (type.getTypeClass()) {
        case clang::Type::Pointer:
            return typeUsesFloat16(llvm::cast<clang::PointerType>(type).getPointeeType(), visited);
        case clang::Type::Builtin: {
            auto kind = llvm::cast<clang::BuiltinType>(type).getKind();
            return kind == clang::BuiltinType::Float16 || kind == clang::BuiltinType::BFloat16;
        }
        case clang::Type::Typedef:
            return typeUsesFloat16(llvm::cast<clang::TypedefType>(type).desugar(), visited);
        case clang::Type::PredefinedSugar:
            return typeUsesFloat16(llvm::cast<clang::PredefinedSugarType>(type).desugar(), visited);
        case clang::Type::Record: {
            auto* def = llvm::cast<clang::RecordType>(type).getDecl()->getDefinition();
            if (!def) return false;
            return recordUsesFloat16(*def, visited);
        }
        case clang::Type::Paren:
            return typeUsesFloat16(llvm::cast<clang::ParenType>(type).getInnerType(), visited);
        case clang::Type::FunctionProto: {
            auto& functionProtoType = llvm::cast<clang::FunctionProtoType>(type);
            if (typeUsesFloat16(functionProtoType.getReturnType(), visited)) return true;
            for (clang::QualType paramType : functionProtoType.getParamTypes()) {
                if (typeUsesFloat16(paramType, visited)) return true;
            }
            return false;
        }
        case clang::Type::FunctionNoProto:
            return typeUsesFloat16(llvm::cast<clang::FunctionNoProtoType>(type).getReturnType(), visited);
        case clang::Type::ConstantArray:
            return typeUsesFloat16(llvm::cast<clang::ConstantArrayType>(type).getElementType(), visited);
        case clang::Type::IncompleteArray:
            return typeUsesFloat16(llvm::cast<clang::IncompleteArrayType>(type).getElementType(), visited);
        case clang::Type::Attributed:
            return typeUsesFloat16(llvm::cast<clang::AttributedType>(type).getEquivalentType(), visited);
        case clang::Type::Decayed:
            return typeUsesFloat16(llvm::cast<clang::DecayedType>(type).getDecayedType(), visited);
        case clang::Type::Vector:
            return typeUsesFloat16(llvm::cast<clang::VectorType>(type).getElementType(), visited);
        case clang::Type::Enum:
        default:
            return false;
        }
    }

    bool recordUsesFloat16(const clang::RecordDecl& recordDecl, std::unordered_set<const clang::RecordDecl*>& visited) {
        const clang::RecordDecl* def = recordDecl.getDefinition();
        if (!def) return false;
        if (!visited.insert(llvm::cast<clang::RecordDecl>(def->getCanonicalDecl())).second) return false;
        for (auto* field : def->fields()) {
            if (typeUsesFloat16(field->getType(), visited)) return true;
        }
        return false;
    }

    // 16-bit floats have no cx counterpart and no size- and ABI-preserving
    // mapping, so declarations using them are skipped. The header still
    // imports; using a skipped name fails at the use site.
    bool skipIfUsesFloat16(clang::QualType type, llvm::StringRef name, Location location) {
        std::unordered_set<const clang::RecordDecl*> visited;
        if (!typeUsesFloat16(type, visited)) return false;
        WARN(location, "skipping C declaration '" << name << "': 16-bit floating-point types are not supported");
        return true;
    }

    bool skipIfUsesFloat16(const clang::RecordDecl& recordDecl) {
        std::unordered_set<const clang::RecordDecl*> visited;
        if (!recordUsesFloat16(recordDecl, visited)) return false;
        WARN(toCx(recordDecl.getLocation()), "skipping C declaration '" << getName(recordDecl) << "': 16-bit floating-point types are not supported");
        return true;
    }

    bool HandleTopLevelDecl(clang::DeclGroupRef declGroup) override {
        for (clang::Decl* decl : declGroup) {
            try {
                switch (decl->getKind()) {
                case clang::Decl::Function: {
                    auto& clangDecl = llvm::cast<clang::FunctionDecl>(*decl);
                    if (skipIfUsesFloat16(clangDecl.getType(), clangDecl.getNameAsString(), toCx(clangDecl.getLocation()))) break;
                    auto functionDecl = toCx(clangDecl);
                    if (module.symbolTable.findInTopLevelScope(functionDecl->getName()).empty()) {
                        module.addToSymbolTable(functionDecl);
                        module.sourceFiles.front().topLevelDecls.push_back(functionDecl);
                    }
                    break;
                }
                case clang::Decl::Record: {
                    auto& recordDecl = llvm::cast<clang::RecordDecl>(*decl);
                    // Convert definitions even when a forward declaration came
                    // first; toCx unifies them via the canonical declaration.
                    if (!decl->isFirstDecl() && !recordDecl.isCompleteDefinition()) break;
                    if (skipIfUsesFloat16(recordDecl)) break;
                    toCx(recordDecl);
                    break;
                }
                case clang::Decl::Enum: {
                    auto& enumDecl = llvm::cast<clang::EnumDecl>(*decl);
                    bool isAnonymous = getName(enumDecl).empty();
                    std::vector<EnumCase> cases;

                    for (clang::EnumConstantDecl* enumerator : enumDecl.enumerators()) {
                        auto enumeratorName = enumerator->getName();
                        auto value = enumerator->getInitVal();
                        auto valueExpr = makeAST<IntLiteralExpr>(value, Location());
                        cases.push_back(EnumCase(enumeratorName, valueExpr, Type(), AccessLevel::Default, Location()));
                        auto type = isAnonymous ? enumDecl.getIntegerType()
                                                : astContext->getTagType(clang::ElaboratedTypeKeyword::None, clang::NestedNameSpecifier(), &enumDecl, false);
                        addIntegerConstantToSymbolTable(enumeratorName, value, type);
                    }

                    auto* cxEnumDecl =
                        makeAST<EnumDecl>(getName(enumDecl), std::move(cases), std::vector<Type>(), AccessLevel::Default, module, nullptr, Location());
                    module.addToSymbolTable(cxEnumDecl);
                    module.sourceFiles.front().topLevelDecls.push_back(cxEnumDecl);
                    break;
                }
                case clang::Decl::Var: {
                    auto& varDecl = llvm::cast<clang::VarDecl>(*decl);
                    if (varDecl.getLinkageInternal() != clang::Linkage::External) break;
                    if (skipIfUsesFloat16(varDecl.getType(), varDecl.getNameAsString(), toCx(varDecl.getLocation()))) break;
                    auto* cxVarDecl = toCx(varDecl);
                    module.addToSymbolTable(*cxVarDecl);
                    module.sourceFiles.front().topLevelDecls.push_back(cxVarDecl);
                    break;
                }
                case clang::Decl::Typedef: {
                    auto& typedefDecl = llvm::cast<clang::TypedefDecl>(*decl);
                    if (skipIfUsesFloat16(typedefDecl.getUnderlyingType(), typedefDecl.getNameAsString(), toCx(typedefDecl.getLocation()))) break;
                    auto underlyingType = toCx(typedefDecl.getUnderlyingType());
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
        auto params = map(decl.parameters(), [&](clang::ParmVarDecl* param) { return ParamDecl(toCx(param->getType()), param->getName(), false, Location()); });

        FunctionProto proto(decl.getName(), std::move(params), toCx(decl.getReturnType()), decl.isVariadic(), true);
        if (auto asmLabelAttr = decl.getAttr<clang::AsmLabelAttr>()) {
            proto.asmLabel = asmLabelAttr->getLabel().str();
        }
        return makeAST<FunctionDecl>(std::move(proto), std::vector<GenericArg>(), AccessLevel::Default, module, toCx(decl.getLocation()));
    }

    Location toCx(clang::SourceLocation location) {
        auto presumedLocation = sourceManager.getPresumedLoc(location);
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
        auto result = compilerInstance.getSema().ActOnNumericConstant(token);
        if (!result.isUsable()) return;
        clang::Expr* parsed = result.get();

        if (auto* intLiteral = llvm::dyn_cast<clang::IntegerLiteral>(parsed)) {
            llvm::APSInt value(intLiteral->getValue(), parsed->getType()->isUnsignedIntegerType());
            cToCxConverter.addIntegerConstantToSymbolTable(name, std::move(value), parsed->getType());
        } else if (auto* floatLiteral = llvm::dyn_cast<clang::FloatingLiteral>(parsed)) {
            cToCxConverter.addFloatConstantToSymbolTable(name, floatLiteral->getValue());
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
    auto it = Module::getAllImportedModulesMap().find(headerName);
    if (it != Module::getAllImportedModulesMap().end()) {
        importer.addImportedModule(it->second);
        return true;
    }

    clang::CompilerInstance ci;
    clang::DiagnosticOptions diagOpts;
    auto* diagClient = new ErrorIgnoringTextDiagPrinter(llvm::errs(), diagOpts);
    ci.createDiagnostics(diagClient);

    auto args = map(typechecker.options.cflags, [](auto& cflag) { return cflag.c_str(); });
    args.push_back("-fgnuc-version=4.2.1"); // Enable compatibility with GCC macros in imported headers.
#ifdef _WIN32
    args.push_back("-fms-extensions"); // Needed to parse MSVC system headers.
#endif
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

    for (llvm::StringRef includePath : llvm::concat<const std::string>(typechecker.options.importSearchPaths, getCCompilerSearchPaths())) {
        ci.getHeaderSearchOpts().AddPath(includePath, clang::frontend::System, false, true);
        ci.getHeaderSearchOpts().AddPath(includePath, clang::frontend::System, false, false);
    }
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
        REPORT_ERROR(importDecl.location, "couldn't find C header file '" << importDecl.target << "' in the following locations:" << searchDirs);
        return false;
    }

    auto headerPath = fileEntry->getFileEntry().tryGetRealPathName();
    if (headerPath.empty()) headerPath = headerName;
    importDecl.importedHeaderPath = headerPath;

    std::string headerModuleName = headerName.str();
    llvm::replace(headerModuleName, '.', '_');
    auto module = new Module(std::move(headerModuleName));
    module->isCHeaderImport = true;
    module->addSourceFile(SourceFile(headerPath.str(), module));

    auto cToCxConverter = new CToCxConverter(*module, typechecker, targetInfo, ci.getSourceManager());
    ci.setASTConsumer(std::unique_ptr<CToCxConverter>(cToCxConverter));
    ci.createASTContext();
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

    importer.addImportedModule(module);
    Module::getAllImportedModulesMap()[headerName] = module;
    return true;
}

#else // CX_NO_C_IMPORT

bool cx::importCHeader(SourceFile&, ImportDecl&, Typechecker&) {
    // C header imports are not supported in builds without the Clang-based
    // importer. Returning false makes the caller report an error.
    return false;
}

#endif // CX_NO_C_IMPORT
