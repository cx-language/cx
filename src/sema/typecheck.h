#pragma once

#include <functional>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/Support/ErrorOr.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/stmt.h"

namespace llvm {
class StringRef;
template<typename T>
class ArrayRef;
template<typename T, unsigned N>
class SmallVector;
} // namespace llvm

namespace delta {

class Module;
class PackageManifest;
class SourceFile;
struct SourceLocation;
struct Type;
struct CompileOptions;

class Typechecker {
public:
    Typechecker(const CompileOptions& options)
    : currentModule(nullptr), currentSourceFile(nullptr), currentFunction(nullptr), currentStmt(nullptr), currentInitializedFields(nullptr),
      isPostProcessing(false), options(options) {}
    void typecheckModule(Module& module, const PackageManifest* manifest);

private:
    Module* getCurrentModule() const { return NOTNULL(currentModule); }
    Type typecheckExpr(Expr& expr, bool useIsWriteOnly = false, Type expectedType = Type());
    void typecheckVarDecl(VarDecl& decl, bool isGlobal);
    void typecheckFieldDecl(FieldDecl& decl);
    void typecheckTopLevelDecl(Decl& decl, const PackageManifest* manifest);
    void typecheckParams(llvm::MutableArrayRef<ParamDecl> params, AccessLevel userAccessLevel);
    void typecheckFunctionDecl(FunctionDecl& decl);
    void typecheckFunctionTemplate(FunctionTemplate& decl);
    void typecheckMethodDecl(Decl& decl);

    void typecheckStmt(Stmt*& stmt);
    void typecheckCompoundStmt(CompoundStmt& stmt);
    void typecheckReturnStmt(ReturnStmt& stmt);
    void typecheckVarStmt(VarStmt& stmt);
    void typecheckIfStmt(IfStmt& ifStmt);
    void typecheckSwitchStmt(SwitchStmt& stmt);
    void typecheckWhileStmt(WhileStmt& whileStmt);
    void typecheckBreakStmt(BreakStmt& breakStmt);
    void typecheckContinueStmt(ContinueStmt& continueStmt);
    void typecheckType(Type type, AccessLevel userAccessLevel);
    void typecheckParamDecl(ParamDecl& decl, AccessLevel userAccessLevel);
    void typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams, AccessLevel userAccessLevel);
    void typecheckTypeDecl(TypeDecl& decl);
    void typecheckTypeTemplate(TypeTemplate& decl);
    void typecheckEnumDecl(EnumDecl& decl);
    void typecheckImportDecl(ImportDecl& decl, const PackageManifest* manifest);

