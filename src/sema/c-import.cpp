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

static clang::PrintingPolicy printingPolicy{clang::LangOptions()};

static Type toDelta(clang::QualType qualtype) {
    auto& type = *qualtype.getTypePtr();
    switch (type.getTypeClass()) {
        case clang::Type::Pointer: {
            auto pointeeType = llvm::cast<clang::PointerType>(type).getPointeeType();
            return Type(PtrType(llvm::make_unique<Type>(toDelta(pointeeType))));
        }
        case clang::Type::Builtin:
            switch (llvm::cast<clang::BuiltinType>(type).getKind()) {
                case clang::BuiltinType::Void:  return Type(BasicType{"void"});
                case clang::BuiltinType::Bool:  return Type(BasicType{"bool"});
                case clang::BuiltinType::Char_S:
                case clang::BuiltinType::Char_U:return Type(BasicType{"char"});
                case clang::BuiltinType::Int:   return Type(BasicType{"int"});
                case clang::BuiltinType::UInt:  return Type(BasicType{"uint"});
                default:
                    auto name = llvm::cast<clang::BuiltinType>(type).getName(printingPolicy);
                    llvm::errs() << "WARNING: Builtin type '" << name << "' not handled\n";
                    return Type(BasicType{"int"});
            }
            return Type(BasicType{llvm::cast<clang::BuiltinType>(type).getName(printingPolicy)});
        case clang::Type::Typedef:
            return toDelta(llvm::cast<clang::TypedefType>(type).desugar());
        case clang::Type::Elaborated:
            return toDelta(llvm::cast<clang::ElaboratedType>(type).getNamedType());
        case clang::Type::Record:
            return Type(BasicType{llvm::cast<clang::RecordType>(type).getDecl()->getName()});
        default:
            llvm::errs() << "WARNING: " << type.getTypeClassName() << " not handled\n";
            return Type(BasicType{"int"});
    }
}

static FuncDecl toDelta(const clang::FunctionDecl& decl) {
    std::vector<ParamDecl> params;
    for (auto* param : decl.parameters()) {
        params.emplace_back(ParamDecl{"", toDelta(param->getType()), param->getNameAsString()});
    }
    return FuncDecl{decl.getNameAsString(), std::move(params), toDelta(decl.getReturnType())};
}

class CToDeltaConverter : public clang::ASTConsumer {
public:
    bool HandleTopLevelDecl(clang::DeclGroupRef declGroup) final override {
        for (clang::Decl* decl : declGroup) {
            switch (decl->getKind()) {
                case clang::Decl::Function:
                    addToSymbolTable(toDelta(llvm::cast<clang::FunctionDecl>(*decl)));
                    break;
                default:
                    break;
            }
        }
        return true; // continue parsing
    }
};

void importCHeader(llvm::StringRef headerName) {
    clang::CompilerInstance ci;
    clang::DiagnosticOptions diagnosticOptions;
    ci.createDiagnostics();

    std::shared_ptr<clang::TargetOptions> pto = std::make_shared<clang::TargetOptions>();
    pto->Triple = llvm::sys::getDefaultTargetTriple();
    clang::TargetInfo* pti = clang::TargetInfo::CreateTargetInfo(ci.getDiagnostics(), pto);
    ci.setTarget(pti);

    ci.createFileManager();
    ci.createSourceManager(ci.getFileManager());

    ci.getHeaderSearchOpts().AddPath("/usr/include",       clang::frontend::System, false, false);
    ci.getHeaderSearchOpts().AddPath("/usr/local/include", clang::frontend::System, false, false);
    ci.createPreprocessor(clang::TU_Complete);
    ci.getPreprocessorOpts().UsePredefines = false;

    ci.setASTConsumer(llvm::make_unique<CToDeltaConverter>());
    ci.createASTContext();

    const clang::DirectoryLookup* curDir = nullptr;
    const clang::FileEntry* pFile = ci.getPreprocessor().getHeaderSearchInfo().LookupFile(
        headerName, {}, false, nullptr, curDir, {}, nullptr, nullptr, nullptr, nullptr);
    if (!pFile) {
        llvm::errs() << "error: couldn't find header '" << headerName << "'\n";
        abort();
    }

    ci.getSourceManager().setMainFileID(ci.getSourceManager().createFileID(
        pFile, clang::SourceLocation(), clang::SrcMgr::C_System));
    ci.getDiagnosticClient().BeginSourceFile(ci.getLangOpts(), &ci.getPreprocessor());
    clang::ParseAST(ci.getPreprocessor(), &ci.getASTConsumer(), ci.getASTContext());
    ci.getDiagnosticClient().EndSourceFile();
}
