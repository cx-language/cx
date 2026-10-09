#pragma once

#include <functional>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <vector>
#pragma warning(push, 0)
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/FunctionExtras.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallVector.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/ErrorOr.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/decl.h"
#include "../ast/expr.h"
#include "../ast/module.h"
#include "../ast/stmt.h"
#include "../build/config.h"
#include "../driver/driver.h"

namespace llvm {
class StringRef;
template<typename T> class ArrayRef;
} // namespace llvm

namespace cx {

struct Module;
struct SourceFile;
struct Location;
struct Type;

using DeclSet = llvm::SmallPtrSet<Decl*, 32>;

struct ArgumentValidation {
    enum Error { None, TooFew, TooMany, InvalidName, DuplicateName, InvalidType };

    Error error;
    int index;
    bool didConvertArguments;
    bool didUnwrapOptional;
    bool didWrapOptional;
    int userConversionCount = 0;

    static ArgumentValidation success(bool didConvertArguments, bool didUnwrapOptional, bool didWrapOptional, int userConversionCount) {
        return {None, -1, didConvertArguments, didUnwrapOptional, didWrapOptional, userConversionCount};
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
    int userConversionCount = 0;
};

struct ConstMutationQuery;
struct VariadicGenericArgs {
    llvm::StringMap<GenericArg> fixedArgs;
    std::vector<llvm::StringMap<GenericArg>> packArgs;
    AstVector<GenericArg> cacheKey;
};

// Variables proven non-null by an enclosing null check, mapped to their unwrapped type.
using NarrowMap = llvm::DenseMap<Decl*, Type>;

// Compiler-generated temporaries a lowered `==`/`!=` reads its operands through.
// Null in global initializers, where the lowering reads the operands directly.
// True for a builtin `assert` call whose condition is the literal `false`, allowing for named arguments.
inline bool isFalseAssert(const CallExpr& call) {
    if (call.isMethodCall() || call.getFunctionName() != "assert" || call.args.empty()) return false;
    for (size_t i = 0; i < call.args.size() && i < call.argParamIndices.size(); ++i) {
        if (call.argParamIndices[i] != 0) continue;
        if (auto* condition = llvm::dyn_cast<BoolLiteralExpr>(call.args[i].value)) {
            return !condition->value;
        }
    }
    if (call.argParamIndices.empty()) {
        if (auto* condition = llvm::dyn_cast<BoolLiteralExpr>(call.args[0].value)) {
            return !condition->value;
        }
    }
    return false;
}

struct ComparisonTemps {
    VarDecl* lhsTemp = nullptr;
    VarDecl* rhsTemp = nullptr;
    Expr* lhsBase;
    Expr* rhsBase;
};

// One storage a value may designate: a base object plus the member path
// from the base outward (empty for the whole object). Same-base roots
// overlap when the mutation path is a prefix of the view path, so a whole
// reassignment conflicts with every view into the object while disjoint
// members stay independent. A null base is the caller's `this`, resolved
// where `this` is in scope (bare method calls, bare field references).
struct ViewMemberRoot {
    Decl* base = nullptr;
    llvm::SmallVector<Decl*, 2> path;
    bool operator==(const ViewMemberRoot& other) const { return base == other.base && path == other.path; }
};

struct Typechecker {
    Typechecker(const CompileOptions& options, const std::vector<BuildConfig::ResolvedDependency>* dependencies = nullptr)
    : currentModule(nullptr), currentSourceFile(nullptr), currentFunction(nullptr), currentStmt(nullptr), currentInitializedFields(nullptr),
      isPostProcessing(false), options(options), dependencies(dependencies) {}
    void typecheckModule(Module& module, const CompileOptions& packageOptions, bool isMainModule);
    void checkUnusedDecls(const Module& mainModule);

    Type typecheckExpr(Expr& expr, bool useIsWriteOnly = false, Type expectedType = Type());
    void typecheckVarDecl(VarDecl& decl);
    // True when the expression can initialize a const or global. Shared by the
    // declaration checker and the C importer (for compound-literal macros).
    bool isSupportedConstInitializer(const Expr& expr);
    void typecheckFieldDecl(FieldDecl& decl);
    void typecheckTopLevelDecl(Decl& decl);
    void typecheckParams(llvm::MutableArrayRef<ParamDecl> params, AccessLevel userAccessLevel);
    void typecheckFunctionDecl(FunctionDecl& decl);
    void typecheckFunctionSignature(FunctionDecl& decl);
    void typecheckFunctionTemplate(FunctionTemplate& decl);
    void typecheckMethodDecl(Decl& decl);
    // Ensures the declaration's signature is checked, checking it on first
    // use. Drives lazy checking of imported modules; main-module decls are
    // already checked eagerly, so this is a no-op for them.
    void ensureSignature(Decl& decl);
    void ensureSignatureImpl(Decl& decl);
    void ensureInterfaces(TypeDecl& decl);
    void markReferenced(Decl* decl);
    // Marks the destructor destroying a value of the given type at scope exit.
    // Explicit destructors mark directly; implicit ones check so members mark.
    void markDestructorFor(Type type);
    // Whole-checks std declarations IRGen references without going through
    // sema name resolution (malloc, assertFail, string.init, operator==),
    // limited to the ones the checked code can actually reach.
    void ensureImplicitRuntimeUses(const Module& mainModule);
    // Points name resolution at the declaration's own module and file.
    // Lazily checked declarations from imported modules must resolve names
    // in their own scope, not the use site's. Callers save and restore.
    void setDeclContext(Decl& decl);

