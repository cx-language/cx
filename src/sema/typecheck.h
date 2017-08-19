#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "../ast/expr.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"
#include "../ast/type-resolver.h"

namespace llvm {
class StringRef;
template<typename T> class ArrayRef;
template<typename T, unsigned N> class SmallVector;
}

namespace delta {

class Module;
class SourceFile;
struct SourceLocation;
struct Type;

using ParserFunction = void(llvm::StringRef filePath, Module& module);

std::vector<Module*> getAllImportedModules();
void typecheckModule(Module& module, llvm::ArrayRef<llvm::StringRef> importSearchPaths,
                     ParserFunction& parse);

class TypeChecker : public TypeResolver {
public:
    explicit TypeChecker(Module* currentModule, SourceFile* currentSourceFile)
    : currentModule(currentModule), currentSourceFile(currentSourceFile), currentFunction(nullptr),
      typecheckingGenericFunction(false) {}

    Module* getCurrentModule() const { return currentModule; }
    const SourceFile* getCurrentSourceFile() const { return currentSourceFile; }

    Decl& findDecl(llvm::StringRef name, SourceLocation location, bool everywhere = false) const;
    llvm::SmallVector<Decl*, 1> findDecls(llvm::StringRef name, bool everywhere = false) const;

    void addToSymbolTable(FunctionDecl& decl) const;
    void addToSymbolTable(FunctionDecl&& decl) const;
    void addToSymbolTable(InitDecl& decl) const;
    void addToSymbolTable(DeinitDecl& decl) const;
    void addToSymbolTable(TypeDecl& decl) const;
    void addToSymbolTable(TypeDecl&& decl) const;
    void addToSymbolTable(VarDecl& decl) const;
    void addToSymbolTable(VarDecl&& decl) const;
    void addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target) const;

    Type typecheckExpr(Expr& expr) const;
    void typecheckVarDecl(VarDecl& decl, bool isGlobal) const;
    void typecheckTopLevelDecl(Decl& decl, llvm::ArrayRef<llvm::StringRef> importSearchPaths,
                               ParserFunction& parse) const;
    void postProcess();

private:
    void typecheckFunctionLikeDecl(FunctionLikeDecl& decl) const;
    void typecheckInitDecl(InitDecl& decl) const;
    void typecheckMemberDecl(Decl& decl) const;

    void typecheckStmt(Stmt& stmt) const;
    void typecheckAssignStmt(AssignStmt& stmt) const;
    void typecheckReturnStmt(ReturnStmt& stmt) const;
    void typecheckVarStmt(VarStmt& stmt) const;
    void typecheckIncrementStmt(IncrementStmt& stmt) const;
    void typecheckDecrementStmt(DecrementStmt& stmt) const;
    void typecheckIfStmt(IfStmt& ifStmt) const;
    void typecheckSwitchStmt(SwitchStmt& stmt) const;
    void typecheckWhileStmt(WhileStmt& whileStmt) const;
    void typecheckForStmt(ForStmt& forStmt) const;
    void typecheckBreakStmt(BreakStmt& breakStmt) const;
    void typecheckParamDecl(ParamDecl& decl) const;
    void typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams) const;
    void typecheckDeinitDecl(DeinitDecl& decl) const;
    void typecheckTypeDecl(TypeDecl& decl) const;
    void typecheckImportDecl(ImportDecl& decl, llvm::ArrayRef<llvm::StringRef> importSearchPaths,
                             ParserFunction& parse) const;

    Type typecheckVarExpr(VarExpr& expr) const;
    Type typecheckArrayLiteralExpr(ArrayLiteralExpr& expr) const;
    Type typecheckPrefixExpr(PrefixExpr& expr) const;
    Type typecheckBinaryExpr(BinaryExpr& expr) const;
    Type typecheckCallExpr(CallExpr& expr) const;
    Type typecheckBuiltinConversion(CallExpr& expr) const;
    Type typecheckCastExpr(CastExpr& expr) const;
    Type typecheckMemberExpr(MemberExpr& expr) const;
    Type typecheckSubscriptExpr(SubscriptExpr& expr) const;
    Type typecheckUnwrapExpr(UnwrapExpr& expr) const;

    Type resolveTypePlaceholder(llvm::StringRef name) const override;
    bool isInterface(Type type) const;
    bool hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const;
    bool implementsInterface(TypeDecl& type, TypeDecl& interface) const;
    bool isValidConversion(Expr& expr, Type unresolvedSource, Type unresolvedTarget) const;
    bool isValidConversion(llvm::ArrayRef<std::unique_ptr<Expr>> exprs, Type source, Type target) const;
    void setCurrentGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                               CallExpr& call, llvm::ArrayRef<ParamDecl> params) const;
    void setCurrentGenericArgsForGenericFunction(FunctionLikeDecl& functionDecl, CallExpr& callExpr) const;
    std::vector<Type> getGenericArgsAsArray() const;
    std::vector<Type> getUnresolvedGenericArgs() const;
    FunctionLikeDecl& resolveOverload(CallExpr& expr, llvm::StringRef callee) const;
    std::vector<Type> inferGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                       const CallExpr& call, llvm::ArrayRef<ParamDecl> params) const;
    bool validateArgs(llvm::ArrayRef<Argument> args, llvm::ArrayRef<ParamDecl> params,
                      bool isVariadic, llvm::StringRef functionName = "",
                      SourceLocation location = SourceLocation::invalid()) const;
    TypeDecl* getTypeDecl(const BasicType& type) const;
    void addToSymbolTableWithName(Decl& decl, llvm::StringRef name) const;
    template<typename DeclT>
    void addToSymbolTableCheckParams(DeclT& decl) const;
    template<typename DeclT>
    void addToSymbolTableNonAST(DeclT& decl) const;

private:
    Module* currentModule;
    SourceFile* currentSourceFile;
    mutable FunctionLikeDecl* currentFunction;
    mutable std::unordered_map<std::string, Type> currentGenericArgs;
    mutable bool typecheckingGenericFunction;
    mutable std::vector<std::pair<FunctionDecl&, CallExpr&>> genericFunctionInstantiationsToTypecheck;
};

}
