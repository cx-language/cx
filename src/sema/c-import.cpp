#include <cstdio>
#include <vector>
#include <memory>
#include <cstdlib>
#include <string>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/ErrorHandling.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Program.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Parse/ParseAST.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/Type.h>
#include <clang/AST/PrettyPrinter.h>
#include <clang/Sema/Sema.h>
#include "c-import.h"
#include "typecheck.h"
#include "../ast/type.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../support/utility.h"

using namespace delta;

namespace delta {
extern llvm::StringMap<std::shared_ptr<Module>> allImportedModules;
}

namespace {

clang::PrintingPolicy printingPolicy{clang::LangOptions()};
clang::TargetInfo* targetInfo;

Type getIntTypeByWidth(int widthInBits, bool asSigned) {
    switch (widthInBits) {
        case  8: return asSigned ? Type::getInt8()  : Type::getUInt8();
        case 16: return asSigned ? Type::getInt16() : Type::getUInt16();
        case 32: return asSigned ? Type::getInt32() : Type::getUInt32();
        case 64: return asSigned ? Type::getInt64() : Type::getUInt64();
    }
    llvm_unreachable("unsupported integer width");
}

Type toDelta(const clang::BuiltinType& type) {
    switch (type.getKind()) {
        case clang::BuiltinType::Void: return Type::getVoid();
        case clang::BuiltinType::Bool: return Type::getBool();
        case clang::BuiltinType::Char_S:
        case clang::BuiltinType::Char_U: return Type::getChar();
        case clang::BuiltinType::SChar: return getIntTypeByWidth(targetInfo->getCharWidth(), true);
        case clang::BuiltinType::UChar: return getIntTypeByWidth(targetInfo->getCharWidth(), false);
        case clang::BuiltinType::Short: return getIntTypeByWidth(targetInfo->getShortWidth(), true);
        case clang::BuiltinType::UShort: return getIntTypeByWidth(targetInfo->getShortWidth(), false);
        case clang::BuiltinType::Int: return Type::getInt();
        case clang::BuiltinType::UInt: return Type::getUInt();
        case clang::BuiltinType::Long: return getIntTypeByWidth(targetInfo->getLongWidth(), true);
        case clang::BuiltinType::ULong: return getIntTypeByWidth(targetInfo->getLongWidth(), false);
        case clang::BuiltinType::LongLong: return getIntTypeByWidth(targetInfo->getLongLongWidth(), true);
        case clang::BuiltinType::ULongLong: return getIntTypeByWidth(targetInfo->getLongLongWidth(), false);
        case clang::BuiltinType::Float: return Type::getFloat32();
        case clang::BuiltinType::Double: return Type::getFloat64();
        case clang::BuiltinType::LongDouble: return Type::getFloat80();
        default: break;
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
            auto paramTypes = map(functionProtoType.getParamTypes(),
                                  [](clang::QualType qualType) { return toDelta(qualType); });
            return FunctionType::get(toDelta(functionProtoType.getReturnType()),
                                     std::move(paramTypes), isMutable);
        }
        case clang::Type::ConstantArray: {
            auto& constantArrayType = llvm::cast<clang::ConstantArrayType>(type);
            if (!constantArrayType.getSize().isIntN(64)) {
                error(SourceLocation::invalid(), "array is too large");
            }
            return ArrayType::get(toDelta(constantArrayType.getElementType()),
                                  constantArrayType.getSize().getLimitedValue(), isMutable);
        }
        case clang::Type::IncompleteArray:
            return ArrayType::get(toDelta(llvm::cast<clang::IncompleteArrayType>(type).getElementType()),
                                  ArrayType::unknownSize);
        case clang::Type::Attributed:
            return toDelta(llvm::cast<clang::AttributedType>(type).getEquivalentType());
        case clang::Type::Decayed:
            return toDelta(llvm::cast<clang::DecayedType>(type).getDecayedType());
        case clang::Type::Enum:
            return toDelta(llvm::cast<clang::EnumType>(type).getDecl()->getIntegerType());
        case clang::Type::Vector:
            return Type::getInt(); // FIXME: Temporary.
        default:
            error(SourceLocation::invalid(), "unhandled type class '", type.getTypeClassName(),
                  "' (importing type '", qualtype.getAsString(), "')");
    }
}

FunctionDecl toDelta(const clang::FunctionDecl& decl, Module* currentModule) {
    auto params = map(decl.parameters(), [](clang::ParmVarDecl* param) {
        return ParamDecl(toDelta(param->getType()), param->getNameAsString(), SourceLocation::invalid());
    });
    FunctionProto proto(decl.getNameAsString(), std::move(params), toDelta(decl.getReturnType()),
                        decl.isVariadic(), true);
    return FunctionDecl(std::move(proto), {}, *currentModule, SourceLocation::invalid());
}

