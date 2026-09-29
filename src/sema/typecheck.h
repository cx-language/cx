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

struct VariadicGenericArgs {
    llvm::StringMap<GenericArg> fixedArgs;
    std::vector<llvm::StringMap<GenericArg>> packArgs;
    std::vector<GenericArg> cacheKey;
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

struct Typechecker {
    Typechecker(const CompileOptions& options, const std::vector<BuildConfig::ResolvedDependency>* dependencies = nullptr)
    : currentModule(nullptr), currentSourceFile(nullptr), currentFunction(nullptr), currentStmt(nullptr), currentInitializedFields(nullptr),
      isPostProcessing(false), options(options), dependencies(dependencies) {}
    void typecheckModule(Module& module, const CompileOptions& packageOptions, bool isMainModule);
    void checkUnusedDecls(const Module& mainModule);

    Type typecheckExpr(Expr& expr, bool useIsWriteOnly = false, Type expectedType = Type());
    void typecheckVarDecl(VarDecl& decl);
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
    void warnAboutUnhandledEnumCases(const SwitchStmt& stmt, Type conditionType) const;
    void typecheckForStmt(ForStmt& forStmt);
    void typecheckDoWhileStmt(DoWhileStmt& doWhileStmt);
    void typecheckBreakStmt(BreakStmt& breakStmt);
    void typecheckContinueStmt(ContinueStmt& continueStmt);
    void typecheckType(Type type, AccessLevel userAccessLevel, bool recheckGenericArgs = true, bool allowReference = false);
    Type resolveTypeAliases(Type type, AccessLevel userAccessLevel = AccessLevel::None, bool foldArraySizes = false);
    Type resolveTypeAliases(Type type, AccessLevel userAccessLevel, llvm::SmallPtrSetImpl<const TypeAliasDecl*>& resolving, bool foldArraySizes = false);
    // Folds a deferred array size (or bare size name) using visible named
    // constants. Throws the specific diagnostic when it cannot.
    Type resolveArraySize(Expr& sizeExpr, Type elementType, Location location, Module* homeModule);
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
    /// Finds the user-declared conversion from source to target: an implicit constructor on the target
    /// or an implicit parameterless member on the source. Pure except for range diagnostics (like the
    /// surrounding probe); null when none applies or several do. viableCount (when given) receives
    /// the number of applicable conversions, so error paths can tell "none" from "ambiguous".
    FunctionDecl* findUserConversion(const Expr* expr, Type source, Type target, int* viableCount = nullptr, bool diagnoseOutOfRange = false) const;
    /// Explains a failed conversion when several user-declared conversions apply, empty otherwise.
    std::string ambiguousConversionHint(const Expr* expr, Type source, Type target) const;
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
    std::vector<Decl*> findDecls(llvm::StringRef name, TypeDecl* receiverTypeDecl = nullptr, bool inAllImportedModules = false);
    std::vector<Decl*> findCalleeCandidates(const CallExpr& expr, llvm::StringRef callee);
    Decl* resolveOverload(llvm::ArrayRef<Decl*> decls, CallExpr& expr, llvm::StringRef callee, Type expectedType, bool allowCommutativeRetry = true);
    std::vector<GenericArg> inferGenericArgsFromCallArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call, llvm::ArrayRef<ParamDecl> params,
                                                         bool returnOnError);
    std::optional<VariadicGenericArgs> inferVariadicGenericArgs(llvm::ArrayRef<GenericParamDecl> genericParams, CallExpr& call,
                                                                llvm::ArrayRef<ParamDecl> params, bool returnOnError);
    ArgumentValidation getArgumentValidationResult(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic);
    std::optional<Match> matchArguments(CallExpr& expr, Decl* calleeDecl, llvm::ArrayRef<ParamDecl> params = {});
    void validateAndConvertArguments(CallExpr& expr, const Decl& calleeDecl, llvm::StringRef functionName = "");
    void validateAndConvertArguments(CallExpr& expr, llvm::ArrayRef<ParamDecl> params, bool isVariadic, llvm::StringRef callee = "",
                                     const Decl* calleeDecl = nullptr);
    TypeDecl* getTypeDecl(const BasicType& type);
    EnumCase* getEnumCase(const Expr& expr, Type expectedType = Type(), CallExpr* call = nullptr);
    EnumCase* getExpectedEnumCase(llvm::StringRef name, Type expectedType);
    VarDecl* getStaticConst(const Expr& expr);
    EnumCase* instantiateEnumCase(TypeTemplate& typeTemplate, llvm::StringRef caseName, const MemberExpr& memberExpr, CallExpr* call, Type expectedType);
    void checkReturnPointerToLocal(const Expr* returnValue) const;
    void warnIfUnusedResult(const Expr& expr, Type type) const;
    void checkHasAccess(const Decl& decl, Location location, AccessLevel userAccessLevel);
    bool inSameModule(const Decl& decl, Location location) const;
    Module* findModuleForFile(const char* file) const;
    void maybeCaptureVariable(VariableDecl& variableDecl);
    llvm::ErrorOr<const Module&> importModule(SourceFile* importer, llvm::StringRef moduleName);
    void deferTypechecking(Decl* decl);
    void postProcess();

