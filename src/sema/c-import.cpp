#include <cstdlib>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/ErrorHandling.h>
#include <clang/Basic/TargetInfo.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Lex/Preprocessor.h>
#include <clang/Lex/HeaderSearch.h>
#include <clang/Parse/ParseAST.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>
#include <clang/AST/Type.h>
#include <clang/AST/PrettyPrinter.h>
#include "c-import.h"
#include "typecheck.h"
#include "../ast/type.h"
#include "../ast/decl.h"

using namespace delta;

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

Type toDelta(clang::QualType qualtype) {
    const bool isMutable = !qualtype.isConstQualified();
    auto& type = *qualtype.getTypePtr();
    switch (type.getTypeClass()) {
        case clang::Type::Pointer: {
            auto pointeeType = llvm::cast<clang::PointerType>(type).getPointeeType();
            return PtrType::get(toDelta(pointeeType), false, isMutable);
        }
        case clang::Type::Builtin: {
            Type deltaType = toDelta(llvm::cast<clang::BuiltinType>(type));
            deltaType.setMutable(isMutable);
            return deltaType;
        }
        case clang::Type::Typedef:
            return toDelta(llvm::cast<clang::TypedefType>(type).desugar());
        case clang::Type::Elaborated:
            return toDelta(llvm::cast<clang::ElaboratedType>(type).getNamedType());
        case clang::Type::Record:
            return BasicType::get(llvm::cast<clang::RecordType>(type).getDecl()->getName(), isMutable);
        default:
            return Type::getInt(isMutable); // FIXME: Dummy.
    }
}

FuncDecl toDelta(const clang::FunctionDecl& decl) {
    std::vector<ParamDecl> params;
    for (auto* param : decl.parameters()) {
        params.emplace_back(toDelta(param->getType()), param->getNameAsString(),
                            SrcLoc::invalid());
    }
    return FuncDecl(decl.getNameAsString(), std::move(params),
                    toDelta(decl.getReturnType()), "", SrcLoc::invalid());
}

llvm::Optional<FieldDecl> toDelta(const clang::FieldDecl& decl) {
    if (decl.getName().empty()) return llvm::None;
    return FieldDecl(toDelta(decl.getType()), decl.getNameAsString(), SrcLoc::invalid());
}

llvm::Optional<TypeDecl> toDelta(const clang::RecordDecl& decl) {
    if (decl.getName().empty()) return llvm::None;

    TypeDecl typeDecl(decl.isUnion() ? TypeTag::Union : TypeTag::Struct,
                      decl.getNameAsString(), {}, {}, SrcLoc::invalid());
    typeDecl.fields.reserve(16); // TODO: Reserve based on the field count of `decl`.
    for (auto* field : decl.fields()) {
        if (auto fieldDecl = toDelta(*field)) {
            typeDecl.fields.emplace_back(std::move(*fieldDecl));
        } else {
            return llvm::None;
        }
    }
    return std::move(typeDecl);
}

void addIntegerConstantToSymbolTable(llvm::StringRef name, int64_t value) {
    auto initializer = std::make_shared<IntLiteralExpr>(value, SrcLoc::invalid());
    initializer->setType(Type::getInt());
    addToSymbolTable(VarDecl(initializer->getType(), name, initializer, SrcLoc::invalid()));
}

void addFloatConstantToSymbolTable(llvm::StringRef name, long double value) {
    auto initializer = std::make_shared<FloatLiteralExpr>(value, SrcLoc::invalid());
    initializer->setType(Type::getFloat64());
    addToSymbolTable(VarDecl(initializer->getType(), name, initializer, SrcLoc::invalid()));
}

class CToDeltaConverter : public clang::ASTConsumer {
public:
    bool HandleTopLevelDecl(clang::DeclGroupRef declGroup) final override {
        for (clang::Decl* decl : declGroup) {
            switch (decl->getKind()) {
                case clang::Decl::Function:
                    addToSymbolTable(toDelta(llvm::cast<clang::FunctionDecl>(*decl)));
                    break;
                case clang::Decl::Record: {
                    if (!decl->isFirstDecl()) break;
                    auto typeDecl = toDelta(llvm::cast<clang::RecordDecl>(*decl));
                    if (typeDecl) addToSymbolTable(std::move(*typeDecl));
                    break;
                }
                case clang::Decl::Enum: {
                    for (auto* enumerator : llvm::cast<clang::EnumDecl>(*decl).enumerators()) {
                        auto value = enumerator->getInitVal().getExtValue();
                        addIntegerConstantToSymbolTable(enumerator->getName(), value);
                    }
                }
                default:
                    break;
            }
        }
        return true; // continue parsing
    }
};

class MacroImporter : public clang::PPCallbacks {
    void MacroDefined(const clang::Token& name, const clang::MacroDirective* macro) final override {
        if (macro->getMacroInfo()->getNumTokens() != 1) return;
        auto& token = macro->getMacroInfo()->getReplacementToken(0);
        if (token.isNot(clang::tok::numeric_constant)) return;

        llvm::StringRef text(token.getLiteralData(), token.getLength());
        if (text.find_first_of(".eE") == llvm::StringRef::npos) {
            long long value = strtoll(text.data(), nullptr, 0);
            addIntegerConstantToSymbolTable(name.getIdentifierInfo()->getName(), value);
        } else {
            long double value = strtold(text.data(), nullptr);
            addFloatConstantToSymbolTable(name.getIdentifierInfo()->getName(), value);
        }
    }
};

} // anonymous namespace

bool delta::importCHeader(llvm::StringRef headerName, llvm::ArrayRef<llvm::StringRef> includePaths) {
    static llvm::StringSet<> importedHeaders;
    if (!importedHeaders.insert(headerName).second) return true; // Already imported?

    clang::CompilerInstance ci;
    clang::DiagnosticOptions diagnosticOptions;
    ci.createDiagnostics();

    std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    targetInfo = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    ci.setTarget(targetInfo);

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());

    ci.getHeaderSearchOpts().AddPath("/usr/include",       clang::frontend::System, false, false);
    ci.getHeaderSearchOpts().AddPath("/usr/local/include", clang::frontend::System, false, false);
    ci.getHeaderSearchOpts().AddPath(CLANG_BUILTIN_INCLUDE_PATH, clang::frontend::System, false, false);
    for (llvm::StringRef includePath : includePaths) {
        ci.getHeaderSearchOpts().AddPath(includePath,      clang::frontend::System, false, false);
    }

    ci.createPreprocessor(clang::TU_Complete);
    auto& pp = ci.getPreprocessor();
    pp.getBuiltinInfo().initializeBuiltins(pp.getIdentifierTable(), pp.getLangOpts());
    pp.addPPCallbacks(llvm::make_unique<MacroImporter>());

    ci.setASTConsumer(llvm::make_unique<CToDeltaConverter>());
    ci.createASTContext();

    const clang::DirectoryLookup* curDir = nullptr;
    const clang::FileEntry* pFile = ci.getPreprocessor().getHeaderSearchInfo().LookupFile(
        headerName, {}, false, nullptr, curDir, {}, nullptr, nullptr, nullptr, nullptr);
    if (!pFile) return false;

    ci.getSourceManager().setMainFileID(ci.getSourceManager().createFileID(
        pFile, clang::SourceLocation(), clang::SrcMgr::C_System));
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext());
    ci.getDiagnosticClient().EndSourceFile();
    return true;
}