llvm::Optional<FieldDecl> toDelta(const clang::FieldDecl& decl, TypeDecl& typeDecl) {
    if (decl.getName().empty()) return llvm::None;
    return FieldDecl(toDelta(decl.getType()), decl.getNameAsString(), typeDecl,
                     SourceLocation::invalid());
}

llvm::Optional<TypeDecl> toDelta(const clang::RecordDecl& decl, Module* currentModule) {
    TypeDecl typeDecl(decl.isUnion() ? TypeTag::Union : TypeTag::Struct,
                      getRecordName(decl), {}, {}, *currentModule, SourceLocation::invalid());
    typeDecl.getFields().reserve(16); // TODO: Reserve based on the field count of `decl`.
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
    return VarDecl(toDelta(decl.getType()), decl.getName(), nullptr, nullptr, *currentModule,
                   SourceLocation::invalid());
}

// TODO: Use llvm::APSInt instead of int64_t.
void addIntegerConstantToSymbolTable(llvm::StringRef name, int64_t value, clang::QualType type,
                                     const Typechecker& typechecker) {
    auto initializer = std::make_shared<IntLiteralExpr>(value, SourceLocation::invalid());
    initializer->setType(toDelta(type).asImmutable());
    typechecker.addToSymbolTable(VarDecl(initializer->getType(), name, initializer, nullptr,
                                         *typechecker.getCurrentModule(), SourceLocation::invalid()));
}

// TODO: Use llvm::APFloat instead of long double.
void addFloatConstantToSymbolTable(llvm::StringRef name, long double value, const Typechecker& typechecker) {
    auto initializer = std::make_shared<FloatLiteralExpr>(value, SourceLocation::invalid());
    initializer->setType(Type::getFloat64());
    typechecker.addToSymbolTable(VarDecl(initializer->getType(), name, initializer, nullptr,
                                         *typechecker.getCurrentModule(), SourceLocation::invalid()));
}

class CToDeltaConverter : public clang::ASTConsumer {
public:
    CToDeltaConverter(const Typechecker& typechecker) : typechecker(typechecker) {}

