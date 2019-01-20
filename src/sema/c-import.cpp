#include "c-import.h"
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/AST/Type.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Parse/ParseAST.h>
#include <clang/Sema/Sema.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/Program.h>
#pragma warning(pop)
#include "typecheck.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../ast/type.h"
#include "../support/utility.h"

using namespace delta;

namespace {

clang::TargetInfo* targetInfo;

Type getIntTypeByWidth(int widthInBits, bool asSigned) {
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

Type toDelta(const clang::BuiltinType& type) {
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
            return Type::getInt();
        case clang::BuiltinType::UInt:
            return Type::getUInt();
        case clang::BuiltinType::Long:
            return getIntTypeByWidth(targetInfo->getLongWidth(), true);
        case clang::BuiltinType::ULong:
            return getIntTypeByWidth(targetInfo->getLongWidth(), false);
        case clang::BuiltinType::LongLong:
            return getIntTypeByWidth(targetInfo->getLongLongWidth(), true);
        case clang::BuiltinType::ULongLong:
            return getIntTypeByWidth(targetInfo->getLongLongWidth(), false);
        case clang::BuiltinType::Float:
            return Type::getFloat32();
        case clang::BuiltinType::Double:
            return Type::getFloat64();
        case clang::BuiltinType::LongDouble:
            return Type::getFloat80();
        default:
            break;
    }
    llvm_unreachable("unsupported builtin type");
}

llvm::StringRef getRecordName(const clang::RecordDecl& recordDecl) {
    if (!recordDecl.getName().empty()) {
        return recordDecl.getName();
    } else if (auto* typedefNameDecl = recordDecl.getTypedefNameForAnonDecl()) {
        return typedefNameDecl->getName();
    }
    return "";
}

Type toDelta(clang::QualType qualtype) {
    const bool isMutable = !qualtype.isConstQualified();
    auto& type = *qualtype.getTypePtr();
    switch (type.getTypeClass()) {
        case clang::Type::Pointer: {
            auto pointeeType = llvm::cast<clang::PointerType>(type).getPointeeType();
            if (pointeeType->isFunctionType()) {
                return OptionalType::get(toDelta(pointeeType), isMutable);
            }
            return OptionalType::get(PointerType::get(toDelta(pointeeType), false), isMutable);
        }
        case clang::Type::Builtin: {
            Type deltaType = toDelta(llvm::cast<clang::BuiltinType>(type));
            deltaType.setMutable(isMutable);
            return deltaType;
        }
        case clang::Type::Typedef: {
            auto desugared = llvm::cast<clang::TypedefType>(type).desugar();
            if (!isMutable) desugared.addConst();
            return toDelta(desugared);
        }
        case clang::Type::Elaborated:
            return toDelta(llvm::cast<clang::ElaboratedType>(type).getNamedType());
        case clang::Type::Record: {
            auto* recordDecl = llvm::cast<clang::RecordType>(type).getDecl();
            return BasicType::get(getRecordName(*recordDecl), {}, isMutable);
        }
        case clang::Type::Paren:
            return toDelta(llvm::cast<clang::ParenType>(type).getInnerType());
        case clang::Type::FunctionProto: {
            auto& functionProtoType = llvm::cast<clang::FunctionProtoType>(type);
            auto paramTypes = map(functionProtoType.getParamTypes(), [](clang::QualType qualType) { return toDelta(qualType).asImmutable(); });
            return FunctionType::get(toDelta(functionProtoType.getReturnType()).asImmutable(), std::move(paramTypes), isMutable);
        }
        case clang::Type::FunctionNoProto: {
            auto& functionNoProtoType = llvm::cast<clang::FunctionNoProtoType>(type);
            // This treats it as a zero-argument function, but really it should accept any number of arguments of any types.
            return FunctionType::get(toDelta(functionNoProtoType.getReturnType()).asImmutable(), {}, isMutable);
        }
        case clang::Type::ConstantArray: {
            auto& constantArrayType = llvm::cast<clang::ConstantArrayType>(type);
            if (!constantArrayType.getSize().isIntN(64)) {
                error(SourceLocation(), "array is too large");
            }
            return ArrayType::get(toDelta(constantArrayType.getElementType()), constantArrayType.getSize().getLimitedValue(), isMutable);
        }
        case clang::Type::IncompleteArray:
            return ArrayType::get(toDelta(llvm::cast<clang::IncompleteArrayType>(type).getElementType()), ArrayType::unknownSize);
        case clang::Type::Attributed:
            return toDelta(llvm::cast<clang::AttributedType>(type).getEquivalentType());
        case clang::Type::Decayed:
            return toDelta(llvm::cast<clang::DecayedType>(type).getDecayedType());
        case clang::Type::Enum:
            return toDelta(llvm::cast<clang::EnumType>(type).getDecl()->getIntegerType());
        case clang::Type::Vector:
            return Type::getInt(); // TODO: Handle vector types properly.
        default:
            error(SourceLocation(), "unhandled type class '", type.getTypeClassName(), "' (importing type '", qualtype.getAsString(), "')");
    }
}

FunctionDecl toDelta(const clang::FunctionDecl& decl, Module* currentModule) {
    auto params = map(decl.parameters(), [](clang::ParmVarDecl* param) {
        return ParamDecl(toDelta(param->getType()), param->getNameAsString(), SourceLocation());
    });
    FunctionProto proto(decl.getNameAsString(), std::move(params), toDelta(decl.getReturnType()), decl.isVariadic(), true);
    return FunctionDecl(std::move(proto), {}, AccessLevel::Default, *currentModule, SourceLocation());
}

llvm::Optional<FieldDecl> toDelta(const clang::FieldDecl& decl, TypeDecl& typeDecl) {
    if (decl.getName().empty()) return llvm::None;
    return FieldDecl(toDelta(decl.getType()), decl.getNameAsString(), typeDecl, AccessLevel::Default, SourceLocation());
}

llvm::Optional<TypeDecl> toDelta(const clang::RecordDecl& decl, Module* currentModule) {
    auto tag = decl.isUnion() ? TypeTag::Union : TypeTag::Struct;
    TypeDecl typeDecl(tag, getRecordName(decl), {}, {}, AccessLevel::Default, *currentModule, SourceLocation());

    for (auto* field : decl.fields()) {
        if (auto fieldDecl = toDelta(*field, typeDecl)) {
            typeDecl.getFields().emplace_back(std::move(*fieldDecl));
        } else {
            return llvm::None;
        }
    }

    return std::move(typeDecl);
}

VarDecl toDelta(const clang::VarDecl& decl, Module* currentModule) {
    return VarDecl(toDelta(decl.getType()), decl.getName(), nullptr, nullptr, AccessLevel::Default, *currentModule, SourceLocation());
}

void addIntegerConstantToSymbolTable(llvm::StringRef name, llvm::APSInt value, clang::QualType qualType, Module& module) {
    auto initializer = llvm::make_unique<IntLiteralExpr>(std::move(value), SourceLocation());
    auto type = toDelta(qualType).asImmutable();
    initializer->setType(type);
    module.addToSymbolTable(VarDecl(type, name, std::move(initializer), nullptr, AccessLevel::Default, module, SourceLocation()));
}

// TODO: Use llvm::APFloat instead of long double.
void addFloatConstantToSymbolTable(llvm::StringRef name, long double value, Module& module) {
    auto initializer = llvm::make_unique<FloatLiteralExpr>(value, SourceLocation());
    auto type = Type::getFloat64();
    initializer->setType(type);
    module.addToSymbolTable(VarDecl(type, name, std::move(initializer), nullptr, AccessLevel::Default, module, SourceLocation()));
}

class CToDeltaConverter : public clang::ASTConsumer {
public:
    CToDeltaConverter(Module& module) : module(module) {}

