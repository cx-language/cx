#pragma once

#include <functional>
#include <string>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/ErrorOr.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/stmt.h"
#include "../build/config.h"
#include "../driver/driver.h"

namespace llvm {
class StringRef;
template<typename T> class ArrayRef;
template<typename T, unsigned N> class SmallVector;
template<typename T> class Optional;
} // namespace llvm

namespace cx {

struct Module;
struct SourceFile;
struct Location;
struct Type;

struct ArgumentValidation {
    enum Error { None, TooFew, TooMany, InvalidName, DuplicateName, InvalidType };

    Error error;
    int index;
    bool didConvertArguments;
    bool didUnwrapOptional;
    bool didWrapOptional;

    static ArgumentValidation success(bool didConvertArguments, bool didUnwrapOptional, bool didWrapOptional) {
        return {None, -1, didConvertArguments, didUnwrapOptional, didWrapOptional};
    }
    static ArgumentValidation tooFew() { return {TooFew, -1, false, false, false}; }
    static ArgumentValidation tooMany() { return {TooMany, -1, false, false, false}; }
    static ArgumentValidation invalidName(size_t index) { return {InvalidName, int(index), false, false, false}; }
    static ArgumentValidation duplicateName(size_t index) { return {DuplicateName, int(index), false, false, false}; }
    static ArgumentValidation invalidType(size_t index) { return {InvalidType, int(index), false, false, false}; }
};

struct Match {
    Decl* decl;
    bool didConvertArguments;
    bool didUnwrapOptional;
    bool didWrapOptional;
};

struct VariadicGenericArgs {
    llvm::StringMap<GenericArg> fixedArgs;
    std::vector<llvm::StringMap<GenericArg>> packArgs;
    std::vector<GenericArg> cacheKey;
};

// Variables proven non-null by an enclosing null check, mapped to their unwrapped type.
using NarrowMap = llvm::DenseMap<Decl*, Type>;

struct Typechecker {
    Typechecker(const CompileOptions& options, const std::vector<BuildConfig::ResolvedDependency>* dependencies = nullptr)
    : currentModule(nullptr), currentSourceFile(nullptr), currentFunction(nullptr), currentStmt(nullptr), currentInitializedFields(nullptr),
      isPostProcessing(false), options(options), dependencies(dependencies) {}
    void typecheckModule(Module& module, const CompileOptions& packageOptions);
    void checkUnusedDecls(const Module& mainModule);

    Type typecheckExpr(Expr& expr, bool useIsWriteOnly = false, Type expectedType = Type());
    void typecheckVarDecl(VarDecl& decl);
    void typecheckFieldDecl(FieldDecl& decl);
    void typecheckTopLevelDecl(Decl& decl);
    void typecheckParams(llvm::MutableArrayRef<ParamDecl> params, AccessLevel userAccessLevel);
    void typecheckFunctionDecl(FunctionDecl& decl);
    void typecheckFunctionTemplate(FunctionTemplate& decl);
    void typecheckMethodDecl(Decl& decl);

    bool typecheckStmt(Stmt*& stmt);
    void typecheckCompoundStmt(CompoundStmt& stmt);
    void typecheckReturnStmt(ReturnStmt& stmt);
    void typecheckVarStmt(VarStmt& stmt);
    void typecheckIfStmt(IfStmt& ifStmt);
    void typecheckSwitchStmt(SwitchStmt& stmt);
    Type typecheckSwitchCondition(Expr*& condition);
    EnumCase* typecheckSwitchCaseValue(Expr*& value, Type conditionType);
    void typecheckSwitchCaseBinding(VarDecl* associatedValue, EnumCase* enumCase);
    void warnAboutUnhandledEnumCases(const SwitchStmt& stmt, Type conditionType) const;
    void typecheckForStmt(ForStmt& forStmt);
    void typecheckDoWhileStmt(DoWhileStmt& doWhileStmt);
    void typecheckBreakStmt(BreakStmt& breakStmt);
    void typecheckContinueStmt(ContinueStmt& continueStmt);
    void typecheckType(Type type, AccessLevel userAccessLevel, bool recheckGenericArgs = true, bool allowReference = false);
    void typecheckParamDecl(ParamDecl& decl, AccessLevel userAccessLevel);
    void typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams, AccessLevel userAccessLevel);
    void typecheckTypeDecl(TypeDecl& decl);
    void typecheckTypeTemplate(TypeTemplate& decl);
    void typecheckEnumDecl(EnumDecl& decl);
    void typecheckImportDecl(ImportDecl& decl);