    Type typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly);
    Type typecheckArrayLiteralExpr(ArrayLiteralExpr& expr, Type expectedType = Type());
    Type typecheckTupleExpr(TupleExpr& expr);
    Type typecheckUnaryExpr(UnaryExpr& expr);
    Type typecheckBinaryExpr(BinaryExpr& expr);
    void typecheckAssignment(Expr& lhs, Expr& rhs, SourceLocation location);
    Type typecheckCallExpr(CallExpr& expr, Type expectedType = Type());
    Type typecheckBuiltinConversion(CallExpr& expr);
    Type typecheckBuiltinCast(CallExpr& expr);
    Type typecheckSizeofExpr(SizeofExpr& expr);
    Type typecheckAddressofExpr(AddressofExpr& expr);
    Type typecheckMemberExpr(MemberExpr& expr);
    Type typecheckIndexExpr(IndexExpr& expr);
    Type typecheckUnwrapExpr(UnwrapExpr& expr);
    Type typecheckLambdaExpr(LambdaExpr& expr, Type expectedType);
    Type typecheckIfExpr(IfExpr& expr);

    bool hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const;
    bool providesInterfaceRequirements(TypeDecl& type, TypeDecl& interface, std::string* errorReason) const;
    bool convert(Expr* expr, Type type, bool allowPointerToTemporary = false) const;
    /// Returns the converted type when the implicit conversion succeeds, or the null type when it doesn't.
    Type isImplicitlyConvertible(const Expr* expr, Type source, Type target, bool allowPointerToTemporary = false) const;
    llvm::StringMap<Type> getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                llvm::ArrayRef<ParamDecl> params, bool returnOnError, Type expectedType);
    Decl* findDecl(llvm::StringRef name, SourceLocation location) const;
    std::vector<Decl*> findDecls(llvm::StringRef name, TypeDecl* receiverTypeDecl = nullptr, bool inAllImportedModules = false) const;
    std::vector<Decl*> findCalleeCandidates(const CallExpr& expr, llvm::StringRef callee);
    Decl* resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee, Type expectedType);
    std::vector<Type> inferGenericArgsFromCallArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                   llvm::ArrayRef<ParamDecl> params, bool returnOnError);
    bool argumentsMatch(CallExpr& expr, const FunctionDecl* functionDecl, llvm::ArrayRef<ParamDecl> params = {});
    void validateArgs(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName = "", SourceLocation location = SourceLocation());
    void validateArgs(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic, llvm::StringRef functionName = "",
                      SourceLocation location = SourceLocation());
    TypeDecl* getTypeDecl(const BasicType& type);
    EnumCase* getEnumCase(const Expr& expr);
    void checkReturnPointerToLocal(const ReturnStmt& stmt) const;
    static void checkHasAccess(const Decl& decl, SourceLocation location, AccessLevel userAccessLevel);
    void checkLambdaCapture(const VariableDecl& variableDecl, const VarExpr& varExpr) const;
    llvm::ErrorOr<const Module&> importDeltaModule(SourceFile* importer, const PackageManifest* manifest, llvm::StringRef moduleName);
    void postProcess();

    /// Returns true if the given expression (of optional type) is guaranteed to be non-null, e.g.
    /// if it was previously checked against null, and the type-checker can prove that it wasn't set
    /// back to null after that check.
    bool isGuaranteedNonNull(const Expr& expr) const;
    bool isGuaranteedNonNull(const Expr& expr, const Stmt& currentControlStmt) const;

    /// Returns true if the given if-statement returns when the given variable is null.
    static bool isEarlyExitNullCheck(const Expr& expr, const IfStmt& stmt);

    /// Returns true if executing the given statement/expression/block might result in 'null' being
    /// assigned to the given variable before evaluating it. Returns false if the variable is
    /// guaranteed to be null when evaluating it. Returns an empty optional if the expression didn't
    /// contain the given variable, but also can't result in 'null' being assigned to it.
    llvm::Optional<bool> maySetToNullBeforeEvaluating(const Expr& var, const Stmt& stmt) const;
    llvm::Optional<bool> subExprMaySetToNullBeforeEvaluating(const Expr& var, const Expr& expr) const;
    llvm::Optional<bool> maySetToNullBeforeEvaluating(const Expr& var, const Expr& expr) const;
    llvm::Optional<bool> maySetToNullBeforeEvaluating(const Expr& var, llvm::ArrayRef<Stmt*> block) const;

    bool isWarningEnabled(llvm::StringRef warning) const;

private:
    Module* currentModule;
    SourceFile* currentSourceFile;
    llvm::ArrayRef<GenericParamDecl> currentGenericParams;
    FunctionDecl* currentFunction;
    Stmt* currentStmt;
    std::vector<Stmt*> currentControlStmts;
    llvm::SmallPtrSet<FieldDecl*, 32>* currentInitializedFields;
    Type functionReturnType;
    bool isPostProcessing;
    std::vector<Decl*> declsToTypecheck;
    const CompileOptions& options;
};

void validateGenericArgCount(size_t genericParamCount, llvm::ArrayRef<Type> genericArgs, llvm::StringRef name, SourceLocation location);

} // namespace delta