    bool typecheckStmt(Stmt*& stmt);
    void typecheckCompoundStmt(CompoundStmt& stmt);
    void typecheckReturnStmt(ReturnStmt& stmt);
    void typecheckVarStmt(VarStmt& stmt);
    void typecheckIfStmt(IfStmt& ifStmt);
    void typecheckSwitchStmt(SwitchStmt& stmt);
    Type typecheckSwitchCondition(Expr*& condition);
    EnumCase* typecheckSwitchCaseValue(Expr*& value, Type conditionType);
    void typecheckSwitchCaseBinding(VarDecl* associatedValue, EnumCase* enumCase, Expr* subject);
    bool subjectBorrows(Expr* subject);
    void warnAboutUnhandledEnumCases(const SwitchStmt& stmt, Type conditionType, bool hadDefault) const;
    void typecheckForStmt(ForStmt& forStmt);
    void typecheckDoWhileStmt(DoWhileStmt& doWhileStmt);
    void typecheckBreakStmt(BreakStmt& breakStmt);
    void typecheckContinueStmt(ContinueStmt& continueStmt);
    void typecheckType(Type type, AccessLevel userAccessLevel, bool recheckGenericArgs = true, bool allowReference = false);
    Type resolveTypeAliases(Type type, AccessLevel userAccessLevel = AccessLevel::None, bool foldArraySizes = false, bool markFunctionTypesExtern = false);
    Type resolveTypeAliases(Type type, AccessLevel userAccessLevel, llvm::SmallPtrSetImpl<const TypeAliasDecl*>& resolving, bool foldArraySizes = false,
                            bool markFunctionTypesExtern = false);
    // Folds a deferred array size (or bare size name) using visible named
    // constants. Throws the specific diagnostic when it cannot.
    Type resolveArraySize(Expr& sizeExpr, Type elementType, Location location, Location endLocation, Module* homeModule);
    TypeAliasDecl* findTypeAlias(Type type);
    void canonicalizeTypeAliases();
    void typecheckParamDecl(ParamDecl& decl, AccessLevel userAccessLevel);
    void typecheckGenericParamDecls(llvm::ArrayRef<GenericParamDecl> genericParams, AccessLevel userAccessLevel);
    void typecheckTypeDecl(TypeDecl& decl);
    void typecheckTypeSignature(TypeDecl& decl);
    void typecheckTypeTemplate(TypeTemplate& decl);
    void typecheckTypeAliasDecl(TypeAliasDecl& decl);
    void typecheckEnumDecl(EnumDecl& decl);
    void typecheckEnumSignature(EnumDecl& decl);
    void typecheckImportDecl(ImportDecl& decl);

    Type typecheckVarExpr(VarExpr& expr, bool useIsWriteOnly, Type expectedType);
    Type typecheckNullLiteralExpr(NullLiteralExpr& expr, Type expectedType);
    Type typecheckArrayLiteralExpr(ArrayLiteralExpr& expr, Type expectedType = Type());
    Type typecheckAnonymousStructExpr(AnonymousStructExpr& expr);
    Type typecheckUnaryExpr(UnaryExpr& expr);
    Type typecheckBinaryExpr(BinaryExpr& expr);
    Type typecheckOptionalComparison(BinaryExpr& expr);
    Type typecheckStructComparison(BinaryExpr& expr);
    ComparisonTemps createComparisonTemps(BinaryExpr& expr);
    Type finishComparisonLowering(BinaryExpr& expr, Expr* result, VarDecl* lhsTemp, VarDecl* rhsTemp);
    Type typecheckNullCoalescingExpr(BinaryExpr& expr);
    void typecheckAssignment(BinaryExpr& expr);
    Type typecheckCallExpr(CallExpr& expr, Type expectedType = Type());
    Type typecheckBuiltinConversion(CallExpr& expr, Type targetType = Type());
    Type typecheckBuiltinCast(CallExpr& expr);
    Type typecheckSizeofExpr(SizeofExpr& expr);
    Type typecheckMemberExpr(MemberExpr& expr, Type expectedType = Type(), bool useIsWriteOnly = false);
    Type typecheckIndexExpr(IndexExpr& expr, bool baseIsWriteOnly = false);
    Type typecheckIndexAssignmentExpr(IndexAssignmentExpr& expr);
    Type typecheckUnwrapExpr(UnwrapExpr& expr);
    Type typecheckLambdaExpr(LambdaExpr& expr, Type expectedType);
    Type typecheckIfExpr(IfExpr& expr);
    Type typecheckSwitchExpr(SwitchExpr& expr, Type expectedType);