    // Marks an expression consumed by a move. Always flags the tree (IRGen skips
    // temporary-destructor registration for flagged constructor calls); only records
    // named declarations as moved when trackVars holds, so copies into copyable
    // consumers keep working while their temps are still recognized as consumed.
    void setMoved(Expr* expr, bool isMoved, bool trackVars = true);
    // Moves ownership out of a projection source (member/index base, unwrap operand,
    // binding subject): owned roots are consumed, temporaries are flagged for
    // destructor elision, and borrowed roots are an error (nothing skips for them).
    void propagateMove(Expr* source, bool trackVars, Location location, bool checkLoop = false);
    void checkNotMoved(const Decl& decl, const VarExpr& expr);

    void applyNarrowings(const Expr& condition, bool polarity);
    void intersectNarrowings(const NarrowMap& other);
    void dropNarrowingsForNames(const llvm::StringSet<>& names);
    static VariableDecl* getEnumNarrowableDecl(const VarExpr& varExpr);
    void narrowEnumCaseComparison(const Expr& lhs, const Expr& rhs, BinaryOperator op, bool polarity);
    void narrowEnumSubjectToCase(const Expr* subject, const EnumCase& enumCase);
    // Restores the whole enum type of an expression narrowed to a case payload.
    // Used where the full value is consumed: switch conditions, `is`, and `&`.
    static void unnarrowEnumView(Expr& expr);
    bool genericArgSatisfiesConstraints(const GenericParamDecl& genericParam, GenericArg genericArg);
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
    Stmt** currentStmt; // Double-pointer so it refers to the correct statement after lowering.
    std::vector<Stmt*> currentControlStmts;
    llvm::SmallPtrSet<FieldDecl*, 32>* currentInitializedFields;
    llvm::SmallPtrSet<Decl*, 32> movedDecls;
    // Values moved on only one side of a conditional: using one warns, and its
    // destructor is skipped like a moved value (leaking the live path).
    llvm::SmallPtrSet<Decl*, 32> maybeMovedDecls;
    // Most recent move site per declaration, so branch merges can warn where
    // a value was conditionally moved.
    llvm::DenseMap<Decl*, Location> moveLocations;
    // Marks a declaration moved-from at the given move site.
    void markMoved(Decl* decl, Location location);
    enum class ConditionalMoveSite { IfThen, IfThenNoElse, IfElse, Switch, SwitchExpr };
    // Guards and move-site lookup shared by conditional-move warnings: skips
    // payload bindings and branch-locals, returns the move site if recorded.
    std::optional<Location> locateConditionalMoveWarning(Decl* decl, size_t branchEntryLocalCount, const llvm::DenseMap<Decl*, Location>& locations);
    // Declarations already warned for a ternary arm move. Nested ternaries warn
    // for their own arms; the outer arms would only repeat them, so each move
    // warns once until the value is reassigned.
    llvm::SmallPtrSet<Decl*, 32> ternaryWarnedDecls;
    // Warns that a value is moved in one ternary arm and leaks when the other
    // arm is taken. Each move warns once (see ternaryWarnedDecls).
    void warnTernaryMove(Decl* decl, bool isThenArm, size_t branchEntryLocalCount);
    // Warns that a value is moved on only some paths through a branch, pointing
    // at its move in the given branch's move map. Branch-local values and
    // payload bindings (which borrow) never leak, so only values declared
    // before the branch warn.
    void warnAboutConditionalMove(Decl* decl, ConditionalMoveSite site, size_t branchEntryLocalCount, const llvm::DenseMap<Decl*, Location>& locations);
    // Merges per-path move sets after a switch: moves on every path stay moved,
    // moves on some paths become maybe-moved and warn.
    void mergeConditionalMoves(const std::vector<llvm::SmallPtrSet<Decl*, 32>>& pathMoved, const std::vector<llvm::SmallPtrSet<Decl*, 32>>& pathMaybe,
                               const llvm::SmallPtrSet<Decl*, 32>& entryMoved, ConditionalMoveSite site, size_t branchEntryLocalCount);
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
    // True while checking a placement-`init` argument, which moves out of raw
    // container storage with no owner, so borrow/dereference moves are allowed.
    bool inMoveInit = false;
    // True while checking an explicit `.deinit()` receiver, which destroys in
    // place with nothing copied out, so dereference and implicit place moves
    // are allowed.
    bool inExplicitDeinit = false;
    std::vector<VarDecl*> localVarDecls;
    NarrowMap narrowedTypes;
    llvm::SmallPtrSet<Decl*, 32> definitelyAssignedDecls;
    bool isPostProcessing;
    std::vector<Decl*> declsToTypecheck;
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
// Explains why a type is not Copyable when a use fails because the value was moved.
std::string copyableHint(Type type);
// Whether a type satisfies a ': Copyable' generic constraint. Structural, not name-based.
bool satisfiesCopyable(Type type);
// Rejects a variadic extra that cannot cross to an `extern "C++"` callee by value.
void validateCppVariadicExtra(Type type, const Expr& arg, llvm::StringRef callee);
// Single match (or C-header duplicates) across modules; throws on ambiguity. No scope lookup.
Decl* findDeclInModules(llvm::StringRef name, Location location, llvm::ArrayRef<Module*> modules);

} // namespace cx