    Type typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly, Type expectedType);
    Type typecheckNullLiteralExpr(NullLiteralExpr& expr, Type expectedType);
    Type typecheckArrayLiteralExpr(ArrayLiteralExpr& expr, Type expectedType = Type());
    Type typecheckAnonymousStructExpr(AnonymousStructExpr& expr);
    Type typecheckUnaryExpr(UnaryExpr& expr);
    Type typecheckBinaryExpr(BinaryExpr& expr);
    Type typecheckNullCoalescingExpr(BinaryExpr& expr);
    void typecheckAssignment(BinaryExpr& expr, Location location);
    Type typecheckCallExpr(CallExpr& expr, Type expectedType = Type());
    Type typecheckBuiltinConversion(CallExpr& expr);
    Type typecheckBuiltinCast(CallExpr& expr);
    Type typecheckSizeofExpr(SizeofExpr& expr);
    Type typecheckMemberExpr(MemberExpr& expr, Type expectedType = Type(), bool useIsWriteOnly = false);
    Type typecheckIndexExpr(IndexExpr& expr, bool baseIsWriteOnly = false);
    Type typecheckIndexAssignmentExpr(IndexAssignmentExpr& expr);
    Type typecheckUnwrapExpr(UnwrapExpr& expr);
    Type typecheckLambdaExpr(LambdaExpr& expr, Type expectedType);
    Type typecheckIfExpr(IfExpr& expr);
    Type typecheckSwitchExpr(SwitchExpr& expr, Type expectedType);

    bool hasMethod(TypeDecl& type, FunctionDecl& functionDecl) const;
    bool providesInterfaceRequirements(TypeDecl& type, TypeDecl& interface, std::string* errorReason) const;
    /// Returns the converted expression if the conversion succeeds, or null otherwise.
    /// Probing conversions (where failure falls back to another attempt) pass diagnoseOutOfRange=false
    /// so an out-of-range literal doesn't abort the still-untried alternatives.
    Expr* convert(Expr* expr, Type type, bool allowPointerToTemporary = false, bool diagnoseOutOfRange = true) const;
    /// Returns the converted type when the implicit conversion succeeds, or the null type when it doesn't.
    Type isImplicitlyConvertible(const Expr* expr, Type source, Type target, bool allowPointerToTemporary = false,
                                 std::optional<ImplicitCastExpr::Kind>* implicitCastKind = nullptr, bool diagnoseOutOfRange = true) const;
    void typecheckImplicitlyBoolConvertibleExpr(Type type, Location location, Location endLocation, bool positive = true);
    GenericArg findGenericArg(Type argType, Type paramType, llvm::StringRef genericParam, bool inFunctionType = false);
    llvm::StringMap<GenericArg> getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, FunctionDecl* decl, bool returnOnError,
                                                      Type expectedType);
    Decl* findDecl(llvm::StringRef name, Location location, Location endLocation = {}) const;
    std::vector<Decl*> findDecls(llvm::StringRef name, TypeDecl* receiverTypeDecl = nullptr, bool inAllImportedModules = false) const;
    std::vector<Decl*> findCalleeCandidates(const CallExpr& expr, llvm::StringRef callee);
    Decl* resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee, Type expectedType, bool allowCommutativeRetry = true);
    std::vector<GenericArg> inferGenericArgsFromCallArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, llvm::ArrayRef<ParamDecl> params,
                                                         bool returnOnError);
    std::optional<VariadicGenericArgs> inferVariadicGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                                llvm::ArrayRef<ParamDecl> params, bool returnOnError);
    ArgumentValidation getArgumentValidationResult(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic);
    std::optional<Match> matchArguments(CallExpr& expr, Decl* calleeDecl, llvm::ArrayRef<ParamDecl> params = {});
    void validateAndConvertArguments(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName = "", Location location = Location());
    void validateAndConvertArguments(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic, llvm::StringRef callee = "",
                                     Location location = Location(), const Decl* calleeDecl = nullptr);
    TypeDecl* getTypeDecl(const BasicType& type);
    EnumCase* getEnumCase(const Expr& expr, Type expectedType = Type(), CallExpr* call = nullptr);
    EnumCase* getExpectedEnumCase(llvm::StringRef name, Type expectedType);
    EnumCase* instantiateEnumCase(TypeTemplate& typeTemplate, llvm::StringRef caseName, const MemberExpr& memberExpr, CallExpr* call, Type expectedType);
    void checkReturnPointerToLocal(const Expr* returnValue) const;
    void warnIfUnusedResult(const Expr& expr, Type type) const;
    static void checkHasAccess(const Decl& decl, Location location, AccessLevel userAccessLevel);
    void maybeCaptureVariable(VariableDecl& variableDecl);
    llvm::ErrorOr<const Module&> importModule(SourceFile* importer, llvm::StringRef moduleName);
    void deferTypechecking(Decl* decl);
    void postProcess();

    void setMoved(Expr* expr, bool isMoved);
    void checkNotMoved(const Decl& decl, const VarExpr& expr);

    void applyNarrowings(const Expr& condition, bool polarity);
    void intersectNarrowings(const NarrowMap& other);
    void dropNarrowingsForNames(const llvm::StringSet<>& names);
    bool validateGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, llvm::ArrayRef<GenericArg> genericArgs, llvm::StringRef name, Location location);
    bool genericArgsMatch(llvm::ArrayRef<GenericParamDecl> genericParams, llvm::ArrayRef<GenericArg> genericArgs);

    Module* currentModule;
    SourceFile* currentSourceFile;
    FunctionDecl* currentFunction;
    Stmt** currentStmt; // Double-pointer so it refers to the correct statement after lowering.
    std::vector<Stmt*> currentControlStmts;
    llvm::SmallPtrSet<FieldDecl*, 32>* currentInitializedFields;
    llvm::SmallPtrSet<Decl*, 32> movedDecls;
    std::vector<VarDecl*> localVarDecls;
    NarrowMap narrowedTypes;
    llvm::SmallPtrSet<Decl*, 32> definitelyAssignedDecls;
    bool isPostProcessing;
    std::vector<Decl*> declsToTypecheck;
    CompileOptions options; // Active package's options; switched per module.
    const std::vector<BuildConfig::ResolvedDependency>* dependencies; // Closure, or null without a project.
};

void validateGenericArgCount(size_t genericParamCount, llvm::ArrayRef<GenericArg> genericArgs, llvm::StringRef name, Location location);
// Returns the enum case tested by an `is` expression's right side, or null when it isn't one.
EnumCase* getIsEnumCase(Expr& expr);
bool containsGenericParam(Type type, llvm::StringRef genericParam);
void diagnoseClosureConversion(Type source, Type target, Location location);
// Suggests an explicit conversion when a value of one numeric type is used where another is expected.
std::string narrowingHint(Type source, Type target);

} // namespace cx