    bool hasMethod(TypeDecl& type, FunctionDecl& functionDecl);
    bool providesInterfaceRequirements(TypeDecl& type, TypeDecl& interface, std::string* errorReason);
    /// Returns the converted expression if the conversion succeeds, or null otherwise.
    /// Probing conversions (where failure falls back to another attempt) pass diagnoseOutOfRange=false
    /// so an out-of-range literal doesn't abort the still-untried alternatives.
    Expr* convert(Expr* expr, Type type, bool allowPointerToTemporary = false, bool diagnoseOutOfRange = true, bool allowOperatorBorrow = false,
                  bool allowUserConversion = true);
    /// Returns the converted type when the implicit conversion succeeds, or the null type when it doesn't.
    /// usesUserConversion (when given) reports whether a user-declared conversion applies at any
    /// level, including nested under optionals and composite literals.
    Type isImplicitlyConvertible(const Expr* expr, Type source, Type target, bool allowPointerToTemporary = false,
                                 std::optional<ImplicitCastExpr::Kind>* implicitCastKind = nullptr, bool diagnoseOutOfRange = true,
                                 bool allowOperatorBorrow = false, bool allowUserConversion = true, bool* usesUserConversion = nullptr) const;
    /// Warns when a constant float expression loses precision converting to target (-Wconversion).
    void checkLossyFloatConversion(const Expr& expr, Type target) const;
    // Conversion warnings are enabled, outside an explicit cast, and outside std itself.
    bool shouldWarnConversion() const;
    /// Finds the user-declared conversion from source to target: an implicit constructor on the target
    /// or an implicit parameterless member on the source. Pure except for range diagnostics (like the
    /// surrounding probe); null when none applies or several do. viableCount (when given) receives
    /// the number of applicable conversions, so error paths can tell "none" from "ambiguous".
    FunctionDecl* findUserConversion(const Expr* expr, Type source, Type target, int* viableCount = nullptr, bool diagnoseOutOfRange = false) const;
    /// Explains a failed conversion when several user-declared conversions apply, empty otherwise.
    std::string ambiguousConversionHint(const Expr* expr, Type source, Type target) const;
    /// Whether the types alone would convert and only the source being a constant blocks it.
    bool isConstBlockedConversion(const Expr& expr, Type source, Type target) const;
    /// Commits a user-declared conversion found by findUserConversion: checks and references the
    /// conversion function, converts the operand to its parameter (constructors), and wraps both in
    /// a UserConversion cast. Null when the operand no longer converts.
    Expr* convertWithUserConversion(Expr* expr, Type target, bool diagnoseOutOfRange, bool allowOperatorBorrow);
    void typecheckImplicitlyBoolConvertibleExpr(Expr*& expr, bool positive = true);
    GenericArg findGenericArg(Type argType, Type paramType, llvm::StringRef genericParam, bool inFunctionType = false);
    llvm::StringMap<GenericArg> getGenericArgsForCall(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, FunctionDecl* decl, bool returnOnError,
                                                      Type expectedType);
    Decl* findDecl(llvm::StringRef name, Location location, Location endLocation = {});
    /// findDecl that returns null on unknown identifier instead of reporting it. Still throws on ambiguous reference.
    Decl* tryFindDecl(llvm::StringRef name, Location location);
    /// Hint note when an unknown identifier is a skipped function-like C macro.
    std::optional<Note> getSkippedMacroNote(llvm::StringRef name) const;
    std::vector<Decl*> findDecls(llvm::StringRef name, TypeDecl* receiverTypeDecl = nullptr, bool inAllImportedModules = false);
    std::vector<Decl*> findCalleeCandidates(const CallExpr& expr, llvm::StringRef callee);
    Decl* resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee, Type expectedType, bool allowCommutativeRetry = true);
    std::vector<GenericArg> inferGenericArgsFromCallArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, llvm::ArrayRef<ParamDecl> params,
                                                         bool returnOnError);
    std::optional<VariadicGenericArgs> inferVariadicGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                                llvm::ArrayRef<ParamDecl> params, bool returnOnError);
    ArgumentValidation getArgumentValidationResult(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic);
    std::optional<Match> matchArguments(CallExpr& expr, Decl* calleeDecl, llvm::ArrayRef<ParamDecl> params = {});
    void validateAndConvertArguments(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName = "", bool diagnose = true);
    void validateAndConvertArguments(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic, llvm::StringRef callee = "",
                                     const Decl* calleeDecl = nullptr, bool diagnose = true);
    TypeDecl* getTypeDecl(const BasicType& type);
    // Instantiates and registers the generic types nested in field position
    // (transitively), so memberless queries (copyability, destruction) resolve
    // instead of defaulting. Placeholder-containing types keep today's leniency.
    void ensureNestedInstantiations(TypeDecl& instantiation);
    void ensureNestedInstantiations(Type type);
    EnumCase* getEnumCase(const Expr& expr, Type expectedType = Type(), CallExpr* call = nullptr);
    EnumCase* getExpectedEnumCase(llvm::StringRef name, Type expectedType);
    VarDecl* getStaticConst(const Expr& expr);
    EnumCase* instantiateEnumCase(TypeTemplate& typeTemplate, llvm::StringRef caseName, const MemberExpr& memberExpr, CallExpr* call, Type expectedType);
    void checkReturnPointerToLocal(const Expr* returnValue) const;
    void checkReturnBorrowedView(const Expr* returnValue) const;
    void warnIfUnusedResult(const Expr& expr, Type type) const;
    void checkHasAccess(const Decl& decl, Location location, AccessLevel userAccessLevel);
    bool inSameModule(const Decl& decl, Location location) const;
    Module* findModuleForFile(const char* file) const;
    void maybeCaptureVariable(VariableDecl& variableDecl);
    llvm::ErrorOr<const Module&> importModule(SourceFile* importer, llvm::StringRef moduleName);
    void deferTypechecking(Decl* decl);
    void postProcess();
    void checkDelegationLiveness();
    // Options of the package owning the declaration. Main-module and
    // unresolvable declarations keep the ambient options.
    const CompileOptions& packageOptionsFor(const Decl& decl) const;

    // Marks an expression consumed by a move. Always flags the tree (IRGen skips
    // temporary-destructor registration for flagged constructor calls); only records
    // named declarations as moved when trackVars holds, so copies into copyable
    // consumers keep working while their temps are still recognized as consumed.
    void setMoved(Expr* expr, bool isMoved, bool trackVars = true);
    // Marks the moves in one conditional branch operand (ternary arm, `??`
    // side), returning the declarations newly moved there. Moves are detected
    // by recorded location changes: re-marking an already-moved value leaves
    // the moved set unchanged, so set diffs alone would miss second-branch
    // moves. Declarations in preDecls (moved before the construct) are skipped.
    DeclSet collectBranchMoves(Expr* branch, const DeclSet& preDecls, bool isMoved, bool trackVars);
    // Moves ownership out of a projection source (member/index base, unwrap operand,
    // binding subject): owned roots are consumed, temporaries are flagged for
    // destructor elision, and borrowed roots are an error (nothing skips for them).
    void propagateMove(Expr* source, bool trackVars, Location location, bool checkLoop = false);
    void checkNotMoved(const Decl& decl, const VarExpr& expr);

    void applyNarrowings(const Expr& condition, bool polarity);
    void intersectNarrowings(const NarrowMap& other);
    void dropNarrowingsForNames(const llvm::StringSet<>& names);
    void dropNarrowingForAddressArg(const Expr& arg, Type paramType);
    static VariableDecl* getEnumNarrowableDecl(const VarExpr& varExpr);
    void narrowEnumCaseComparison(const Expr& lhs, const Expr& rhs, BinaryOperator op, bool polarity);
    void narrowEnumSubjectToCase(const Expr* subject, const EnumCase& enumCase);
    // Narrows a subject to a matched case payload (empty view if payload-less).
    // Optionals unwrap to the wrapped type instead; None erases.
    void applyEnumCaseNarrowing(VariableDecl* varDecl, const EnumCase& enumCase);
    // Restores the whole enum type of an expression narrowed to a case payload.
    // Used where the full value is consumed: switch conditions, `is`, and `&`.
    static void unnarrowEnumView(Expr& expr);
    bool genericArgSatisfiesConstraints(const GenericParamDecl& genericParam, GenericArg genericArg, const llvm::StringMap<GenericArg>& resolvedArgs);
    bool validateGenericConstraints(llvm::ArrayRef<GenericParamDecl> genericParams, llvm::ArrayRef<GenericArg> genericArgs, llvm::StringRef name,
                                    Location location);
    bool validateGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, llvm::ArrayRef<GenericArg> genericArgs, llvm::StringRef name, Location location);
    bool genericArgsMatch(llvm::ArrayRef<GenericParamDecl> genericParams, llvm::ArrayRef<GenericArg> genericArgs);
    bool trySynthesizePrintMethod(TypeDecl& decl, bool silent);
    bool tryEnsurePrintable(TypeDecl& typeDecl, const TypeDecl& interface, bool silent);
    bool tryDesugarEnumIteration(ForEachStmt& forEachStmt);
    TypeDecl* getPrintableDecl();
    bool isConcreteType(Type type);

    Module* currentModule;
    Module* mainModule = nullptr;
    // The program entry point, once seen: the single non-generic,
    // non-method "main" in the main module. Set in typecheckFunctionSignature.
    FunctionDecl* entryMain = nullptr;
    SourceFile* currentSourceFile;
    bool suppressAccessWarnings = false;
    // Set while canonicalizing aliases: resolutions must not trigger checking,
    // which would check declarations out of order. The eager loop and lazy
    // uses ensure signatures afterward.
    bool suppressEnsureSignature = false;
    // Set while speculatively resolving an overload (commuted ==/!= retry): resolution failures
    // return null instead of throwing, since the caller falls through to another strategy whose
    // own diagnostics (or success) apply. Callers under this flag must handle a null callee;
    // nested calls re-throw a silent error instead of propagating null types (see typecheckCallExpr).
    bool overloadProbe = false;
    FunctionDecl* currentFunction;
    // Non-null while re-checking a default value: member fallbacks stop before
    // this function, so names resolve as at the default's declaration.
    FunctionDecl* defaultScopeStop = nullptr;
    Stmt** currentStmt; // Double-pointer so it refers to the correct statement after lowering.
    std::vector<Stmt*> currentControlStmts;
    llvm::SmallPtrSet<FieldDecl*, 32>* currentInitializedFields;
    DeclSet movedDecls;
    // Values moved on only some paths through a conditional expression (ternary,
    // switch expression), where no statement can destroy the live paths: using
    // one warns, and its destructor is skipped like a moved value (leaking
    // the live paths). Statement branches instead destroy live paths at the
    // merge and mark the value moved (see makeMergeDrop).
    DeclSet maybeMovedDecls;
    // Move state captured at each `break` out of a switch arm: breaks reach
    // past the switch, but the if-merge drops them as diverging. Reset per
    // switch (breaks target the innermost one); the merge unions them as
    // extra paths.
    struct SwitchBreakPath {
        DeclSet moved, maybeMoved, assigned;
        BreakStmt* breakStmt = nullptr;
    };
    std::vector<SwitchBreakPath> switchBreakPaths;
    // Most recent move site per declaration, so branch merges can warn where
    // a value was conditionally moved.
    llvm::DenseMap<Decl*, Location> moveLocations;
    // Marks a declaration moved-from at the given move site.
    void markMoved(Decl* decl, Location location);
    enum class ConditionalMoveSite { IfThen, IfThenNoElse, IfElse, Switch, SwitchExpr, ShortCircuitAnd, ShortCircuitOr, NullCoalescing };
    // Guards and move-site lookup shared by conditional-move warnings: skips
    // payload bindings and branch-locals, returns the move site if recorded.
    std::optional<Location> locateConditionalMoveWarning(Decl* decl, size_t branchEntryLocalCount, const llvm::DenseMap<Decl*, Location>& locations);
    // Declarations already warned for a conditional-expression move (ternary
    // arm, `??` side). Nested expressions warn for their own branches; the
    // outer ones would only repeat them, so each move warns once until the
    // value is reassigned.
    DeclSet condWarnedDecls;
    // Warns that a value is moved in one ternary arm and leaks when the other
    // arm is taken. Each move warns once (see condWarnedDecls).
    void warnTernaryMove(Decl* decl, bool isThenArm, size_t branchEntryLocalCount);
    // Warns that a value is moved on only some paths through a branch, pointing
    // at its move in the given branch's move map. Branch-local values and
    // payload bindings (which borrow) never leak, so only values declared
    // before the branch warn.
    void warnAboutConditionalMove(Decl* decl, ConditionalMoveSite site, size_t branchEntryLocalCount, const llvm::DenseMap<Decl*, Location>& locations);
    // Merges per-path move sets after a switch: moves on every path stay moved,
    // nested maybe-moves union into maybeMovedDecls. Returns the declarations
    // moved on some but not all paths; the caller resolves each (destroy on
    // live paths for statements, maybe-move and warn for expressions).
    DeclSet mergeConditionalMoves(const std::vector<DeclSet>& pathMoved, const std::vector<DeclSet>& pathMaybe);
    // Keeps declarations assigned on every path. `paths` must be non-empty.
    void intersectDefinitelyAssigned(llvm::ArrayRef<DeclSet> paths);
    // Merges expression-branch move sets (switch-expression arms, or a
    // short-circuit RHS against entry): moves on every path stay moved,
    // partial moves keep the maybe state and warn.
    void mergeExpressionMoves(const std::vector<DeclSet>& pathMoved, const std::vector<DeclSet>& pathMaybe, ConditionalMoveSite site,
                              size_t branchEntryLocalCount);
    // Typechecks a short-circuit RHS under saved move state, then merges it
    // against the entry state (the RHS may not execute).
    Type typecheckShortCircuitRHS(llvm::function_ref<Type()> checkRHS, ConditionalMoveSite site);
    // Builds and typechecks a `drop(decl)` statement destroying a value that is
    // live on one merge path but moved on another, so it dies exactly once.
    // Checked with the path's move/assignment state; merge state is restored.
    // The call pins std's `drop`, so user overloads cannot hijack
    // compiler-inserted destruction.
    Stmt* makeMergeDrop(Decl* decl, const DeclSet& pathMoved, const DeclSet& pathAssigned, Location location);
    // Resolves one declaration moved on some merge paths but live on others,
    // warning through warn when it keeps maybe-move (nested expression
    // merges, exotics). Bindings and branch-locals silently mark moved; the
    // rest return true so the caller destroys them on the live paths.
    bool resolveMergeDecl(Decl* decl, size_t branchEntryLocalCount, bool anyPathMaybe, llvm::function_ref<void()> warn);
    // Deterministic destruction order for merge drops: reverse declaration
    // order like scope exit (locals, then parameters), leftovers by position.
    std::vector<Decl*> orderMergeDestroys(const DeclSet& symdiff);
    // localVarDecls size at the innermost enclosing loop-body entry, if any;
    // moving a value declared before it is rejected, since the loop may
    // move it again on the next iteration.
    std::optional<size_t> loopEntryLocalCount;
    void errorIfLoopMove(Decl* decl, Location location);
    // Assignment target whose right-hand side is being checked: moving it there
    // is safe even in a loop, since the assignment replenishes it immediately.
    Decl* assignTarget = nullptr;
    // True while checking a return value: it runs once, so loop moves in it
    // cannot execute again on the next iteration.
    bool inReturnValue = false;
    // Switch-case and `is` bindings borrow their subject's payload; moving out of
    // one consumes the whole subject like moving out of a member does.
    llvm::DenseMap<const Decl*, Expr*> bindingSources;
    // Address-of targets of init-bound local pointer variables, so deinit
    // through a dereference can consume a stack target like a direct deinit
    // does. Null means forgotten (reseated or address-taken: deinit through
    // it is an error). Anything else (params, members, heap, globals,
    // complex initializers) stays absent and lenient.
    llvm::DenseMap<const Decl*, Decl*> deinitPtrTargets;
    // Outer locals a closure body deinits through a dereference. The body
    // names only the pointer, so they are not captures; the lambda epilogue
    // marks them moved like the capture loop does. Entries from finished
    // functions are inert: move state is decl-keyed and unnameable decls are
    // never queried.
    std::vector<Decl*> closureMovedDecls;
    // Binds an initializing `&local`/tracked-pointer right-hand side;
    // anything else leaves the pointer untracked (lenient).
    void bindDeinitPtrTarget(VarDecl& decl);
    // A definite value change (assignment, write through an alias): forgets
    // the target even for untracked pointers.
    void reseatDeinitPtrTarget(Decl* ptrDecl);
    // A possible value change (address taken, mutable borrow): forgets the
    // target only when one was tracked.
    void taintDeinitPtrTarget(Decl* ptrDecl);
    // Consumes the target of a deinit receiver dereferencing a tracked
    // pointer, like a direct deinit of the target. Returns false (leaving the
    // receiver to the lenient untracked path) when no tracked dereference is
    // crossed; tainted pointers are an error.
    bool consumeTrackedDeinitTarget(Expr* receiver, llvm::StringRef verb);
    // True while checking a placement-`init` argument, which moves out of raw
    // container storage with no owner, so borrow/dereference moves are allowed.
    bool inMoveInit = false;
    // True while checking an explicit `.deinit()` receiver, which destroys in
    // place with nothing copied out, so dereference and implicit place moves
    // are allowed.
    bool inExplicitDeinit = false;
    // True while checking the arguments of a call on a constant receiver
    // (directly or through a call chain): lambdas there receive
    // constant-derived borrows, so their borrow parameters bind const.
    bool callOnConstReceiver = false;
    // True while checking explicit cast<T> arguments: the cast documents the
    // conversion, so -Wconversion stays silent there.
    bool inExplicitCast = false;
    std::vector<VarDecl*> localVarDecls;
    NarrowMap narrowedTypes;
    DeclSet definitelyAssignedDecls;
    bool isPostProcessing;
    std::vector<Decl*> declsToTypecheck;
    // Method calls on constant receivers that passed the call-site checks:
    // postProcess reports the ones whose callee may mutate its receiver (see
    // const-mutation.cpp). Deferred because callee bodies check on demand.
    // The caller guards against speculative re-checks: discarded bodies never
    // reach Checked, and the drain skips them.
    struct ConstReceiverCheck {
        FunctionDecl* callee;
        FunctionDecl* caller;
        Location begin;
        Location end;
        std::string name;
    };
    std::vector<ConstReceiverCheck> pendingConstReceiverChecks;
    // Const-derived views over constants passed as call arguments: like
    // receiver calls, the verdict waits for postProcess since callee bodies
    // check on demand. Read-only callees (e.g. print) stay legal; only
    // callees that may write through the parameter report.
    struct ConstViewArgCheck {
        FunctionDecl* callee;
        size_t paramIndex;
        FunctionDecl* caller;
        Location begin;
        Location end;
        std::string name;
        size_t argNumber;
        bool isIterator;
    };
    std::vector<ConstViewArgCheck> pendingConstViewArgChecks;
    // A use of a view: a mutation conflicts with the view only when a use
    // follows it on some path. The region pinpoints which conditional arms
    // enclose the use; uses in a sibling arm of the mutation never follow it
    // within one pass. The loop is the innermost enclosing loop, or -1.
    struct ViewUse {
        Location loc;
        int region;
        int loop;
    };
    // A local holding a borrow or view: the storages it designates stay
    // frozen (no reassignment, move, destruction, or mutating call) from its
    // declaration until a use follows. Owners that own no storage need no
    // freezing, so only freezable roots are recorded (see below).
    struct ViewFreezeRecord {
        VarDecl* view;
        llvm::SmallVector<ViewMemberRoot, 2> roots;
        Location viewLoc;
        llvm::SmallVector<ViewUse, 4> uses;
        // The view aliases its owner's object (rather than viewing into it):
        // interior mutation leaves the borrowed slot in place, so only moves
        // and destructions conflict.
        bool objectAlias = false;
        // Innermost loop enclosing the declaration, or -1.
        int loop = -1;
    };
    std::vector<ViewFreezeRecord> viewFreezeRecords;
    // Reassignments, moves, and destructions of freezable roots, resolved
    // against view uses when the function body is done.
    struct RootMutation {
        ViewMemberRoot root;
        Location loc;
        bool isMove;
        int region;
        int loop;
        // The named borrow the write goes through, if any: writing through a
        // view cannot invalidate the view itself.
        VarDecl* throughView = nullptr;
    };
    std::vector<RootMutation> viewRootMutations;
    // Method calls and borrow-argument passes on freezable roots: the
    // mutating verdict waits for postProcess since callee bodies check on
    // demand. Calls a view use follows queue a ViewFreezeCallCheck then.
    struct ViewCallCandidate {
        FunctionDecl* callee;
        const ParamDecl* param;
        ViewMemberRoot root;
        // The named view the call goes through (receiver or argument), if any:
        // mutating through a view cannot invalidate the view itself.
        VarDecl* receiverView = nullptr;
        Location begin;
        Location end;
        std::string name;
        int region;
        int loop;
    };
    std::vector<ViewCallCandidate> viewCallCandidates;
    // One if/switch arm on the path from the function body to a use or
    // mutation site. Arms with the same cond but different arms never execute
    // together, so a use there never follows a mutation here. Region -1 is
    // the function body itself.
    struct ViewBranchRegion {
        int cond;
        int arm;
        int parent;
        int depth;
    };
    std::vector<ViewBranchRegion> viewBranchRegions;
    int currentViewRegion = -1;
    int viewBranchCondCounter = 0;
    // Loop-nesting forest: the parent of each loop id, with -1 for top-level
    // loops. currentViewLoop is the innermost enclosing loop, or -1.
    std::vector<int> viewLoopParents;
    int currentViewLoop = -1;
    void pushViewBranchArm(int cond, int arm);
    void popViewBranchArm();
    int newViewBranchCond();
    int newViewLoop();
    bool viewUseFollows(const ViewFreezeRecord& record, Location loc, int region, int loop) const;
    void recordViewLocal(VarDecl& decl);
    // Checks body-recorded mutations and calls (everything past the snapshots)
    // against the range roots: a for loop freezes its range for the whole
    // loop, since the lowered iterator temp carries the loop-line location
    // everywhere and its own window would collapse.
    void checkLoopBodyFreezes(Expr& range, size_t mutationStart, size_t candidateStart);
    void recordViewUse(Decl* decl, Location loc);
    void recordRootMutation(ViewMemberRoot root, Location loc, bool isMove, VarDecl* throughView = nullptr);
    void rebindViewLocal(VarDecl& view, Expr& lhs, Expr& rhs);
    void recordViewCallCandidate(FunctionDecl* callee, const ParamDecl* param, Expr& rootExpr, Location begin, Location end, llvm::StringRef name);
    // Checks one record against recorded mutations and calls. drainEnd bounds
    // the window for rebinds (the old binding ends there); null checks the
    // whole function at the end.
    void checkViewRecord(const ViewFreezeRecord& record, const Location* drainEnd);
    void checkViewFreezes();
    struct ViewFreezeCallCheck {
        FunctionDecl* callee;
        const ParamDecl* param;
        FunctionDecl* caller;
        ViewMemberRoot root;
        std::string viewName;
        bool isLoop;
        Location begin;
        Location end;
        std::string name;
    };
    std::vector<ViewFreezeCallCheck> pendingViewFreezeCallChecks;
    void checkViewFreezeCalls(ConstMutationQuery& query);
    // A whole-field `=` assignment found by the constructor scan, with the
    // state needed to decide whether it overwrites a live value. initsBefore
    // counts delegating `init(...)` calls ahead of it, splitting the body
    // into pre/post delegation segments; the flag marks lambdas (unknown
    // timing).
    struct FieldAssign {
        FieldDecl* field;
        BinaryExpr* assign;
        bool definitelyAssigned;
        int initsBefore;
        bool inLambda;
    };
    // Delegating constructors whose cross-body liveness waits for postProcess:
    // the target body may check later (or never), so pre-init dead stores and
    // post-init overwrites of target-built values resolve once all bodies are
    // checked. Non-Checked callers are speculative noise, like above.
    struct DelegationCheck {
        ConstructorDecl* ctor;
        llvm::SmallVector<FieldAssign, 16> assigns;
        llvm::SmallVector<ConstructorDecl*, 2> targets;
        size_t firstInit;
    };
    std::vector<DelegationCheck> pendingDelegationChecks;
    // Set while checking function signatures (parameters and return type).
    // Types mentioned there materialize no values, so their destructors must
    // not be demand-checked: values are dropped (and their destructors marked)
    // at bodies, variable declarations, and field declarations instead.
    bool checkingFunctionSignature = false;
    // Constructs seen while checking that need implicit runtime declarations
    // at IRGen (see ensureImplicitRuntimeUses). Set conservatively: a missed
    // construct would emit a call to an unchecked body, so when in doubt set.
    // Mutable for const helpers like convert() that also observe them.
    struct ImplicitUses {
        bool stringLiteral = false;
        bool stringSwitch = false;
        bool enumSwitch = false;
        bool unwrap = false;
        bool checkedArithmetic = false;
        bool assertCall = false;
        bool payloadlessEnumCase = false;
    };
    mutable ImplicitUses implicitUses;
    // Types whose infinite-size error was already reported by the early size check.
    llvm::SmallPtrSet<const TypeDecl*, 16> infiniteSizeReported;
    CompileOptions options; // Active package's options; switched per module.
    const std::vector<BuildConfig::ResolvedDependency>* dependencies; // Closure, or null without a project.
};

