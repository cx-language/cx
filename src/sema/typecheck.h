#pragma once

#include <memory>
#include <string>
#include <vector>
#include <llvm/ADT/StringMap.h>
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"

namespace llvm {
class StringRef;
template<typename T> class ArrayRef;
template<typename T, unsigned N> class SmallVector;
}

namespace delta {

class Module;
class PackageManifest;
class SourceFile;
struct SourceLocation;
struct Type;

using ParserFunction = void(llvm::StringRef filePath, Module& module);

std::vector<Module*> getAllImportedModules();
void typecheckModule(Module& module, const PackageManifest* manifest,
                     llvm::ArrayRef<std::string> importSearchPaths, ParserFunction& parse);

class TypeChecker {
public:
    explicit TypeChecker(Module* currentModule, SourceFile* currentSourceFile)
    : currentModule(currentModule), currentSourceFile(currentSourceFile), currentFunction(nullptr),
      isPostProcessing(false) {}

    Module* getCurrentModule() const { return currentModule; }
    const SourceFile* getCurrentSourceFile() const { return currentSourceFile; }

    Decl& findDecl(llvm::StringRef name, SourceLocation location, bool everywhere = false) const;
    std::vector<Decl*> findDecls(llvm::StringRef name, bool everywhere = false) const;

    void addToSymbolTable(FunctionTemplate& decl) const;
    void addToSymbolTable(FunctionDecl& decl) const;
    void addToSymbolTable(FunctionDecl&& decl) const;
    void addToSymbolTable(TypeTemplate& decl) const;
    void addToSymbolTable(TypeDecl& decl) const;
    void addToSymbolTable(TypeDecl&& decl) const;
    void addToSymbolTable(VarDecl& decl, bool global) const;
    void addToSymbolTable(VarDecl&& decl) const;
    void addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target) const;

    Type typecheckExpr(Expr& expr, bool useIsWriteOnly = false) const;
    void typecheckVarDecl(VarDecl& decl, bool isGlobal) const;
    void typecheckFieldDecl(FieldDecl& decl) const;
    void typecheckTopLevelDecl(Decl& decl, const PackageManifest* manifest,
                               llvm::ArrayRef<std::string> importSearchPaths,
                               ParserFunction& parse) const;
    void postProcess();

private:
    void typecheckFunctionDecl(FunctionDecl& decl) const;
    void typecheckFunctionTemplate(FunctionTemplate& decl) const;
    void typecheckMemberDecl(Decl& decl) const;

    void typecheckStmt(Stmt& stmt) const;
    void typecheckAssignStmt(AssignStmt& stmt) const;
    void typecheckCompoundStmt(CompoundStmt& stmt) const;
    void typecheckReturnStmt(ReturnStmt& stmt) const;
    void typecheckVarStmt(VarStmt& stmt) const;
    void typecheckIncrementStmt(IncrementStmt& stmt) const;
    void typecheckDecrementStmt(DecrementStmt& stmt) const;
    void typecheckIfStmt(IfStmt& ifStmt) const;
    void typecheckSwitchStmt(SwitchStmt& stmt) const;
    void typecheckWhileStmt(WhileStmt& whileStmt) const;
    void typecheckBreakStmt(BreakStmt& breakStmt) const;
    void typecheckType(Type type, SourceLocation location) const;
    void typecheckParamDecl(ParamDecl& decl) const;
    void typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams) const;
    void typecheckTypeDecl(TypeDecl& decl) const;
    void typecheckTypeTemplate(TypeTemplate& decl) const;
    void typecheckImportDecl(ImportDecl& decl, const PackageManifest* manifest,
                             llvm::ArrayRef<std::string> importSearchPaths, ParserFunction& parse) const;

    Type typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly) const;
    Type typecheckArrayLiteralExpr(ArrayLiteralExpr& expr) const;
    Type typecheckPrefixExpr(PrefixExpr& expr) const;
    Type typecheckBinaryExpr(BinaryExpr& expr) const;
    Type typecheckCallExpr(CallExpr& expr) const;
    Type typecheckBuiltinConversion(CallExpr& expr) const;
    Type typecheckCastExpr(CastExpr& expr) const;
    Type typecheckSizeofExpr(SizeofExpr& expr) const;
    Type typecheckMemberExpr(MemberExpr& expr) const;
    Type typecheckSubscriptExpr(SubscriptExpr& expr) const;
    Type typecheckUnwrapExpr(UnwrapExpr& expr) const;

    bool isInterface(Type type) const;
    bool hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const;
    bool implementsInterface(TypeDecl& type, TypeDecl& interface) const;
    bool isValidConversion(Expr& expr, Type source, Type target) const;
    bool isValidConversion(llvm::ArrayRef<std::unique_ptr<Expr>> exprs, Type source, Type target) const;
    llvm::StringMap<Type> getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams,
                                                CallExpr& call, llvm::ArrayRef<ParamDecl> params) const;
    FunctionDecl& resolveOverload(CallExpr& expr, llvm::StringRef callee) const;
    std::vector<Type> inferGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                       const CallExpr& call, llvm::ArrayRef<ParamDecl> params) const;
    bool isImplicitlyCopyable(Type type) const;
    bool validateArgs(const CallExpr& expr, const FunctionDecl& functionDecl,
                      llvm::StringRef functionName = "",
                      SourceLocation location = SourceLocation::invalid()) const;
    bool validateArgs(const CallExpr& expr, bool isMutating, llvm::ArrayRef<ParamDecl> params,
                      bool isVariadic, llvm::StringRef functionName = "",
                      SourceLocation location = SourceLocation::invalid()) const;
    TypeDecl* getTypeDecl(const BasicType& type) const;
    void markFieldAsInitialized(Expr& expr) const;
    void addToSymbolTableWithName(Decl& decl, llvm::StringRef name, bool global) const;
    template<typename DeclT>
    void addToSymbolTableNonAST(DeclT& decl) const;

private:
    Module* currentModule;
    SourceFile* currentSourceFile;
    mutable FunctionDecl* currentFunction;
    mutable bool isPostProcessing;
    mutable std::vector<Decl*> declsToTypecheck;
};

}