    bool HandleTopLevelDecl(clang::DeclGroupRef declGroup) final override {
        for (clang::Decl* decl : declGroup) {
            switch (decl->getKind()) {
                case clang::Decl::Function:
                    module.addToSymbolTable(toDelta(llvm::cast<clang::FunctionDecl>(*decl), &module));
                    break;
                case clang::Decl::Record: {
                    if (!decl->isFirstDecl()) break;
                    auto typeDecl = toDelta(llvm::cast<clang::RecordDecl>(*decl), &module);
                    if (typeDecl) {
                        // Skip redefinitions caused by different modules including the same headers.
                        if (module.findDecls(typeDecl->getName(), nullptr, nullptr).empty()) {
                            module.addToSymbolTable(std::move(*typeDecl));
                        }
                    }
                    break;
                }
                case clang::Decl::Enum: {
                    auto& enumDecl = llvm::cast<clang::EnumDecl>(*decl);
                    for (auto* enumerator : enumDecl.enumerators()) {
                        auto& value = enumerator->getInitVal();
                        addIntegerConstantToSymbolTable(enumerator->getName(), value, enumDecl.getIntegerType(), module);
                    }
                    break;
                }
                case clang::Decl::Var:
                    module.addToSymbolTable(toDelta(llvm::cast<clang::VarDecl>(*decl), &module));
                    break;
                case clang::Decl::Typedef: {
                    auto& typedefDecl = llvm::cast<clang::TypedefDecl>(*decl);
                    if (auto* baseTypeId = typedefDecl.getUnderlyingType().getBaseTypeIdentifier()) {
                        module.addIdentifierReplacement(typedefDecl.getName(), baseTypeId->getName());
                    }
                    break;
                }
                default:
                    break;
            }
        }
        return true; // continue parsing
    }

private:
    Module& module;
};

class MacroImporter : public clang::PPCallbacks {
public:
    MacroImporter(Module& module, clang::Sema& clangSema) : module(module), clangSema(clangSema) {}