// Re-checking a default value (field or parameter) resolves names as at its
// declaration: local scopes hidden, member fallbacks stopped. Declared here
// so all default-checking sites share it.
struct DefaultResolveScope {
    SymbolTable::LocalScopeGuard scopes;
    llvm::SaveAndRestore<FunctionDecl*> stop;

    explicit DefaultResolveScope(Typechecker& checker, bool seedThis = true);
};

// Saves move and conditional-move state for one branch. Assignment state is included
// unless the caller snapshots it outside the branch (ternary arms, short-circuit RHS).
struct BranchStateScope {
    llvm::SaveAndRestore<DeclSet> movedDecls;
    llvm::SaveAndRestore<DeclSet> maybeMovedDecls;
    llvm::SaveAndRestore<DeclSet> condWarnedDecls;
    std::optional<llvm::SaveAndRestore<DeclSet>> assignedDecls;

    explicit BranchStateScope(Typechecker& checker, bool saveAssigned = true)
    : movedDecls(checker.movedDecls), maybeMovedDecls(checker.maybeMovedDecls), condWarnedDecls(checker.condWarnedDecls) {
        if (saveAssigned) assignedDecls.emplace(checker.definitelyAssignedDecls);
    }
};

void validateGenericArgCount(size_t genericParamCount, llvm::ArrayRef<GenericArg> genericArgs, llvm::StringRef name, Location location);
// Returns the enum case tested by an `is` expression's right side, or null when it isn't one.
EnumCase* getIsEnumCase(Expr& expr);
bool containsGenericParam(Type type, llvm::StringRef genericParam);
// True when a fixed array flows into a view type (slice or raw pointer): the
// consumer borrows the elements without taking ownership, so the source must
// still die at its own site instead of being flagged as moved.
bool isArrayBorrow(Type source, Type target);
void diagnoseClosureConversion(Type source, Type target, const Expr& expr);
// Suggests an explicit conversion when a value of one numeric type is used where another is expected.
std::string narrowingHint(Type source, Type target);
// Whether the types alone would bind and only the source being a constant blocks forming the borrow.
bool isBorrowOfConstant(const Expr& expr, Type source, Type target);
// The type named after "to constant" in that diagnostic: the pointee when an
// already-formed borrow is propagated, else the source itself.
Type borrowOfConstantSubject(Type source, Type target);
// Whether storing the value would launder a constant: a lazy passthrough
// iterator derived from constant storage, which later reads would traverse
// as mutable. Materialized results (mapped iterators, collected lists) and
// for-in's own lowering are exempt.
bool isStoredConstIterator(const Expr& init, Type type);
// Whether passing the value would launder a constant: a const-derived
// argument bound to a by-value view parameter (string, slice, or
// view-holding value), which hands the callee an alias of frozen storage
// with no const marker of its own. Borrows bind-check instead, pointers
// cannot be formed over constants, and iterator arguments keep their own
// predicate and message.
bool isStoredConstView(const Expr& init, Type type);
// Whether the expression names frozen constant storage (a const binding or something derived from one).
// followCalls also sees through method calls, whose results may alias receiver storage.
bool exprIsConst(const Expr& expr, bool followCalls = false);
// Whether a value of this type may alias other storage: a borrow, pointer, or
// view, or a wrapper implementing Iterator (whose traversal borrows). Plain
// structs and scalars own or copy their contents, so they cannot alias.
bool typeMayAliasStorage(Type type);
// Whether a value of this type may alias other storage, looking through
// struct fields, fixed-array elements, and enum payloads. An owned List
// counts only when its elements do; an interface counts as unknown.
bool typeMayAliasStorageDeep(Type type);
// Whether the type is a standard-library iterator whose yields are always
// fresh copies: it implements Iterator, and every nullary value() declares a
// non-borrow, non-view return (e.g. ByteIterator's uint8). Borrow-yielding
// iterators (filters, maps) may designate source storage.
bool isFreshYieldingIterator(Type type);
// Whether a value of this type is a safe-handle view: a borrow, slice,
// string, or borrow-yielding iterator. Raw pointers are excluded: they are
// the explicit unsafe escape hatch.
bool isSafeViewType(Type type);
// Storage root an alias-capable value derives from, traced through
// borrow-returning projections (calls, member access, named borrows/views,
// casts) to the underlying declaration. Literals and globals are immortal;
// values through raw pointers or unknown shapes are tainted (unknowable).
struct ViewRoot {
    // Every storage the value may designate (e.g. both sides of a
    // `chain(a, b)`): freezing covers all of them, and returning errors on
    // the first owned local or parameter among them.
    llvm::SmallVector<ViewMemberRoot, 2> decls;
    bool immortal = false;
    bool tainted = false;
    bool temporary = false;
    bool traced = false;
    // The temporary lives until scope end: it sits in receiver position of
    // a call whose return type may borrow it (mirroring IRGen's
    // emittingReceiver), so locals may view it but returns still dangle.
    bool extended = false;
    // The terminal is a dereference: it refers through to the target instead
    // of a temporary.
    bool derefTerminal = false;
    Type tempType;
    Type projectedType;
};
// followViews also follows named view-typed locals to their initializers,
// descends through unwraps and borrow dereferences, and stops at calls
// returning fresh owned values (projections below those designate the
// temporary, not the receiver). The pointer path keeps the legacy tracing.
// followVars=false stops at named variables instead of following their
// initializers: for mutation sites, which affect the named object itself
// rather than the storage a view of it designates.
ViewRoot traceViewRoot(const Expr* expr, bool followViews = false, bool followVars = true);
// Whether the declaration is storage a view freezes: an owned local,
// parameter, or field whose reassignment, move, destruction, or mutation may
// invalidate views designating it. Carriers that own nothing (scalars,
// views, view-holding structs) need no freezing, since reassigning or
// destroying them leaves the designated storage untouched.
bool isFreezableViewRoot(Decl* decl);
// Shared cache for may-write queries: one per postProcess drain, so repeated
// and overlapping call graphs analyze once. See const-mutation.cpp.
struct ConstMutationQuery {
    using Key = std::tuple<FunctionDecl*, bool, bool, bool, std::vector<const Decl*>, std::vector<const Decl*>>;
    std::map<Key, bool> cache;
    std::set<Key> inProgress;
};
// Whether a method may mutate its receiver: its body (transitively) writes
// through `this`, hands `this`-derived values to mutable channels, or calls
// methods that do. Read-only methods return false, so calls on constants to
// them stay legal. See const-mutation.cpp.
bool methodMayMutateReceiver(FunctionDecl& method, ConstMutationQuery& query);
// Whether a callee may write through one of its parameters or return it: the
// parameter is a view root (a fresh wrapper, never the caller's own storage),
// so only writes through it count, not the callee mutating its own temp copy.
bool functionMayWriteThroughParam(FunctionDecl& func, const ParamDecl& param, ConstMutationQuery& query);
// Whether a constructor may capture its parameter into the product (or
// beyond): copying constructors return false, so building owned values
// from constant views stays legal. See const-mutation.cpp.
bool constructorMayCaptureParam(ConstructorDecl& ctor, const ParamDecl& param, ConstMutationQuery& query);
// Explains why a type is not Copyable when a use fails because the value was moved.
std::string copyableHint(Type type);
// Whether a type satisfies a ': Copyable' generic constraint. Structural, not name-based.
bool satisfiesCopyable(Type type);
// Rejects a variadic extra that cannot cross to an `extern "C++"` callee by value.
void validateCppVariadicExtra(Type type, const Expr& arg, llvm::StringRef callee);
// Rejects a variadic extra that cannot cross to an `extern "C"` callee by value.
void validateCVariadicExtra(Type type, const Expr& arg);
// Single match (or C-header duplicates) across modules; throws on ambiguity. No scope lookup.
Decl* findDeclInModules(llvm::StringRef name, Location location, llvm::ArrayRef<Module*> modules);

/// Resets synthesized temporary names so a repeated compilation in the same
/// process names them identically.
void resetTypecheckerCounters();

} // namespace cx