    bool HandleTopLevelDecl(clang::DeclGroupRef declGroup) final override {
        for (clang::Decl* decl : declGroup) {
            switch (decl->getKind()) {
                case clang::Decl::Function:
                    typechecker.addToSymbolTable(toDelta(llvm::cast<clang::FunctionDecl>(*decl),
                                                         typechecker.getCurrentModule()));
                    break;
                case clang::Decl::Record: {
                    if (!decl->isFirstDecl()) break;
                    auto typeDecl = toDelta(llvm::cast<clang::RecordDecl>(*decl),
                                            typechecker.getCurrentModule());
                    if (typeDecl) {
                        // Skip redefinitions caused by different modules including the same headers.
                        if (typechecker.findDecls(typeDecl->getName(), true).empty()) {
                            typechecker.addToSymbolTable(std::move(*typeDecl));
                        }
                    }
                    break;
                }
                case clang::Decl::Enum: {
                    auto& enumDecl = llvm::cast<clang::EnumDecl>(*decl);
                    for (auto* enumerator : enumDecl.enumerators()) {
                        auto value = enumerator->getInitVal().getExtValue();
                        addIntegerConstantToSymbolTable(enumerator->getName(), value,
                                                        enumDecl.getIntegerType(), typechecker);
                    }
                    break;
                }
                case clang::Decl::Var:
                    typechecker.addToSymbolTable(toDelta(llvm::cast<clang::VarDecl>(*decl),
                                                         typechecker.getCurrentModule()));
                    break;
                case clang::Decl::Typedef: {
                    auto& typedefDecl = llvm::cast<clang::TypedefDecl>(*decl);
                    if (auto* baseTypeId = typedefDecl.getUnderlyingType().getBaseTypeIdentifier()) {
                        typechecker.addIdentifierReplacement(typedefDecl.getName(), baseTypeId->getName());
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
    const Typechecker& typechecker;
};

class MacroImporter : public clang::PPCallbacks {
public:
    MacroImporter(const Typechecker& typechecker, clang::Sema& clangSema)
    : typechecker(typechecker), clangSema(clangSema) {}

    void MacroDefined(const clang::Token& name, const clang::MacroDirective* macro) final override {
        if (macro->getMacroInfo()->getNumTokens() != 1) return;
        auto& token = macro->getMacroInfo()->getReplacementToken(0);

        switch (token.getKind()) {
            case clang::tok::identifier:
                typechecker.addIdentifierReplacement(name.getIdentifierInfo()->getName(),
                                                     token.getIdentifierInfo()->getName());
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
            addIntegerConstantToSymbolTable(name, value.getExtValue(), parsed->getType(), typechecker);
        } else if (auto* floatLiteral = llvm::dyn_cast<clang::FloatingLiteral>(parsed)) {
            // TODO: Use llvm::APFloat instead of lossy conversion to double.
            auto value = floatLiteral->getValueAsApproximateDouble();
            addFloatConstantToSymbolTable(name, value, typechecker);
        }
    }

private:
    const Typechecker& typechecker;
    clang::Sema& clangSema;
};

} // anonymous namespace

static void addHeaderSearchPathsFromEnvVar(clang::CompilerInstance& ci, const char* name) {
    if (const char* pathList = std::getenv(name)) {
        llvm::SmallVector<llvm::StringRef, 1> paths;
        llvm::StringRef(pathList).split(paths, llvm::sys::EnvPathSeparator, -1, false);

        for (llvm::StringRef path : paths) {
            ci.getHeaderSearchOpts().AddPath(path, clang::frontend::System, false, false);
        }
    }
}

static void addHeaderSearchPathsFromCCompilerOutput(clang::CompilerInstance& ci) {
    std::string command = "echo | " + getCCompilerPath() + " -E -v - 2>&1 | grep '^ /'";
    std::shared_ptr<FILE> process(popen(command.c_str(), "r"), pclose);

    while (!std::feof(process.get())) {
        std::string path;

        while (true) {
            int ch = std::fgetc(process.get());

            if (ch == EOF || ch == '\n') {
                break;
            } else if (!path.empty() || ch != ' ') {
                path += (char) ch;
            }
        }

        if (llvm::sys::fs::is_directory(path)) {
            ci.getHeaderSearchOpts().AddPath(path, clang::frontend::System, false, false);
        }
    }
}

bool delta::importCHeader(SourceFile& importer, llvm::StringRef headerName,
                          llvm::ArrayRef<std::string> importSearchPaths,
                          llvm::ArrayRef<std::string> frameworkSearchPaths) {
    auto it = allImportedModules.find(headerName);
    if (it != allImportedModules.end()) {
        importer.addImportedModule(it->second);
        return true;
    }

    auto module = std::make_shared<Module>(headerName);
    Typechecker typechecker(module.get(), nullptr);

    clang::CompilerInstance ci;
    clang::DiagnosticOptions diagnosticOptions;
    ci.createDiagnostics();

    std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    targetInfo = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    ci.setTarget(targetInfo);

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());

    addHeaderSearchPathsFromCCompilerOutput(ci);
    ci.getHeaderSearchOpts().AddPath("/usr/include",       clang::frontend::System, false, false);
    ci.getHeaderSearchOpts().AddPath("/usr/local/include", clang::frontend::System, false, false);
    ci.getHeaderSearchOpts().AddPath(CLANG_BUILTIN_INCLUDE_PATH, clang::frontend::System, false, false);
    addHeaderSearchPathsFromEnvVar(ci, "CPATH");
    addHeaderSearchPathsFromEnvVar(ci, "C_INCLUDE_PATH");
    for (llvm::StringRef includePath : importSearchPaths) {
        ci.getHeaderSearchOpts().AddPath(includePath,      clang::frontend::System, false, false);
    }
    for (llvm::StringRef frameworkPath : frameworkSearchPaths) {
        ci.getHeaderSearchOpts().AddPath(frameworkPath,    clang::frontend::System, true, false);
    }

    ci.createPreprocessor(clang::TU_Complete);
    auto& pp = ci.getPreprocessor();
    pp.getBuiltinInfo().initializeBuiltins(pp.getIdentifierTable(), pp.getLangOpts());

    ci.setASTConsumer(llvm::make_unique<CToDeltaConverter>(typechecker));
    ci.createASTContext();
    ci.createSema(clang::TU_Complete, nullptr);
    pp.addPPCallbacks(llvm::make_unique<MacroImporter>(typechecker, ci.getSema()));

    const clang::DirectoryLookup* curDir = nullptr;
    const clang::FileEntry* fileEntry = ci.getPreprocessor().getHeaderSearchInfo().LookupFile(
        headerName, {}, false, nullptr, curDir, {}, nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!fileEntry) return false;

    auto fileID = ci.getSourceManager().createFileID(fileEntry, clang::SourceLocation(),
                                                     clang::SrcMgr::C_System);
    ci.getSourceManager().setMainFileID(fileID);
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext());
    ci.getDiagnosticClient().EndSourceFile();

    importer.addImportedModule(module);
    allImportedModules[module->getName()] = module;
    return true;
}