    void MacroDefined(const clang::Token& name, const clang::MacroDirective* macro) final override {
        if (macro->getMacroInfo()->getNumTokens() != 1) return;
        auto& token = macro->getMacroInfo()->getReplacementToken(0);

        switch (token.getKind()) {
            case clang::tok::identifier:
                module.addIdentifierReplacement(name.getIdentifierInfo()->getName(), token.getIdentifierInfo()->getName());
                return;

            case clang::tok::numeric_constant:
                importNumericConstant(name.getIdentifierInfo()->getName(), token);
                return;

            default:
                return;
        }
    }

private:
    void importNumericConstant(llvm::StringRef name, const clang::Token& token) {
        auto result = clangSema.ActOnNumericConstant(token);
        if (!result.isUsable()) return;
        clang::Expr* parsed = result.get();

        if (auto* intLiteral = llvm::dyn_cast<clang::IntegerLiteral>(parsed)) {
            llvm::APSInt value(intLiteral->getValue(), parsed->getType()->isUnsignedIntegerType());
            addIntegerConstantToSymbolTable(name, std::move(value), parsed->getType(), module);
        } else if (auto* floatLiteral = llvm::dyn_cast<clang::FloatingLiteral>(parsed)) {
            // TODO: Use llvm::APFloat instead of lossy conversion to double.
            auto value = floatLiteral->getValueAsApproximateDouble();
            addFloatConstantToSymbolTable(name, value, module);
        }
    }

private:
    Module& module;
    clang::Sema& clangSema;
};

} // namespace

bool delta::importCHeader(SourceFile& importer, llvm::StringRef headerName, llvm::ArrayRef<std::string> importSearchPaths,
                          llvm::ArrayRef<std::string> frameworkSearchPaths) {
    auto it = Module::getAllImportedModulesMap().find(headerName);
    if (it != Module::getAllImportedModulesMap().end()) {
        importer.addImportedModule(it->second);
        return true;
    }

    auto module = std::make_shared<Module>(headerName);

    clang::CompilerInstance ci;
    clang::DiagnosticOptions diagnosticOptions;
    ci.createDiagnostics();

    std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    targetInfo = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    ci.setTarget(targetInfo);

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());

    for (llvm::StringRef includePath : importSearchPaths) {
        ci.getHeaderSearchOpts().AddPath(includePath, clang::frontend::System, false, false);
    }
    for (llvm::StringRef frameworkPath : frameworkSearchPaths) {
        ci.getHeaderSearchOpts().AddPath(frameworkPath, clang::frontend::System, true, false);
    }

    ci.createPreprocessor(clang::TU_Complete);
    auto& pp = ci.getPreprocessor();
    pp.getBuiltinInfo().initializeBuiltins(pp.getIdentifierTable(), pp.getLangOpts());

    ci.setASTConsumer(llvm::make_unique<CToDeltaConverter>(*module));
    ci.createASTContext();
    ci.createSema(clang::TU_Complete, nullptr);
    pp.addPPCallbacks(llvm::make_unique<MacroImporter>(*module, ci.getSema()));

    const clang::DirectoryLookup* curDir = nullptr;
    const clang::FileEntry* fileEntry = ci.getPreprocessor().getHeaderSearchInfo().LookupFile(headerName, {}, false, nullptr, curDir, {},
                                                                                              nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!fileEntry) return false;

    auto fileID = ci.getSourceManager().createFileID(fileEntry, clang::SourceLocation(), clang::SrcMgr::C_System);
    ci.getSourceManager().setMainFileID(fileID);
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext());
    ci.getDiagnosticClient().EndSourceFile();

    importer.addImportedModule(module);
    Module::getAllImportedModulesMap()[module->getName()] = module;
    return true;
}
