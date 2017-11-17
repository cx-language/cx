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
                     llvm::ArrayRef<std::string> importSearchPaths,
                     llvm::ArrayRef<std::string> frameworkSearchPaths,
                     ParserFunction& parse);

class Typechecker {
public:
    explicit Typechecker(Module* currentModule, SourceFile* currentSourceFile)
    : currentModule(currentModule), currentSourceFile(currentSourceFile), currentFunction(nullptr),
      isPostProcessing(false) {}

    Module* getCurrentModule() const { return currentModule; }
    const SourceFile* getCurrentSourceFile() const { return currentSourceFile; }

    Decl& findDecl(llvm::StringRef name, SourceLocation location, bool everywhere = false) const;
    std::vector<Decl*> findDecls(llvm::StringRef name, bool everywhere = false,
                                 TypeDecl* receiverTypeDecl = nullptr) const;
    std::vector<Decl*> findCalleeCandidates(const CallExpr& expr, llvm::StringRef callee);

    void addToSymbolTable(FunctionTemplate& decl) const;
    void addToSymbolTable(FunctionDecl& decl) const;
    void addToSymbolTable(FunctionDecl&& decl) const;
    void addToSymbolTable(TypeTemplate& decl) const;
    void addToSymbolTable(TypeDecl& decl) const;
    void addToSymbolTable(TypeDecl&& decl) const;
    void addToSymbolTable(EnumDecl& decl) const;
    void addToSymbolTable(VarDecl& decl, bool global) const;
    void addToSymbolTable(VarDecl&& decl) const;
    void addIdentifierReplacement(llvm::StringRef source, llvm::StringRef target) const;

    Type typecheckExpr(Expr& expr, bool useIsWriteOnly = false);
    void typecheckVarDecl(VarDecl& decl, bool isGlobal);
    void typecheckFieldDecl(FieldDecl& decl);
    void typecheckTopLevelDecl(Decl& decl, const PackageManifest* manifest,
                               llvm::ArrayRef<std::string> importSearchPaths,
                               llvm::ArrayRef<std::string> frameworkSearchPaths,
                               ParserFunction& parse);
    void postProcess();

private:
    void typecheckParams(llvm::MutableArrayRef<ParamDecl> params);
    void typecheckFunctionDecl(FunctionDecl& decl);
    void typecheckFunctionTemplate(FunctionTemplate& decl);
    void typecheckMemberDecl(Decl& decl);

    void typecheckStmt(Stmt& stmt);
    void typecheckAssignStmt(AssignStmt& stmt);
    void typecheckCompoundStmt(CompoundStmt& stmt);
    void typecheckReturnStmt(ReturnStmt& stmt);
    void typecheckVarStmt(VarStmt& stmt);
    void typecheckIncrementStmt(IncrementStmt& stmt);
    void typecheckDecrementStmt(DecrementStmt& stmt);
    void typecheckIfStmt(IfStmt& ifStmt);
    void typecheckSwitchStmt(SwitchStmt& stmt);
    void typecheckWhileStmt(WhileStmt& whileStmt);
    void typecheckBreakStmt(BreakStmt& breakStmt);
    void typecheckType(Type type, SourceLocation location);
    void typecheckParamDecl(ParamDecl& decl);
    void typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams);
    void typecheckTypeDecl(TypeDecl& decl);
    void typecheckTypeTemplate(TypeTemplate& decl);
    void typecheckEnumDecl(EnumDecl& decl);
    void typecheckImportDecl(ImportDecl& decl, const PackageManifest* manifest,
                             llvm::ArrayRef<std::string> importSearchPaths,
                             llvm::ArrayRef<std::string> frameworkSearchPaths,
                             ParserFunction& parse);

    Type typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly);
    Type typecheckArrayLiteralExpr(ArrayLiteralExpr& expr);
    Type typecheckTupleExpr(TupleExpr& expr);
    Type typecheckPrefixExpr(PrefixExpr& expr);
    Type typecheckBinaryExpr(BinaryExpr& expr);
    Type typecheckCallExpr(CallExpr& expr);
    Type typecheckBuiltinConversion(CallExpr& expr);
    Type typecheckCastExpr(CastExpr& expr);
    Type typecheckSizeofExpr(SizeofExpr& expr);
    Type typecheckMemberExpr(MemberExpr& expr);
    Type typecheckSubscriptExpr(SubscriptExpr& expr);
    Type typecheckUnwrapExpr(UnwrapExpr& expr);
    Type typecheckLambdaExpr(LambdaExpr& expr);
    Type typecheckIfExpr(IfExpr& expr);

    bool isInterface(Type type);
    bool hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const;
    void checkImplementsInterface(TypeDecl& type, TypeDecl& interface, SourceLocation location) const;
    bool implementsInterface(TypeDecl& type, TypeDecl& interface, std::string* errorReason) const;
    bool isImplicitlyConvertible(const Expr* expr, Type source, Type target, Type* convertedType) const;
    llvm::StringMap<Type> getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams,
                                                CallExpr& call, llvm::ArrayRef<ParamDecl> params,
                                                bool returnOnError);
    Decl* resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee,
                          bool returnNullOnError = false);
    std::vector<Type> inferGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams,
                                       CallExpr& call, llvm::ArrayRef<ParamDecl> params,
                                       bool returnOnError);
    bool isImplicitlyCopyable(Type type);
    bool argumentsMatch(const CallExpr& expr, const FunctionDecl* functionDecl,
                        llvm::ArrayRef<ParamDecl> params = {}) const;
    void validateArgs(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName = "",
                      SourceLocation location = SourceLocation::invalid()) const;
    void validateArgs(CallExpr& expr, bool isMutating, llvm::ArrayRef<ParamDecl> params,
                      bool isVariadic, llvm::StringRef functionName = "",
                      SourceLocation location = SourceLocation::invalid()) const;
    TypeDecl* getTypeDecl(const BasicType& type);
    void markFieldAsInitialized(Expr& expr) const;
    void addToSymbolTableWithName(Decl& decl, llvm::StringRef name, bool global) const;
    template<typename DeclT>
    void addToSymbolTableNonAST(DeclT& decl) const;

private:
    Module* currentModule;
    SourceFile* currentSourceFile;
    FunctionDecl* currentFunction;
    bool isPostProcessing;
    std::vector<Decl*> declsToTypecheck;
};

}
