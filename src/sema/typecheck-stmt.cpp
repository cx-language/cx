#include "typecheck.h"
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/arena.h"
#include "../ast/module.h"
#include "../driver/driver.h"

using namespace cx;

// True when no path through the block falls through to the next statement: every path
// returns, calls a never-returning function, or breaks/continues past the analyzed block.
// `break`/`continue` inside a nested loop or switch target that construct instead.
static bool allPathsDiverge(llvm::ArrayRef<Stmt*> block, bool assertsOn, int nestLevel = 0) {
    if (block.empty()) return false;

    switch (block.back()->kind) {
    case StmtKind::ReturnStmt:
        return true;
    case StmtKind::BreakStmt:
    case StmtKind::ContinueStmt:
        return nestLevel == 0;
    case StmtKind::ExprStmt: {
        auto& exprStmt = llvm::cast<ExprStmt>(*block.back());
        auto call = llvm::dyn_cast<CallExpr>(exprStmt.expr);
        if (!call) return false;
        if (call->type && call->type.isNeverType()) return true;
        if (assertsOn && isFalseAssert(*call)) return true;
        return false;
    }
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(*block.back());
        return allPathsDiverge(ifStmt.thenBody, assertsOn, nestLevel) && allPathsDiverge(ifStmt.elseBody, assertsOn, nestLevel);
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(*block.back());
        if (!llvm::all_of(switchStmt.cases, [&](SwitchCase& c) { return allPathsDiverge(c.stmts, assertsOn, nestLevel + 1); })) return false;
        if (switchStmt.defaultStmts.empty()) return switchStmt.coversAllEnumCases;
        return allPathsDiverge(switchStmt.defaultStmts, assertsOn, nestLevel + 1);
    }
    case StmtKind::CompoundStmt:
        return allPathsDiverge(llvm::cast<CompoundStmt>(*block.back()).body, assertsOn, nestLevel);
    default:
        return false;
    }
}

static void collectAssignedNames(const Expr& expr, llvm::StringSet<>& names);
static void collectAssignedNames(const Stmt* stmt, llvm::StringSet<>& names);
static void collectAssignedNames(const Expr* condition, llvm::ArrayRef<Stmt*> body, llvm::StringSet<>& names);

static void collectAssignedNames(const Expr& expr, llvm::StringSet<>& names) {
    switch (expr.kind) {
    case ExprKind::VarExpr:
    case ExprKind::StringLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
    case ExprKind::IntLiteralExpr:
    case ExprKind::FloatLiteralExpr:
    case ExprKind::BoolLiteralExpr:
    case ExprKind::NullLiteralExpr:
    case ExprKind::UndefinedLiteralExpr:
    case ExprKind::SizeofExpr:
        return;
    case ExprKind::ArrayLiteralExpr:
        for (auto& element : llvm::cast<ArrayLiteralExpr>(expr).elements)
            collectAssignedNames(*element, names);
        return;
    case ExprKind::AnonymousStructExpr:
        for (auto& element : llvm::cast<AnonymousStructExpr>(expr).elements)
            collectAssignedNames(*element.value, names);
        return;
    case ExprKind::UnaryExpr:
        collectAssignedNames(llvm::cast<UnaryExpr>(expr).getOperand(), names);
        return;
    case ExprKind::BinaryExpr: {
        auto& binary = llvm::cast<BinaryExpr>(expr);
        if ((binary.op == Token::Assignment || isCompoundAssignmentOperator(binary.op)) && binary.getLHS().isVarExpr()) {
            names.insert(llvm::cast<VarExpr>(binary.getLHS()).identifier);
        }
        collectAssignedNames(binary.getLHS(), names);
        collectAssignedNames(binary.getRHS(), names);
        return;
    }
    case ExprKind::CallExpr: {
        auto& call = llvm::cast<CallExpr>(expr);
        collectAssignedNames(*call.callee, names);
        for (auto& arg : call.args)
            collectAssignedNames(*arg.value, names);
        return;
    }
    case ExprKind::MemberExpr:
        collectAssignedNames(*llvm::cast<MemberExpr>(expr).base, names);
        return;
    case ExprKind::IndexExpr:
        collectAssignedNames(*llvm::cast<IndexExpr>(expr).getBase(), names);
        collectAssignedNames(*llvm::cast<IndexExpr>(expr).getIndex(), names);
        return;
    case ExprKind::IndexAssignmentExpr: {
        auto& indexAssignment = llvm::cast<IndexAssignmentExpr>(expr);
        collectAssignedNames(*indexAssignment.getBase(), names);
        collectAssignedNames(*indexAssignment.getIndex(), names);
        collectAssignedNames(*indexAssignment.getValue(), names);
        return;
    }
    case ExprKind::UnwrapExpr:
        collectAssignedNames(*llvm::cast<UnwrapExpr>(expr).getReceiver(), names);
        return;
    case ExprKind::LambdaExpr: {
        auto* functionDecl = llvm::cast<LambdaExpr>(expr).functionDecl;
        for (auto& param : functionDecl->getParams()) {
            if (param.defaultValue) collectAssignedNames(*param.defaultValue, names);
        }
        if (functionDecl->body) {
            for (auto& stmt : *functionDecl->body)
                collectAssignedNames(stmt, names);
        }
        return;
    }
    case ExprKind::IfExpr: {
        auto& ifExpr = llvm::cast<IfExpr>(expr);
        collectAssignedNames(*ifExpr.condition, names);
        collectAssignedNames(*ifExpr.thenExpr, names);
        collectAssignedNames(*ifExpr.elseExpr, names);
        return;
    }
    case ExprKind::SwitchExpr: {
        auto& switchExpr = llvm::cast<SwitchExpr>(expr);
        collectAssignedNames(*switchExpr.condition, names);
        for (auto& arm : switchExpr.arms) {
            collectAssignedNames(*arm.value, names);
            collectAssignedNames(*arm.expr, names);
        }
        if (auto* defaultExpr = switchExpr.defaultExpr) collectAssignedNames(*defaultExpr, names);
        return;
    }
    case ExprKind::ImplicitCastExpr:
        collectAssignedNames(*llvm::cast<ImplicitCastExpr>(expr).operand, names);
        return;
    case ExprKind::VarDeclExpr:
        if (auto* initializer = llvm::cast<VarDeclExpr>(expr).varDecl->initializer) collectAssignedNames(*initializer, names);
        return;
    }
    llvm_unreachable("all cases handled");
}

static void collectAssignedNames(const Stmt* stmt, llvm::StringSet<>& names) {
    switch (stmt->kind) {
    case StmtKind::ReturnStmt:
        if (auto* value = llvm::cast<ReturnStmt>(stmt)->value) collectAssignedNames(*value, names);
        return;
    case StmtKind::VarStmt:
        for (auto* decl : llvm::cast<VarStmt>(stmt)->decls)
            if (auto* initializer = decl->initializer) collectAssignedNames(*initializer, names);
        return;
    case StmtKind::ExprStmt:
        collectAssignedNames(*llvm::cast<ExprStmt>(stmt)->expr, names);
        return;
    case StmtKind::DeferStmt:
        collectAssignedNames(*llvm::cast<DeferStmt>(stmt)->expr, names);
        return;
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(*stmt);
        collectAssignedNames(*ifStmt.condition, names);
        for (auto& thenStmt : ifStmt.thenBody)
            collectAssignedNames(thenStmt, names);
        for (auto& elseStmt : ifStmt.elseBody)
            collectAssignedNames(elseStmt, names);
        return;
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(*stmt);
        collectAssignedNames(*switchStmt.condition, names);
        for (auto& switchCase : switchStmt.cases) {
            collectAssignedNames(*switchCase.value, names);
            for (auto& caseStmt : switchCase.stmts)
                collectAssignedNames(caseStmt, names);
        }
        for (auto& defaultStmt : switchStmt.defaultStmts)
            collectAssignedNames(defaultStmt, names);
        return;
    }
    case StmtKind::WhileStmt: {
        auto& whileStmt = llvm::cast<WhileStmt>(*stmt);
        collectAssignedNames(whileStmt.condition, whileStmt.body, names);
        return;
    }
    case StmtKind::DoWhileStmt: {
        auto& doWhileStmt = llvm::cast<DoWhileStmt>(*stmt);
        collectAssignedNames(doWhileStmt.condition, doWhileStmt.body, names);
        return;
    }
    case StmtKind::ForStmt: {
        auto& forStmt = llvm::cast<ForStmt>(*stmt);
        if (forStmt.variable) collectAssignedNames(forStmt.variable, names);
        if (forStmt.condition) collectAssignedNames(*forStmt.condition, names);
        for (auto* increment : forStmt.increments)
            collectAssignedNames(*increment, names);
        for (auto& bodyStmt : forStmt.body)
            collectAssignedNames(bodyStmt, names);
        return;
    }
    case StmtKind::ForEachStmt: {
        auto& forEachStmt = llvm::cast<ForEachStmt>(*stmt);
        collectAssignedNames(*forEachStmt.range, names);
        if (auto* initializer = forEachStmt.variable->initializer) collectAssignedNames(*initializer, names);
        for (auto& bodyStmt : forEachStmt.body)
            collectAssignedNames(bodyStmt, names);
        return;
    }
    case StmtKind::BreakStmt:
    case StmtKind::ContinueStmt:
        return;
    case StmtKind::CompoundStmt:
        for (auto& bodyStmt : llvm::cast<CompoundStmt>(*stmt).body)
            collectAssignedNames(bodyStmt, names);
        return;
    }
    llvm_unreachable("all cases handled");
}

static void collectAssignedNames(const Expr* condition, llvm::ArrayRef<Stmt*> body, llvm::StringSet<>& names) {
    if (condition) collectAssignedNames(*condition, names);
    for (const Stmt* stmt : body)
        collectAssignedNames(stmt, names);
}

// Iterator methods (e.g. `value()`) borrow the iterated target, not the iterator
// object, so borrows derived through iterator-typed receivers are not local.
static bool isIteratorType(Type type) {
    if (!type) return false;
    auto* typeDecl = type.removeOptional().removePointer().getDecl();
    return typeDecl && typeDecl->implementsInterface("Iterator");
}

bool cx::isSafeViewType(Type type) {
    if (!type) return false;
    Type inner = type.removeOptional();
    if (inner.isReferenceType() || inner.isSlice() || inner.isString()) return true;
    auto* typeDecl = inner.removePointer().getDecl();
    return typeDecl && !typeDecl->isInterface() && typeDecl->implementsInterface("Iterator") && !isFreshYieldingIterator(type);
}

static ViewRoot traceViewRootImpl(const Expr* expr, bool followViews, bool followVars, llvm::SmallPtrSet<const Decl*, 8>& seenBorrows);

// Raw and array pointers (but not borrows): the unsafe hatch, whose targets
// are unknowable.
static bool isUnsafePointerType(Type type) {
    if (!type) return false;
    Type inner = type.removeOptional();
    return (inner.isPointerType() && !inner.isReferenceType()) || inner.isArrayPointer();
}

// Mirrors IRGen's cannotBorrowReceiver: calls returning these types evaluate
// their receiver as a statement temporary instead of extending it to scope
// end, so views of such a temporary dangle.
static bool viewReturnCannotBorrow(Type type) {
    if (!type) return true;
    if (type.isBuiltinType() && !type.isPointerType()) return true;
    if (type.isOptionalType()) return viewReturnCannotBorrow(type.getWrappedType());
    if (type.isFixedArray()) return viewReturnCannotBorrow(type.getElementType());
    if (type.isAnonymousStructType()) {
        return llvm::all_of(type.getAnonymousStructElements(), [](auto& element) { return viewReturnCannotBorrow(element.type); });
    }
    return false;
}

// Arguments that may carry the call's storage: view-typed values, or values
// bound to a borrow/view parameter (e.g. `id(x)` returns a borrow of `x`).
static void collectViewArgs(const CallExpr& callExpr, llvm::SmallVectorImpl<const Expr*>& out) {
    auto* callee = llvm::dyn_cast_or_null<FunctionDecl>(callExpr.calleeDecl);
    for (size_t i = 0; i < callExpr.args.size(); i++) {
        Expr* arg = callExpr.args[i].value;
        // Diverging arguments (e.g. `abort()`) produce no value and stay out.
        if (!arg || !arg->type || arg->type.isNeverType()) continue;
        if (isSafeViewType(arg->type)) {
            out.push_back(arg);
            continue;
        }
        // An explicit address designates its operand's storage (e.g. the
        // `&list` in `Slice(&list)`), even though its own type is a pointer.
        if (auto* unaryArg = llvm::dyn_cast<UnaryExpr>(arg); unaryArg && unaryArg->op == Token::And) {
            out.push_back(arg);
            continue;
        }
        if (!callee) continue;
        int paramIndex = callExpr.paramIndexForArg(i);
        auto params = callee->getParams();
        if (paramIndex >= 0 && size_t(paramIndex) < params.size() && isSafeViewType(params[paramIndex].type)) out.push_back(arg);
    }
}

ViewRoot cx::traceViewRoot(const Expr* expr, bool followViews, bool followVars) {
    llvm::SmallPtrSet<const Decl*, 8> seenBorrows;
    return traceViewRootImpl(expr, followViews, followVars, seenBorrows);
}

static ViewRoot traceViewRootImpl(const Expr* expr, bool followViews, bool followVars, llvm::SmallPtrSet<const Decl*, 8>& seenBorrows) {
    if (!expr) {
        ViewRoot root;
        root.tainted = true;
        return root;
    }

    ViewRoot root;
    const Expr* operand = expr;

    // Borrow-returning projections borrow their base (e.g. `r.unwrap()` borrows
    // `r`); trace through calls, member access, named borrows, and casts to the
    // root referent. A direct `return callee()` needs no tracing: the callee's
    // own return was already checked, so only traced roots are reported below.
    // Note: operators are CallExprs too; only method calls have receivers.
    while (!root.tainted) {
        while (auto* implicitCastExpr = llvm::dyn_cast<ImplicitCastExpr>(operand)) {
            operand = implicitCastExpr->operand;
        }
        const Expr* next = nullptr;
        // Extra candidates for calls that may designate an argument's storage
        // (e.g. `split(path).dir` designates `path`): view-carrying arguments
        // are traced too, and every root is kept. Bare method calls also keep
        // an implicit `this` root. Receiver stays the primary path.
        llvm::SmallVector<const Expr*, 4> argCandidates;
        const CallExpr* argCall = nullptr;
        // Unary, binary, unwrap, and index-assignment expressions subclass
        // CallExpr, but trace by kind: only calls and indexing descend.
        if (operand->kind == ExprKind::UnaryExpr) {
            if (auto* unaryExpr = llvm::cast<UnaryExpr>(operand); followViews && unaryExpr->op == Token::And) {
                // An explicit address designates its operand's storage.
                next = &unaryExpr->getOperand();
            } else if (followViews && unaryExpr->op == Token::Star) {
                // Dereferencing a borrow reborrows the same storage, while
                // dereferencing a raw pointer refers anywhere (unknowable).
                Type target = unaryExpr->getOperand().type;
                if (!target || !target.removeOptional().isReferenceType()) {
                    root.tainted = true;
                    break;
                }
                next = &unaryExpr->getOperand();
            }
        } else if (operand->kind == ExprKind::UnwrapExpr) {
            if (followViews) {
                // Unwrap results borrow the operand, whose temporaries extend.
                root.extended = true;
                next = llvm::cast<UnwrapExpr>(operand)->getReceiver();
            }
        } else if (operand->kind == ExprKind::BinaryExpr || operand->kind == ExprKind::IndexAssignmentExpr) {
            // Fresh values: the terminal check below reports a temporary.
        } else if (auto* ifExpr = llvm::dyn_cast<IfExpr>(operand)) {
            // Either live branch may produce the value, so every one stays a
            // candidate; the merge below keeps all of their roots. Diverging
            // branches (e.g. `abort()`) produce no value and stay out.
            if (followViews) {
                for (Expr* branch : {ifExpr->thenExpr, ifExpr->elseExpr}) {
                    if (branch && branch->type && !branch->type.isNeverType()) argCandidates.push_back(branch);
                }
            }
        } else if (auto* switchExpr = llvm::dyn_cast<SwitchExpr>(operand)) {
            // Same for every live arm. Payload bindings (e.g. `case Ok value`)
            // resolve to their own declaration rather than the condition's
            // storage, so borrows through them stay silent (a hole).
            if (followViews) {
                for (auto& arm : switchExpr->arms) {
                    if (arm.expr && arm.expr->type && !arm.expr->type.isNeverType()) argCandidates.push_back(arm.expr);
                }
                if (switchExpr->defaultExpr && switchExpr->defaultExpr->type && !switchExpr->defaultExpr->type.isNeverType()) {
                    argCandidates.push_back(switchExpr->defaultExpr);
                }
            }
        } else if (auto* callExpr = llvm::dyn_cast<CallExpr>(operand)) {
            // A call returning a fresh owned value is new storage: projections
            // above it designate the temporary, not the receiver. Indexing
            // always designates the base's storage even though its type reads
            // as a value (typecheckIndexExpr strips the borrow).
            if (followViews && !llvm::isa<IndexExpr>(operand) && callExpr->type && !isSafeViewType(callExpr->type) && !isUnsafePointerType(callExpr->type)) {
                // Members of the product may still be views of an argument
                // (e.g. `split(path).dir`), so view-carrying arguments stay
                // candidates once a projection above selected a member;
                // anything else designates the temporary (e.g. viewing a
                // copying conversion like `StringBuf(v)`).
                if (root.traced) collectViewArgs(*callExpr, argCandidates);
                if (argCandidates.empty()) break;
                argCall = callExpr;
            } else {
                next = callExpr->getReceiver();
                // A call on a type name (e.g. `Result.Ok(...)`) constructs a temporary;
                // don't descend into the type itself.
                if (auto* base = llvm::dyn_cast_or_null<VarExpr>(next)) {
                    if (!base->decl) next = nullptr;
                }
                if (next && isIteratorType(next->type)) {
                    root.tainted = true;
                    break;
                }
                if (next) {
                    // The receiver temporary lives until scope end when the
                    // declared return type may borrow it (IRGen extends those).
                    Type declaredReturn = callExpr->type;
                    if (auto* calleeDecl = llvm::dyn_cast_or_null<FunctionDecl>(callExpr->calleeDecl)) declaredReturn = calleeDecl->getReturnType();
                    root.extended = !viewReturnCannotBorrow(declaredReturn);
                }
                if (followViews && !next) {
                    collectViewArgs(*callExpr, argCandidates);
                    argCall = callExpr;
                }
            }
        } else if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(operand)) {
            next = memberExpr->base;
        } else if (auto* varExpr = llvm::dyn_cast<VarExpr>(operand)) {
            // Unresolved names (e.g. the `Outcome` in `Outcome.Ok`) have no declaration to follow.
            if (auto* varDecl = varExpr->decl ? llvm::dyn_cast<VarDecl>(varExpr->decl) : nullptr) {
                // Borrow aliases designate the referent's storage, so all modes
                // follow them; other views stop here unless following deeply
                // (a method call affects the handle object, not the storage a
                // view of it designates).
                bool follow = varDecl->type.isReferenceType() || (followViews && followVars && isSafeViewType(varDecl->type));
                if (follow && varDecl->initializer && seenBorrows.insert(varDecl).second) next = varDecl->initializer;
            }
        }
        bool implicitThisCall = argCall && argCall->calleeDecl && argCall->calleeDecl->kind == DeclKind::MethodDecl;
        if (!next && (!argCandidates.empty() || implicitThisCall)) {
            ViewRoot merged;
            bool sawTemp = false;
            bool sawOwningTemp = false;
            merged.extended = true;
            for (const Expr* candidate : argCandidates) {
                ViewRoot traced = traceViewRootImpl(candidate, followViews, followVars, seenBorrows);
                for (Decl* decl : traced.decls) {
                    if (!llvm::is_contained(merged.decls, decl)) merged.decls.push_back(decl);
                }
                merged.immortal |= traced.immortal;
                merged.tainted |= traced.tainted;
                merged.temporary |= traced.temporary;
                merged.implicitThis |= traced.implicitThis;
                // Only owning temporaries can dangle; the merged value may
                // designate any of them, so all must be extended to be safe.
                if (traced.temporary) {
                    if (!sawTemp) merged.tempType = traced.tempType;
                    sawTemp = true;
                    if (!traced.tempType || traced.tempType.needsDestruction()) {
                        if (!sawOwningTemp) merged.tempType = traced.tempType;
                        sawOwningTemp = true;
                        merged.extended &= traced.extended;
                    }
                }
            }
            if (!sawOwningTemp) merged.extended = false;
            // A bare method call implicitly receives `this`, so its result may
            // designate the caller's storage even with no explicit receiver.
            if (implicitThisCall) merged.implicitThis = true;
            if (!root.traced) root.projectedType = operand->type;
            merged.traced = true;
            if (root.traced && root.projectedType) merged.projectedType = root.projectedType;
            return merged;
        }
        if (!next) break;
        if (!root.traced) root.projectedType = operand->type;
        operand = next;
        root.traced = true;
        // A raw pointer's target is unknown (it may point anywhere), so a borrow
        // traced through one cannot be attributed to a local; stay silent.
        if (!followViews && operand->type && operand->type.removeOptional().isPointerType() && !operand->type.removeOptional().isReferenceType()) {
            root.tainted = true;
        }
    }

    if (root.tainted) return root;

    if (auto* varExpr = llvm::dyn_cast<VarExpr>(operand)) {
        if (!varExpr->decl) {
            root.tainted = true;
        } else if (followViews && isUnsafePointerType(varExpr->type)) {
            // Raw pointers are the unsafe hatch: values reached through them
            // cannot be attributed to any storage.
            root.tainted = true;
        } else {
            root.decls.push_back(varExpr->decl);
        }
        return root;
    }
    if (operand->isStringLiteralExpr()) {
        root.immortal = true;
        return root;
    }
    switch (operand->kind) {
    case ExprKind::CallExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr:
    case ExprKind::BinaryExpr:
    case ExprKind::UnwrapExpr:
    case ExprKind::ArrayLiteralExpr:
        root.temporary = true;
        root.tempType = operand->type;
        break;
    case ExprKind::UnaryExpr:
        if (llvm::cast<UnaryExpr>(operand)->op == Token::Star) {
            root.derefTerminal = true;
        } else {
            root.temporary = true;
            root.tempType = operand->type;
        }
        break;
    default:
        root.tainted = true;
        break;
    }
    return root;
}

void Typechecker::checkReturnPointerToLocal(const Expr* returnValue) const {
    // Borrow returns are diagnosed by checkReturnBorrowedView instead.
    if (currentFunction->getReturnType().removeOptional().isReferenceType()) return;
    if (auto* unaryExpr = llvm::dyn_cast<UnaryExpr>(returnValue)) {
        if (unaryExpr->op == Token::And) {
            returnValue = &unaryExpr->getOperand();
        }
    }

    ViewRoot root = traceViewRoot(returnValue);
    if (root.tainted) return;

    Type localVariableType;
    // The legacy pointer path never merges argument roots, so at most one
    // declaration comes back.
    if (!root.decls.empty()) {
        Decl* decl = root.decls.front();
        switch (decl->kind) {
        case DeclKind::VarDecl: {
            auto* varDecl = llvm::cast<VarDecl>(decl);
            if (varDecl->getName() == "this") return;
            // The address of a borrow is the referent's address, not the variable slot.
            if (varDecl->parent && varDecl->parent->isFunctionDecl() && !varDecl->type.isReferenceType()) {
                localVariableType = varDecl->type;
            }
            break;
        }
        case DeclKind::ParamDecl: {
            auto paramType = llvm::cast<ParamDecl>(decl)->type;
            // The address of a borrow is the caller's address, not the parameter slot.
            if (!paramType.isReferenceType()) localVariableType = paramType;
            break;
        }

        default:
            break;
        }
    }

    // Through projections the referent has the projection's type, not the root's.
    Type referentType = (root.traced && root.projectedType) ? root.projectedType : localVariableType;
    if (localVariableType && currentFunction->getReturnType().removeOptional().isPointerType()
        && currentFunction->getReturnType().removeOptional().getPointee() == referentType.removeReference()) {
        WARN_RANGE(getExprRangeStart(*returnValue), returnValue->endLocation,
                   "returning pointer to local variable (local variables will not exist after the function returns)");
    }

    // A borrow of an owned temporary (e.g. `makeValue().borrow()`) dangles once
    // the statement ends. Dereferences refer through to the target instead of
    // a temporary, and borrow-typed results reborrow the referent (whose own
    // return was already checked), so neither warns here.
    if (!localVariableType && root.traced && root.temporary && !root.derefTerminal && currentFunction->getReturnType().removeOptional().isPointerType()) {
        WARN_RANGE(getExprRangeStart(*returnValue), returnValue->endLocation,
                   "returning pointer to temporary (temporaries are destroyed at the end of the statement)");
    }
}

void Typechecker::checkReturnBorrowedView(const Expr* returnValue) const {
    Type returnType = currentFunction->getReturnType().removeOptional();
    bool direct = returnType.isReferenceType() || returnType.isSlice() || returnType.isString();
    if (!direct && !isSafeViewType(returnType)) return;

    ViewRoot root = traceViewRoot(returnValue, /*followViews=*/true);

    // Borrow and view parameters (and `this`) designate caller storage, which
    // outlives the call; fields are `this`-rooted the same way. Only owned
    // locals, by-value owned parameters, and temporaries dangle here.
    // (For-loop elements have no initializer to follow, so they stay silent.)
    auto isOwnedLocal = [](Decl* decl) {
        auto* varDecl = llvm::dyn_cast<VarDecl>(decl);
        return varDecl && varDecl->getName() != "this" && !varDecl->isGlobal() && varDecl->parent && varDecl->parent->isFunctionDecl()
            && !varDecl->type.removeOptional().isReferenceType() && !isSafeViewType(varDecl->type) && !isUnsafePointerType(varDecl->type);
    };
    auto isOwnedParam = [](Decl* decl) {
        auto* paramDecl = llvm::dyn_cast<ParamDecl>(decl);
        if (!paramDecl || paramDecl->type.removeOptional().isReferenceType() || isSafeViewType(paramDecl->type) || isUnsafePointerType(paramDecl->type)) {
            return false;
        }
        // A by-value parameter that owns no storage only carries views that
        // designate caller-side storage, which outlives the call; only
        // parameters that own their buffer (or are the buffer) dangle here.
        Type type = paramDecl->type.removeOptional();
        if (type.needsDestruction()) return true;
        auto* typeDecl = type.getDecl();
        bool carrier = (typeDecl && (typeDecl->isStruct() || typeDecl->isUnion() || llvm::isa<EnumDecl>(typeDecl))) || type.isAnonymousStructType();
        return !carrier;
    };
    // Lambdas capture outer values by copy or move, so views of those
    // captures designate closure storage, not the outer variable. (Borrows
    // capture by address instead; escaping lambdas over those stay a hole.)
    auto isOuterCapture = [this](Decl* decl) {
        if (!currentFunction->isLambda()) return false;
        auto* var = llvm::dyn_cast<VariableDecl>(decl);
        return var && var->parent && var->parent != currentFunction;
    };
    for (Decl* decl : root.decls) {
        if (!isOwnedLocal(decl) || isOuterCapture(decl)) continue;
        ERROR_RANGE(getExprRangeStart(*returnValue), returnValue->endLocation,
                    "cannot return '" << currentFunction->getReturnType() << "' derived from local variable '" << decl->getName()
                                      << "' (local variables will not exist after the function returns)");
    }
    for (Decl* decl : root.decls) {
        if (!isOwnedParam(decl) || isOuterCapture(decl)) continue;
        ERROR_RANGE(getExprRangeStart(*returnValue), returnValue->endLocation,
                    "cannot return '" << currentFunction->getReturnType() << "' derived from by-value parameter '" << decl->getName()
                                      << "' (it is destroyed when the function returns)");
    }
    if (root.tainted || root.immortal) return;

    // A direct `return callee()` needs no tracing: the callee's own return was
    // already checked. Anything else designating a temporary dangles.
    bool literal = returnValue->isArrayLiteralExpr() || returnValue->isAnonymousStructExpr();
    if (root.temporary && (root.traced || (direct && literal))) {
        ERROR_RANGE(getExprRangeStart(*returnValue), returnValue->endLocation,
                    "cannot return '" << currentFunction->getReturnType()
                                      << "' derived from a temporary (temporaries are destroyed at the end of the statement)");
    }
}

bool cx::isFreezableViewRoot(Decl* decl) {
    if (auto* varDecl = llvm::dyn_cast_or_null<VarDecl>(decl)) {
        if (varDecl->isGlobal() || !varDecl->parent || !varDecl->parent->isFunctionDecl()) return false;
        Type type = varDecl->type.removeOptional();
        if (varDecl->getName() == "this") return type.getPointee().needsDestruction();
        if (type.isReferenceType() || isSafeViewType(type)) return false;
        return type.needsDestruction();
    }
    if (auto* paramDecl = llvm::dyn_cast_or_null<ParamDecl>(decl)) {
        Type type = paramDecl->type.removeOptional();
        if (type.isReferenceType()) return type.getPointee().needsDestruction();
        if (isSafeViewType(type)) return false;
        return type.needsDestruction();
    }
    return false;
}

Decl* Typechecker::normalizeViewRoot(Decl* decl) {
    if (!decl || decl->kind != DeclKind::FieldDecl || !currentFunction) return decl;
    return tryFindDecl("this", currentFunction->getLocation());
}

static void collectFreezeRoots(Typechecker& checker, ViewRoot& root, Location viewLoc, llvm::SmallVectorImpl<Decl*>& roots) {
    for (Decl* decl : root.decls) {
        Decl* normalized = checker.normalizeViewRoot(decl);
        if (normalized && isFreezableViewRoot(normalized) && !llvm::is_contained(roots, normalized)) roots.push_back(normalized);
    }
    if (root.implicitThis) {
        Decl* thisDecl = checker.tryFindDecl("this", viewLoc);
        if (thisDecl && isFreezableViewRoot(thisDecl) && !llvm::is_contained(roots, thisDecl)) roots.push_back(thisDecl);
    }
}

// Whether a temporary root dangles for a local view: scope-extended
// receiver temporaries and trivial ones (function-lived allocas) are fine,
// and a direct same-typed or moved product transfers into the variable.
static bool viewTempDangles(const Expr* init, const ViewRoot& root, Type viewType) {
    if (!root.temporary || root.extended) return false;
    if (root.tempType && !root.tempType.needsDestruction()) return false;
    if (!root.traced) {
        const Expr* peeled = init;
        while (auto* cast = llvm::dyn_cast<ImplicitCastExpr>(peeled))
            peeled = cast->operand;
        if (peeled->isMovedFrom) return false;
        if (peeled->type && viewType && peeled->type.removeOptional() == viewType.removeOptional()) return false;
    }
    return true;
}

void Typechecker::recordViewLocal(VarDecl& decl) {
    if (decl.isGlobal() || decl.isForLoopElement || !decl.initializer) return;
    if (!isSafeViewType(decl.type)) return;
    ViewRoot root = traceViewRoot(decl.initializer, /*followViews=*/true);
    if (root.tainted) return;
    if (viewTempDangles(decl.initializer, root, decl.type)) {
        ERROR_RANGE(getExprRangeStart(*decl.initializer), decl.initializer->endLocation,
                    "cannot initialize view '" << decl.getName() << "' from a temporary (temporaries are destroyed at the end of the statement)");
    }
    // The view comes alive after its initializer, so the creating call never
    // counts as a mutation of its own root.
    ViewFreezeRecord record{&decl, {}, decl.initializer->endLocation, decl.initializer->endLocation};
    collectFreezeRoots(*this, root, decl.getLocation(), record.roots);
    if (!record.roots.empty()) viewFreezeRecords.push_back(std::move(record));
}

void Typechecker::checkLoopBodyFreezes(Expr& range, size_t mutationStart, size_t candidateStart) {
    ViewRoot root = traceViewRoot(&range, /*followViews=*/true);
    if (root.tainted) return;
    llvm::SmallVector<Decl*, 2> roots;
    collectFreezeRoots(*this, root, range.location, roots);
    if (roots.empty()) return;
    for (size_t i = mutationStart; i < viewRootMutations.size(); i++) {
        auto& mutation = viewRootMutations[i];
        if (!llvm::is_contained(roots, mutation.root)) continue;
        if (mutation.isMove) {
            REPORT_ERROR_RANGE(mutation.loc, mutation.loc, "cannot move '" << mutation.root->getName() << "' while the loop over it is still iterating");
        } else {
            REPORT_ERROR_RANGE(mutation.loc, mutation.loc, "cannot assign to '" << mutation.root->getName() << "' while the loop over it is still iterating");
        }
    }
    for (size_t i = candidateStart; i < viewCallCandidates.size(); i++) {
        auto& candidate = viewCallCandidates[i];
        if (!llvm::is_contained(roots, candidate.root)) continue;
        pendingViewFreezeCallChecks.push_back({candidate.callee, candidate.param, currentFunction, candidate.root, "the loop",
                                               /*isLoop=*/true, candidate.begin, candidate.end, candidate.name});
    }
}

void Typechecker::recordViewUse(Decl* decl, Location loc) {
    for (auto& record : viewFreezeRecords) {
        if (record.view == decl) record.lastUse = loc;
    }
}

void Typechecker::recordRootMutation(Decl* root, Location loc, bool isMove) {
    // Moves in a return statement exit the function, so no later use can
    // observe them (deferred expressions stay a hole).
    if (isMove && inReturnValue) return;
    Decl* normalized = normalizeViewRoot(root);
    // Mutating through a borrow alias mutates the referent's storage.
    if (auto* varDecl = llvm::dyn_cast<VarDecl>(normalized);
        varDecl && varDecl->type.removeOptional().isReferenceType() && varDecl->getName() != "this" && varDecl->initializer) {
        ViewRoot target = traceViewRoot(varDecl->initializer, /*followViews=*/true);
        for (Decl* decl : target.decls)
            recordRootMutation(decl, loc, isMove);
        if (target.implicitThis) {
            if (Decl* thisDecl = tryFindDecl("this", loc)) recordRootMutation(thisDecl, loc, isMove);
        }
        return;
    }
    if (!isFreezableViewRoot(normalized)) return;
    viewRootMutations.push_back({normalized, loc, isMove});
}

void Typechecker::rebindViewLocal(VarDecl& view, Expr& rhs, Location loc) {
    llvm::erase_if(viewFreezeRecords, [&](auto& record) { return record.view == &view; });
    if (!isSafeViewType(view.type)) return;
    ViewRoot root = traceViewRoot(&rhs, /*followViews=*/true);
    if (root.tainted) return;
    if (viewTempDangles(&rhs, root, view.type)) {
        ERROR_RANGE(getExprRangeStart(rhs), rhs.endLocation,
                    "cannot bind view '" << view.getName() << "' to a temporary (temporaries are destroyed at the end of the statement)");
    }
    ViewFreezeRecord record{&view, {}, loc, loc};
    collectFreezeRoots(*this, root, loc, record.roots);
    if (!record.roots.empty()) viewFreezeRecords.push_back(std::move(record));
}

void Typechecker::recordViewCallCandidate(FunctionDecl* callee, const ParamDecl* param, Expr& rootExpr, Location begin, Location end, llvm::StringRef name) {
    if (!callee || !currentFunction) return;
    ViewRoot root = traceViewRoot(&rootExpr, /*followViews=*/true, /*followVars=*/false);
    for (Decl* decl : root.decls) {
        Decl* normalized = normalizeViewRoot(decl);
        if (isFreezableViewRoot(normalized)) viewCallCandidates.push_back({callee, param, normalized, begin, end, name.str()});
    }
    if (root.implicitThis) {
        if (Decl* thisDecl = tryFindDecl("this", begin)) {
            if (isFreezableViewRoot(thisDecl)) viewCallCandidates.push_back({callee, param, thisDecl, begin, end, name.str()});
        }
    }
}

static bool viewLocBefore(Location a, Location b) {
    return a.line < b.line || (a.line == b.line && a.column < b.column);
}

void Typechecker::checkViewFreezes() {
    for (auto& record : viewFreezeRecords) {
        for (auto& mutation : viewRootMutations) {
            if (!llvm::is_contained(record.roots, mutation.root)) continue;
            if (!viewLocBefore(record.viewLoc, mutation.loc) || viewLocBefore(record.lastUse, mutation.loc)) continue;
            if (mutation.isMove) {
                REPORT_ERROR_RANGE(mutation.loc, mutation.loc,
                                   "cannot move '" << mutation.root->getName() << "' while view '" << record.view->getName()
                                                   << "' borrowed from it is still in use");
            } else {
                REPORT_ERROR_RANGE(mutation.loc, mutation.loc,
                                   "cannot assign to '" << mutation.root->getName() << "' while view '" << record.view->getName()
                                                        << "' borrowed from it is still in use");
            }
        }
        for (auto& candidate : viewCallCandidates) {
            if (!llvm::is_contained(record.roots, candidate.root)) continue;
            if (!viewLocBefore(record.viewLoc, candidate.begin) || viewLocBefore(record.lastUse, candidate.begin)) continue;
            pendingViewFreezeCallChecks.push_back({candidate.callee, candidate.param, currentFunction, candidate.root, record.view->getName().str(),
                                                   /*isLoop=*/false, candidate.begin, candidate.end, candidate.name});
        }
    }
}

void Typechecker::checkViewFreezeCalls(ConstMutationQuery& query) {
    std::sort(pendingViewFreezeCallChecks.begin(), pendingViewFreezeCallChecks.end(), [](const ViewFreezeCallCheck& a, const ViewFreezeCallCheck& b) {
        if (a.callee != b.callee) return a.callee < b.callee;
        if (a.begin.line != b.begin.line) return a.begin.line < b.begin.line;
        return a.begin.column < b.begin.column;
    });
    pendingViewFreezeCallChecks.erase(std::unique(pendingViewFreezeCallChecks.begin(), pendingViewFreezeCallChecks.end(),
                                                  [](const ViewFreezeCallCheck& a, const ViewFreezeCallCheck& b) {
                                                      return a.callee == b.callee && a.begin.line == b.begin.line && a.begin.column == b.begin.column;
                                                  }),
                                      pendingViewFreezeCallChecks.end());
    for (auto& check : pendingViewFreezeCallChecks) {
        if (check.caller && check.caller->checkState != Decl::CheckState::Checked) continue;
        if (check.callee->checkState != Decl::CheckState::Checked) continue;
        bool mutates = check.param ? functionMayWriteThroughParam(*check.callee, *check.param, query) : methodMayMutateReceiver(*check.callee, query);
        if (!mutates) continue;
        if (check.param) {
            if (check.isLoop) {
                REPORT_ERROR_RANGE(check.begin, check.end,
                                   "cannot pass '" << check.root->getName() << "' to '" << check.name
                                                   << "' while the loop over it is still iterating (it may write through the argument)");
            } else {
                REPORT_ERROR_RANGE(check.begin, check.end,
                                   "cannot pass '" << check.root->getName() << "' to '" << check.name << "' while view '" << check.viewName
                                                   << "' borrowed from it is still in use (it may write through the argument)");
            }
        } else {
            if (check.isLoop) {
                REPORT_ERROR_RANGE(check.begin, check.end,
                                   "cannot call '" << check.name << "' on '" << check.root->getName()
                                                   << "' while the loop over it is still iterating (it mutates the receiver)");
            } else {
                REPORT_ERROR_RANGE(check.begin, check.end,
                                   "cannot call '" << check.name << "' on '" << check.root->getName() << "' while view '" << check.viewName
                                                   << "' borrowed from it is still in use (it mutates the receiver)");
            }
        }
    }
    pendingViewFreezeCallChecks.clear();
}

void Typechecker::warnIfUnusedResult(const Expr& expr, Type type) const {
    // Anchor on the enclosing function so generic stdlib code instantiated from
    // user code stays exempt; currentModule is the instantiation site there.
    Module* module = currentFunction ? currentFunction->getModule() : currentModule;
    if (module->name == "std") return;
    if (!type || type.isVoid() || type.isNeverType()) return;
    auto* call = llvm::dyn_cast<CallExpr>(&expr);
    auto* ctor = call ? llvm::dyn_cast_or_null<ConstructorDecl>(call->calleeDecl) : nullptr;
    // `init` on another instance reinitializes it, and in constructors bare,
    // `this`, or qualified init delegates to `this`; neither builds a
    // temporary. Only explicit `Type.init(...)` construction falls through.
    auto* currentCtor = llvm::dyn_cast<ConstructorDecl>(currentFunction);
    if (ctor && call->getFunctionName() == "init" && (currentCtor || call->isForeignInit())) return;
    if (!options.warnUnusedResult && !ctor) return;
    // A bare `Type(...)` builds a temporary that dies immediately, in a
    // constructor almost always a mistyped delegation.
    if (ctor && currentCtor && currentCtor->getTypeDecl() == ctor->getTypeDecl()) {
        WARN_RANGE(getExprRangeStart(expr), expr.endLocation, "unused result of type '" << type << "'; use 'init(...)' to delegate to another constructor");
        return;
    }
    WARN_RANGE(getExprRangeStart(expr), expr.endLocation, "unused result of type '" << type << "'");
}

void Typechecker::typecheckReturnStmt(ReturnStmt& stmt) {
    llvm::SaveAndRestore saveInReturnValue(inReturnValue, true);
    Type returnValueType = stmt.value ? typecheckExpr(*stmt.value, false, currentFunction->getReturnType()) : Type::getVoid();

    if (!currentFunction->getReturnType()) {
        ASSERT(currentFunction->isLambda());
        if (returnValueType.isReferenceType()) {
            // A borrow can't be returned: read the value out (copying or moving it) instead of aliasing it.
            stmt.value = makeAST<ImplicitCastExpr>(stmt.value, returnValueType.getPointee(), ImplicitCastExpr::AutoDereference);
            returnValueType = returnValueType.getPointee();
        }
        currentFunction->proto.returnType = returnValueType;
    }

    if (!stmt.value) {
        if (!currentFunction->getReturnType().isVoid()) {
            ERROR_RANGE(stmt.location, getIdentifierEndLocation(stmt.location, "return"),
                        "expected return statement to return a value of type '" << currentFunction->getReturnType() << "'");
        }
        return;
    }

    if (auto converted = convert(stmt.value, currentFunction->getReturnType())) {
        stmt.value = converted;
        if (isStoredConstIterator(*stmt.value, currentFunction->getReturnType())) {
            ERROR_RANGE(getExprRangeStart(*stmt.value), stmt.value->endLocation,
                        "cannot return '" << currentFunction->getReturnType() << "' over a constant (collect with 'toList()' first)");
        } else if (isStoredConstView(*stmt.value, currentFunction->getReturnType())) {
            ERROR_RANGE(getExprRangeStart(*stmt.value), stmt.value->endLocation, "cannot return '" << currentFunction->getReturnType() << "' over a constant");
        }
    } else {
        diagnoseClosureConversion(returnValueType, currentFunction->getReturnType(), *stmt.value);
        Type displayReturn = currentFunction->getReturnType();
        if (isBorrowOfConstant(*stmt.value, returnValueType, currentFunction->getReturnType())) {
            // Binding a borrow is not a type mismatch; say what actually failed.
            ERROR_RANGE(getExprRangeStart(*stmt.value), stmt.value->endLocation,
                        "cannot bind '" << displayReturn << "' to constant '" << borrowOfConstantSubject(returnValueType, currentFunction->getReturnType())
                                        << "' in return value" << narrowingHint(returnValueType, displayReturn)
                                        << ambiguousConversionHint(stmt.value, returnValueType, currentFunction->getReturnType()));
        } else if (isConstBlockedConversion(*stmt.value, returnValueType, currentFunction->getReturnType())) {
            // The conversion exists; only the returned value being a constant blocks it.
            ERROR_RANGE(getExprRangeStart(*stmt.value), stmt.value->endLocation,
                        "cannot convert '" << returnValueType << "' to '" << displayReturn
                                           << "' over a constant in return value (use 'var' instead of 'const')");
        } else {
            ERROR_RANGE(getExprRangeStart(*stmt.value), stmt.value->endLocation,
                        "mismatching return type '" << returnValueType << "', expected '" << displayReturn << "'"
                                                    << narrowingHint(returnValueType, displayReturn)
                                                    << ambiguousConversionHint(stmt.value, returnValueType, currentFunction->getReturnType()));
        }
    }

    checkReturnPointerToLocal(stmt.value);
    checkReturnBorrowedView(stmt.value);
    // Returning a borrow transfers no ownership, so reborrows never track moves.
    bool trackVars = (!stmt.value || !stmt.value->type || !stmt.value->type.removeReference().isImplicitlyCopyable())
                  && !currentFunction->getReturnType().removeOptional().isReferenceType();
    if (stmt.value && stmt.value->type && !isArrayBorrow(stmt.value->type, currentFunction->getReturnType())) {
        setMoved(stmt.value, true, trackVars);
    }
    stmt.movedDecls.clear();
    stmt.movedDecls.insert(stmt.movedDecls.end(), movedDecls.begin(), movedDecls.end());
    stmt.movedDecls.insert(stmt.movedDecls.end(), maybeMovedDecls.begin(), maybeMovedDecls.end());
}

void Typechecker::typecheckVarStmt(VarStmt& stmt) {
    for (auto* decl : stmt.decls) {
        typecheckVarDecl(*decl);
    }
}

std::optional<Location> Typechecker::locateConditionalMoveWarning(Decl* decl, size_t branchEntryLocalCount, const llvm::DenseMap<Decl*, Location>& locations) {
    // Compiler-generated temporaries (`__`-prefixed, lexer-reserved) are
    // un-actionable: lowering-internal moves never warn.
    if (decl->getName().starts_with("__")) return std::nullopt;
    // Payload bindings borrow their subject's storage and run no destructor,
    // so only the subject's leak warns.
    if (bindingSources.count(decl)) return std::nullopt;
    // Values declared inside the branch die there; moving one there is final.
    auto found = std::find(localVarDecls.begin(), localVarDecls.end(), decl);
    if (found != localVarDecls.end() && size_t(found - localVarDecls.begin()) >= branchEntryLocalCount) return std::nullopt;
    auto loc = locations.find(decl);
    if (loc == locations.end()) return std::nullopt;
    return loc->second;
}

void Typechecker::warnAboutConditionalMove(Decl* decl, ConditionalMoveSite site, size_t branchEntryLocalCount,
                                           const llvm::DenseMap<Decl*, Location>& locations) {
    auto loc = locateConditionalMoveWarning(decl, branchEntryLocalCount, locations);
    if (!loc) return;
    auto name = decl->getName();
    switch (site) {
    case ConditionalMoveSite::IfThen:
        WARN(*loc, "value '" << name << "' is moved in the 'then' branch but not the 'else' branch; it may leak when the condition is false (add 'else { drop("
                             << name << "); }' if this was intended)");
        break;
    case ConditionalMoveSite::IfThenNoElse:
        WARN(*loc, "value '" << name
                             << "' is moved in the 'then' branch but there is no 'else' branch; it may leak when the condition is false (add 'else { drop("
                             << name << "); }' if this was intended)");
        break;
    case ConditionalMoveSite::IfElse:
        WARN(*loc, "value '" << name << "' is moved in the 'else' branch but not the 'then' branch; it may leak when the condition is true (add 'drop(" << name
                             << ");' to the 'then' branch if this was intended)");
        break;
    case ConditionalMoveSite::Switch:
        WARN(*loc, "value '" << name << "' is moved in only some arms of this 'switch'; it may leak on the other paths (add 'drop(" << name
                             << ");' to the other arms if this was intended)");
        break;
    case ConditionalMoveSite::SwitchExpr:
        WARN(*loc,
             "value '"
                 << name
                 << "' is moved in only some arms of this 'switch' expression; it may leak on the other paths (move it on every path if this was intended)");
        break;
    case ConditionalMoveSite::ShortCircuitAnd:
        WARN(*loc, "value '" << name
                             << "' is moved in the right-hand side of this '&&' but it may not execute; it may leak when the left side is false "
                                "(restructure with 'if' if this was intended)");
        break;
    case ConditionalMoveSite::ShortCircuitOr:
        WARN(*loc, "value '" << name
                             << "' is moved in the right-hand side of this '||' but it may not execute; it may leak when the left side is true "
                                "(restructure with 'if' if this was intended)");
        break;
    case ConditionalMoveSite::NullCoalescing:
        // Split '??' across literals: ??' is a trigraph.
        WARN(*loc, "value '" << name
                             << "' is moved in the right-hand side of this '??"
                                "' but it may not execute; it may leak when the left side is not null (restructure with 'if' if this was intended)");
        break;
    }
}

// Parameters arrive initialized but never enter definitelyAssignedDecls;
// every path may destroy them.
static bool isAssignedOnPath(Decl* decl, const DeclSet& pathAssigned) {
    return decl->kind == DeclKind::ParamDecl || pathAssigned.count(decl);
}

void Typechecker::typecheckIfStmt(IfStmt& ifStmt) {
    typecheckExpr(*ifStmt.condition);
    typecheckImplicitlyBoolConvertibleExpr(ifStmt.condition);
    currentControlStmts.push_back(&ifStmt);

    // A value moved in every branch is moved after the if statement. Moves from
    // only one branch destroy the value on the other branch and mark it moved,
    // so later uses error. An empty else body moves nothing, so then-only
    // moves always destroy on the else path.
    DeclSet thenMovedDecls, elseMovedDecls;
    DeclSet thenMaybeMovedDecls, elseMaybeMovedDecls;
    llvm::DenseMap<Decl*, Location> thenMoveLocations, elseMoveLocations;
    // Values already moved before the if get no conditional-move warning: the
    // asymmetry comes from reassignment in the other branch, not a new move.
    DeclSet preMovedDecls = movedDecls;
    size_t branchEntryLocalCount = localVarDecls.size();
    NarrowMap outerNarrowings = narrowedTypes;
    NarrowMap thenNarrowings, elseNarrowings;
    DeclSet thenAssignedDecls, elseAssignedDecls;

    {
        Scope scope(currentFunction, &currentModule->symbolTable);
        BranchStateScope branchState(*this);
        applyNarrowings(*ifStmt.condition, true);
        if (ifStmt.isBinding) {
            auto* isExpr = llvm::cast<BinaryExpr>(ifStmt.condition);
            ASSERT(isExpr->op == Token::Is);
            typecheckSwitchCaseBinding(ifStmt.isBinding, getIsEnumCase(isExpr->getRHS()), &isExpr->getLHS());
        }
        for (auto& stmt : ifStmt.thenBody) {
            typecheckStmt(stmt);
        }
        thenMovedDecls = movedDecls;
        thenMaybeMovedDecls = maybeMovedDecls;
        thenMoveLocations = moveLocations;
        thenNarrowings = narrowedTypes;
        thenAssignedDecls = definitelyAssignedDecls;
        narrowedTypes = outerNarrowings;
    }

    {
        Scope scope(currentFunction, &currentModule->symbolTable);
        BranchStateScope branchState(*this);
        applyNarrowings(*ifStmt.condition, false);
        for (auto& stmt : ifStmt.elseBody) {
            typecheckStmt(stmt);
        }
        elseMovedDecls = movedDecls;
        elseMaybeMovedDecls = maybeMovedDecls;
        elseMoveLocations = moveLocations;
        elseNarrowings = narrowedTypes;
        elseAssignedDecls = definitelyAssignedDecls;
        narrowedTypes = outerNarrowings;
    }

    // A move holds after the if only if it holds on every path reaching past it.
    // When one branch diverges (e.g. `if b return;`), the other branch decides.
    bool assertsOn = assertsEnabled(options.mode, currentFunction && currentFunction->isTest);
    bool thenDiverges = allPathsDiverge(ifStmt.thenBody, assertsOn);
    bool elseDiverges = !ifStmt.elseBody.empty() && allPathsDiverge(ifStmt.elseBody, assertsOn);
    if (thenDiverges && !elseDiverges) {
        movedDecls = elseMovedDecls;
        maybeMovedDecls = elseMaybeMovedDecls;
        narrowedTypes = elseNarrowings;
        definitelyAssignedDecls = elseAssignedDecls;
    } else if (elseDiverges && !thenDiverges) {
        movedDecls = thenMovedDecls;
        maybeMovedDecls = thenMaybeMovedDecls;
        narrowedTypes = thenNarrowings;
        definitelyAssignedDecls = thenAssignedDecls;
    } else if (!thenDiverges && !elseDiverges) {
        DeclSet symdiff = mergeConditionalMoves({thenMovedDecls, elseMovedDecls}, {thenMaybeMovedDecls, elseMaybeMovedDecls});
        for (auto* decl : orderMergeDestroys(symdiff)) {
            auto warnRemaining = [&] {
                if (preMovedDecls.count(decl)) return;
                if (thenMovedDecls.count(decl)) {
                    warnAboutConditionalMove(decl, ifStmt.elseLocation.isValid() ? ConditionalMoveSite::IfThen : ConditionalMoveSite::IfThenNoElse,
                                             branchEntryLocalCount, thenMoveLocations);
                } else {
                    warnAboutConditionalMove(decl, ConditionalMoveSite::IfElse, branchEntryLocalCount, elseMoveLocations);
                }
            };
            bool anyPathMaybe = thenMaybeMovedDecls.count(decl) || elseMaybeMovedDecls.count(decl);
            if (!resolveMergeDecl(decl, branchEntryLocalCount, anyPathMaybe, warnRemaining)) continue;
            if (!thenMovedDecls.count(decl) && isAssignedOnPath(decl, thenAssignedDecls)) {
                ifStmt.thenBody.push_back(makeMergeDrop(decl, thenMovedDecls, thenAssignedDecls, ifStmt.condition->location));
            }
            if (!elseMovedDecls.count(decl) && isAssignedOnPath(decl, elseAssignedDecls)) {
                ifStmt.elseBody.push_back(makeMergeDrop(decl, elseMovedDecls, elseAssignedDecls, ifStmt.condition->location));
            }
            if (isAssignedOnPath(decl, thenAssignedDecls) || isAssignedOnPath(decl, elseAssignedDecls)) movedDecls.insert(decl);
        }
        narrowedTypes = thenNarrowings;
        intersectNarrowings(elseNarrowings);
        intersectDefinitelyAssigned({thenAssignedDecls, elseAssignedDecls});
    }

    currentControlStmts.pop_back();
}

// Collects the enum cases handled by the given case values, or nullopt when the condition
// isn't an enum or a case doesn't resolve to one of its cases.
static std::optional<llvm::SmallPtrSet<EnumCase*, 8>> getHandledEnumCases(llvm::ArrayRef<Expr*> caseValues, Type conditionType) {
    if (!conditionType.isEnumType()) return std::nullopt;
    auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
    llvm::SmallPtrSet<EnumCase*, 8> handledCases;
    for (auto* value : caseValues) {
        auto* memberExpr = llvm::dyn_cast<MemberExpr>(value);
        auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
        if (!enumCase || enumCase->getEnumDecl() != enumDecl) return std::nullopt;
        handledCases.insert(enumCase);
    }
    return handledCases;
}

static std::optional<llvm::SmallPtrSet<EnumCase*, 8>> getHandledEnumCases(const SwitchStmt& stmt, Type conditionType) {
    std::vector<Expr*> caseValues;
    for (auto& switchCase : stmt.cases) {
        caseValues.push_back(switchCase.value);
    }
    return getHandledEnumCases(caseValues, conditionType);
}

static bool coversAllEnumCases(const SwitchStmt& stmt, Type conditionType) {
    auto handledCases = getHandledEnumCases(stmt, conditionType);
    if (!handledCases) return false;
    return handledCases->size() == llvm::cast<EnumDecl>(conditionType.getDecl())->cases.size();
}

EnumCase* Typechecker::typecheckSwitchCaseValue(Expr*& value, Type conditionType) {
    if (conditionType.isEnumType()) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(value)) {
            auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
            if (enumDecl->getCaseByName(varExpr->identifier)) {
                // A bare `case B:` mirrors the qualified `case E.B:`, so desugar to the qualified form.
                value = makeAST<MemberExpr>(makeAST<VarExpr>(enumDecl->getName(), varExpr->location), varExpr->identifier, varExpr->location);
                value->endLocation = varExpr->endLocation;
            }
        }
    }

    Type caseType = typecheckExpr(*value, false, conditionType);

    if (auto converted = convert(value, conditionType)) {
        value = converted;
    } else {
        ERROR_RANGE(getExprRangeStart(*value), value->endLocation,
                    "case value type '" << caseType << "' doesn't match switch condition type '" << conditionType << "'");
    }

    auto* memberExpr = llvm::dyn_cast<MemberExpr>(value);
    auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
    if (!enumCase && !value->isConstant()) {
        ERROR_RANGE(getExprRangeStart(*value), value->endLocation, "case value must be constant");
    }
    return enumCase;
}

void Typechecker::typecheckSwitchCaseBinding(VarDecl* associatedValue, EnumCase* enumCase, Expr* subject) {
    if (!associatedValue) return;
    // The parser has no enclosing declaration for bindings in switch expressions; adopt them here.
    associatedValue->parent = currentFunction;
    associatedValue->isImplicitlyBound = true;
    if (!enumCase) {
        ERROR_RANGE(associatedValue->location, getIdentifierEndLocation(associatedValue->location, associatedValue->getName()),
                    "only enum cases can bind associated values");
    }
    if (!enumCase->associatedType) {
        ERROR_RANGE(associatedValue->location, getIdentifierEndLocation(associatedValue->location, associatedValue->getName()),
                    "enum case '" << enumCase->getName() << "' has no associated values to bind");
    }
    Type associatedType = NOTNULL(enumCase->associatedType);
    if (associatedType.isAnonymousStructType() && associatedType.getAnonymousStructElements().size() == 1) {
        associatedType = associatedType.getAnonymousStructElements().front().type;
    }
    if (subject && subjectBorrows(subject)) {
        // The binding aliases borrowed storage (e.g. `switch this`), so moving out of it
        // is forbidden; only borrowing uses are sound. Codegen already passes the pointer.
        associatedType = PointerType::get(associatedType, PointerKind::Reference);
    } else if (subject) {
        bindingSources[associatedValue] = subject;
    }
    associatedValue->type = associatedType;
    typecheckVarDecl(*associatedValue);
    definitelyAssignedDecls.insert(associatedValue);
}

void Typechecker::applyEnumCaseNarrowing(VariableDecl* varDecl, const EnumCase& enumCase) {
    if (varDecl->type.isOptionalType()) {
        if (enumCase.associatedType) {
            narrowedTypes[varDecl] = varDecl->type.getWrappedType();
        } else {
            narrowedTypes.erase(varDecl);
        }
        return;
    }
    narrowedTypes[varDecl] = enumCase.associatedType ? enumCase.associatedType : AnonymousStructType::get({});
}

void Typechecker::narrowEnumSubjectToCase(const Expr* subject, const EnumCase& enumCase) {
    auto* varExpr = llvm::dyn_cast<VarExpr>(subject);
    if (!varExpr) return;
    auto* varDecl = getEnumNarrowableDecl(*varExpr);
    if (!varDecl) return;
    if (enumCase.getEnumDecl() != llvm::cast<EnumDecl>(varDecl->type.getDecl())) return;
    applyEnumCaseNarrowing(varDecl, enumCase);
}

bool Typechecker::subjectBorrows(Expr* subject) {
    // Borrow subjects read through: an implicit dereference cast (e.g. `switch this`)
    // or an explicit dereference (e.g. `switch *p`). Both borrow the original storage.
    if (auto* cast = llvm::dyn_cast<ImplicitCastExpr>(subject)) {
        return cast->castKind == ImplicitCastExpr::AutoDereference;
    }
    if (auto* unary = llvm::dyn_cast<UnaryExpr>(subject)) {
        return unary->op == Token::Star;
    }
    return subject->type.isReferenceType();
}

// Switch expressions lower directly to a switch instruction, so unlike switch statements
// they accept neither string conditions nor null cases.
Type Typechecker::typecheckSwitchCondition(Expr*& condition) {
    Type conditionType = typecheckExpr(*condition);
    // A subject narrowed to a case payload isn't switchable; restore the whole enum.
    // (Optional narrowings are kept: switching on the unwrapped value is intended there.)
    unnarrowEnumView(*condition);
    conditionType = condition->type;

    if (conditionType.isReferenceType()) {
        // Borrows read through implicitly (e.g. `switch this`, where receivers are borrows).
        // Switching only reads the value, so dereferencing is allowed even for non-copyable pointees.
        condition = makeAST<ImplicitCastExpr>(condition, conditionType.getPointee(), ImplicitCastExpr::AutoDereference);
        conditionType = conditionType.getPointee();
    }

    if ((conditionType.removeOptional().isPointerType() && !conditionType.removeOptional().isReferenceType())
        || conditionType.removeOptional().isArrayPointer()) {
        ERROR_RANGE(getExprRangeStart(*condition), condition->endLocation,
                    "switch condition must have integer, char, or enum type, got '" << conditionType << "'; dereference it explicitly (e.g. 'switch (*p)')");
    }

    // Pointer-implemented optionals have no tag to switch on.
    bool isSwitchableEnum = conditionType.isEnumType() && !(conditionType.isOptionalType() && conditionType.isImplementedAsPointer());
    if (!conditionType.isInteger() && !conditionType.isChar() && !isSwitchableEnum) {
        ERROR_RANGE(getExprRangeStart(*condition), condition->endLocation,
                    "switch condition must have integer, char, or enum type, got '" << conditionType << "'");
    }
    if (isSwitchableEnum) implicitUses.enumSwitch = true;
    return conditionType;
}

// Finds the statement list containing the target (for inserting drops before
// a switch break). A break captured for this switch nests only under ifs and
// compounds: anything under a loop or inner switch targets that instead.
// Statement lowering preserves statement pointers, so captured break
// pointers stay valid.
static bool findStmtSlot(AstVector<Stmt*>& stmts, const Stmt* target, AstVector<Stmt*>*& outVec, size_t& outIndex) {
    for (size_t i = 0; i < stmts.size(); ++i) {
        if (stmts[i] == target) {
            outVec = &stmts;
            outIndex = i;
            return true;
        }
        auto* stmt = stmts[i];
        switch (stmt->kind) {
        case StmtKind::IfStmt: {
            auto& ifStmt = *llvm::cast<IfStmt>(stmt);
            if (findStmtSlot(ifStmt.thenBody, target, outVec, outIndex) || findStmtSlot(ifStmt.elseBody, target, outVec, outIndex)) return true;
            break;
        }
        case StmtKind::CompoundStmt:
            if (findStmtSlot(llvm::cast<CompoundStmt>(stmt)->body, target, outVec, outIndex)) return true;
            break;
        default:
            break;
        }
    }
    return false;
}

DeclSet Typechecker::mergeConditionalMoves(const std::vector<DeclSet>& pathMoved, const std::vector<DeclSet>& pathMaybe) {
    DeclSet mergedMovedDecls = pathMoved.front();
    DeclSet unionMovedDecls;
    maybeMovedDecls.clear();
    for (auto& path : pathMoved) {
        for (auto* decl : llvm::to_vector(mergedMovedDecls)) {
            if (!path.count(decl)) mergedMovedDecls.erase(decl);
        }
        unionMovedDecls.insert(path.begin(), path.end());
    }
    for (auto& path : pathMaybe) {
        maybeMovedDecls.insert(path.begin(), path.end());
    }
    DeclSet symdiff;
    for (auto* decl : unionMovedDecls) {
        if (!mergedMovedDecls.count(decl)) symdiff.insert(decl);
    }
    movedDecls = std::move(mergedMovedDecls);
    return symdiff;
}

void Typechecker::mergeExpressionMoves(const std::vector<DeclSet>& pathMoved, const std::vector<DeclSet>& pathMaybe, ConditionalMoveSite site,
                                       size_t branchEntryLocalCount) {
    // Branch checking restores the entry state, so it is still current here.
    DeclSet entryMoved = movedDecls;
    DeclSet symdiff = mergeConditionalMoves(pathMoved, pathMaybe);
    for (auto* decl : symdiff) {
        maybeMovedDecls.insert(decl);
        if (entryMoved.count(decl)) continue;
        // Locate before inserting: never-warn declarations (bindings, temps,
        // locals) must not consume the dedup slot. Each move warns once (like
        // warnTernaryMove) so a later propagation of the same move stays silent.
        if (!locateConditionalMoveWarning(decl, branchEntryLocalCount, moveLocations)) continue;
        if (!condWarnedDecls.insert(decl).second) continue;
        warnAboutConditionalMove(decl, site, branchEntryLocalCount, moveLocations);
    }
}

Type Typechecker::typecheckShortCircuitRHS(llvm::function_ref<Type()> checkRHS, ConditionalMoveSite site) {
    size_t branchEntryLocalCount = localVarDecls.size();
    Type rightType;
    DeclSet rhsMovedDecls, rhsMaybeMovedDecls;
    {
        BranchStateScope branchState(*this, false);
        rightType = checkRHS();
        rhsMovedDecls = movedDecls;
        rhsMaybeMovedDecls = maybeMovedDecls;
    }
    mergeExpressionMoves({movedDecls, rhsMovedDecls}, {maybeMovedDecls, rhsMaybeMovedDecls}, site, branchEntryLocalCount);
    return rightType;
}

Stmt* Typechecker::makeMergeDrop(Decl* decl, const DeclSet& pathMoved, const DeclSet& pathAssigned, Location location) {
    llvm::SaveAndRestore saveMovedDecls(movedDecls, pathMoved);
    llvm::SaveAndRestore saveMaybeMovedDecls(maybeMovedDecls, DeclSet());
    llvm::SaveAndRestore saveMoveLocations(moveLocations);
    llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls, pathAssigned);
    auto* call = makeAST<CallExpr>(makeAST<VarExpr>("drop", location), AstVector<NamedValue>{NamedValue(makeAST<VarExpr>(decl->getName(), location))},
                                   AstVector<GenericArg>(), location);
    // Pin std's drop: user overloads must not hijack compiler-inserted
    // destruction. A template-valued calleeDecl skips lookup below.
    if (auto* stdModule = Module::getStdlibModule()) {
        if (Decl* stdDrop = stdModule->symbolTable.findOne("drop")) call->calleeDecl = stdDrop;
    }
    Stmt* stmt = makeAST<ExprStmt>(call);
    typecheckStmt(stmt);
    return stmt;
}

bool Typechecker::resolveMergeDecl(Decl* decl, size_t branchEntryLocalCount, bool anyPathMaybe, llvm::function_ref<void()> warn) {
    auto* variableDecl = anyPathMaybe ? nullptr : llvm::dyn_cast<VariableDecl>(decl);
    if (!variableDecl) {
        maybeMovedDecls.insert(decl);
        warn();
        return false;
    }
    if (bindingSources.count(decl)) {
        movedDecls.insert(decl);
        return false;
    }
    bool isPreBranch = variableDecl->kind == DeclKind::ParamDecl;
    if (!isPreBranch) {
        auto found = std::find(localVarDecls.begin(), localVarDecls.end(), decl);
        isPreBranch = found != localVarDecls.end() && size_t(found - localVarDecls.begin()) < branchEntryLocalCount;
    }
    if (!isPreBranch || !variableDecl->type.needsDestruction()) {
        movedDecls.insert(decl);
        return false;
    }
    return true;
}

std::vector<Decl*> Typechecker::orderMergeDestroys(const DeclSet& symdiff) {
    std::vector<Decl*> ordered;
    for (auto it = localVarDecls.rbegin(); it != localVarDecls.rend(); ++it) {
        if (symdiff.count(*it)) ordered.push_back(*it);
    }
    if (currentFunction) {
        auto params = currentFunction->getParams();
        for (auto it = params.rbegin(); it != params.rend(); ++it) {
            if (symdiff.count(&*it)) ordered.push_back(const_cast<ParamDecl*>(&*it));
        }
    }
    std::vector<Decl*> leftovers;
    for (auto* decl : symdiff) {
        if (!llvm::is_contained(ordered, decl)) leftovers.push_back(decl);
    }
    llvm::sort(leftovers, [](const Decl* a, const Decl* b) {
        auto locA = a->getLocation(), locB = b->getLocation();
        return std::tie(locA.line, locA.column) < std::tie(locB.line, locB.column);
    });
    ordered.insert(ordered.end(), leftovers.begin(), leftovers.end());
    return ordered;
}

static void recordBranchEnd(std::vector<DeclSet>& assigned, std::vector<DeclSet>& moved, std::vector<DeclSet>& maybeMoved, const DeclSet& pathAssigned,
                            const DeclSet& pathMoved, const DeclSet& pathMaybeMoved) {
    assigned.push_back(pathAssigned);
    moved.push_back(pathMoved);
    maybeMoved.push_back(pathMaybeMoved);
}

void Typechecker::intersectDefinitelyAssigned(llvm::ArrayRef<DeclSet> paths) {
    definitelyAssignedDecls = paths.front();
    for (auto& path : paths.drop_front()) {
        for (auto* decl : llvm::to_vector(definitelyAssignedDecls)) {
            if (!path.count(decl)) definitelyAssignedDecls.erase(decl);
        }
    }
}

void Typechecker::typecheckSwitchStmt(SwitchStmt& stmt) {
    Type conditionType = typecheckExpr(*stmt.condition);
    // A subject narrowed to a case payload isn't switchable; restore the whole enum.
    // (Optional narrowings are kept: switching on the unwrapped value is intended there.)
    unnarrowEnumView(*stmt.condition);
    conditionType = stmt.condition->type;

    if (conditionType.isReferenceType()) {
        // Borrows read through implicitly (e.g. `switch this`, where receivers are borrows).
        // Switching only reads the value, so dereferencing is allowed even for non-copyable pointees.
        stmt.condition = makeAST<ImplicitCastExpr>(stmt.condition, conditionType.getPointee(), ImplicitCastExpr::AutoDereference);
        conditionType = conditionType.getPointee();
    }

    if ((conditionType.removeOptional().isPointerType() && !conditionType.removeOptional().isReferenceType())
        || conditionType.removeOptional().isArrayPointer()) {
        ERROR_RANGE(getExprRangeStart(*stmt.condition), stmt.condition->endLocation,
                    "switch condition must have integer, char, string, or enum type, got '" << conditionType
                                                                                            << "'; dereference it explicitly (e.g. 'switch (*p)')");
    }

    // Pointer-implemented optionals have no tag to switch on.
    bool isSwitchableEnum = conditionType.isEnumType() && !(conditionType.isOptionalType() && conditionType.isImplementedAsPointer());
    bool isString = conditionType.isString();
    if (!conditionType.isInteger() && !conditionType.isChar() && !isSwitchableEnum && !isString) {
        ERROR_RANGE(getExprRangeStart(*stmt.condition), stmt.condition->endLocation,
                    "switch condition must have integer, char, string, or enum type, got '" << conditionType << "'");
    }
    if (isString) implicitUses.stringSwitch = true;
    if (isSwitchableEnum) implicitUses.enumSwitch = true;

    // Case values run before every case body, so variables assigned in any value
    // are un-narrowed for the bodies. Bodies may or may not run, so variables
    // assigned in any body are un-narrowed after the switch.
    llvm::StringSet<> valueAssignedNames;
    for (auto& switchCase : stmt.cases)
        collectAssignedNames(*switchCase.value, valueAssignedNames);
    dropNarrowingsForNames(valueAssignedNames);
    llvm::StringSet<> bodyAssignedNames;
    for (auto& switchCase : stmt.cases) {
        for (auto& caseStmt : switchCase.stmts)
            collectAssignedNames(caseStmt, bodyAssignedNames);
    }
    for (auto& defaultStmt : stmt.defaultStmts)
        collectAssignedNames(defaultStmt, bodyAssignedNames);

    currentControlStmts.push_back(&stmt);
    llvm::SaveAndRestore saveBreakPaths(switchBreakPaths, std::vector<SwitchBreakPath>());

    std::vector<DeclSet> bodyAssignedDecls;
    // Arms run independently (there is no fallthrough), so each is checked
    // from the entry state and merged like an if branch below.
    std::vector<DeclSet> pathMovedDecls, pathMaybeMovedDecls;
    DeclSet entryMovedDecls = movedDecls;
    DeclSet entryMaybeMovedDecls = maybeMovedDecls;
    DeclSet entryAssignedDecls = definitelyAssignedDecls;
    // Where each path's merge drops go, parallel to pathMovedDecls: an arm or
    // default body to append to, or a break to insert before.
    struct MergePathTarget {
        AstVector<Stmt*>* body = nullptr;
        BreakStmt* breakStmt = nullptr;
    };
    std::vector<MergePathTarget> pathTargets;
    size_t branchEntryLocalCount = localVarDecls.size();
    bool assertsOn = assertsEnabled(options.mode, currentFunction && currentFunction->isTest);

    for (auto& switchCase : stmt.cases) {
        if (conditionType.isEnumType()) {
            if (auto* varExpr = llvm::dyn_cast<VarExpr>(switchCase.value)) {
                auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
                if (enumDecl->getCaseByName(varExpr->identifier)) {
                    // A bare `case B:` mirrors the qualified `case E.B:`, so desugar to the qualified form.
                    switchCase.value = makeAST<MemberExpr>(makeAST<VarExpr>(enumDecl->getName(), varExpr->location), varExpr->identifier, varExpr->location);
                    switchCase.value->endLocation = varExpr->endLocation;
                }
            }
        }

        Type caseType = typecheckExpr(*switchCase.value, false, conditionType);

        auto* memberExpr = llvm::dyn_cast<MemberExpr>(switchCase.value);
        auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;

        if (switchCase.value->isNullLiteralExpr()) {
            ERROR_RANGE(getExprRangeStart(*switchCase.value), switchCase.value->endLocation,
                        "case value type 'null' doesn't match switch condition type '" << conditionType << "'");
        } else if (auto converted = convert(switchCase.value, conditionType)) {
            switchCase.value = converted;
            // Conversions can wrap the value, hiding the enum case from the checks below.
            memberExpr = llvm::dyn_cast<MemberExpr>(switchCase.value);
            enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
        } else {
            ERROR_RANGE(getExprRangeStart(*switchCase.value), switchCase.value->endLocation,
                        "case value type '" << caseType << "' doesn't match switch condition type '" << conditionType << "'");
        }

        if (conditionType.isOptionalType() && !conditionType.getWrappedType().isPointerType() && !enumCase && !(caseType == conditionType)) {
            // Value-optional conditions (e.g. int?) only match enum cases (Some/None); a wrapped
            // value has no case representation, so don't silently wrap to the optional type.
            ERROR_RANGE(getExprRangeStart(*switchCase.value), switchCase.value->endLocation,
                        "case value type '" << caseType << "' doesn't match switch condition type '" << conditionType << "'");
        }

        if (!enumCase && !switchCase.value->isConstant()) {
            ERROR_RANGE(getExprRangeStart(*switchCase.value), switchCase.value->endLocation, "case value must be constant");
        }

        Scope scope(nullptr, &currentModule->symbolTable);
        NarrowMap outerNarrowings = narrowedTypes;
        BranchStateScope branchState(*this);

        typecheckSwitchCaseBinding(switchCase.associatedValue, enumCase, stmt.condition);
        if (!switchCase.associatedValue && enumCase) {
            narrowEnumSubjectToCase(stmt.condition, *enumCase);
        }

        for (auto& caseStmt : switchCase.stmts) {
            typecheckStmt(caseStmt);
        }
        narrowedTypes = outerNarrowings;
        // Arms reaching their end contribute their end state; arms exiting
        // only via break contribute just their captured break paths below
        // (a break reaches past the switch, unlike return/continue).
        if (!allPathsDiverge(switchCase.stmts, assertsOn)) {
            recordBranchEnd(bodyAssignedDecls, pathMovedDecls, pathMaybeMovedDecls, definitelyAssignedDecls, movedDecls, maybeMovedDecls);
            pathTargets.push_back({&switchCase.stmts});
        }
    }

    {
        Scope scope(nullptr, &currentModule->symbolTable);
        NarrowMap outerNarrowings = narrowedTypes;
        BranchStateScope branchState(*this);
        for (auto& defaultStmt : stmt.defaultStmts) {
            typecheckStmt(defaultStmt);
        }
        narrowedTypes = outerNarrowings;
        if (!stmt.defaultStmts.empty() && !allPathsDiverge(stmt.defaultStmts, assertsOn)) {
            recordBranchEnd(bodyAssignedDecls, pathMovedDecls, pathMaybeMovedDecls, definitelyAssignedDecls, movedDecls, maybeMovedDecls);
            pathTargets.push_back({&stmt.defaultStmts});
        }
    }

    dropNarrowingsForNames(bodyAssignedNames);

    currentControlStmts.pop_back();

    stmt.coversAllEnumCases = coversAllEnumCases(stmt, conditionType);
    // A move holds after the switch only if it holds on every path reaching
    // past it. Values matching no arm take an implicit empty path.
    if (stmt.defaultStmts.empty() && !stmt.coversAllEnumCases) {
        recordBranchEnd(bodyAssignedDecls, pathMovedDecls, pathMaybeMovedDecls, entryAssignedDecls, entryMovedDecls, entryMaybeMovedDecls);
        pathTargets.push_back({&stmt.defaultStmts});
    }
    for (auto& path : switchBreakPaths) {
        recordBranchEnd(bodyAssignedDecls, pathMovedDecls, pathMaybeMovedDecls, path.assigned, path.moved, path.maybeMoved);
        pathTargets.push_back({nullptr, path.breakStmt});
    }
    // The merge below appends drops for the implicit path to defaultStmts;
    // readers below mean the user-written default.
    bool hadDefault = !stmt.defaultStmts.empty();
    if (!pathMovedDecls.empty()) {
        DeclSet symdiff = mergeConditionalMoves(pathMovedDecls, pathMaybeMovedDecls);
        for (auto* decl : orderMergeDestroys(symdiff)) {
            auto warnRemaining = [&] {
                if (!entryMovedDecls.count(decl)) {
                    warnAboutConditionalMove(decl, ConditionalMoveSite::Switch, branchEntryLocalCount, moveLocations);
                }
            };
            bool anyPathMaybe = llvm::any_of(pathMaybeMovedDecls, [&](auto& path) { return path.count(decl); });
            if (!resolveMergeDecl(decl, branchEntryLocalCount, anyPathMaybe, warnRemaining)) continue;
            for (size_t i = 0; i < pathMovedDecls.size(); ++i) {
                if (pathMovedDecls[i].count(decl) || !isAssignedOnPath(decl, bodyAssignedDecls[i])) continue;
                Stmt* drop = makeMergeDrop(decl, pathMovedDecls[i], bodyAssignedDecls[i], stmt.condition->location);
                auto& target = pathTargets[i];
                if (target.body) {
                    target.body->push_back(drop);
                } else {
                    AstVector<Stmt*>* slot = nullptr;
                    size_t index = 0;
                    for (auto& switchCase : stmt.cases) {
                        if (findStmtSlot(switchCase.stmts, target.breakStmt, slot, index)) break;
                    }
                    if (!slot) findStmtSlot(stmt.defaultStmts, target.breakStmt, slot, index);
                    ASSERT(slot);
                    slot->insert(slot->begin() + index, drop);
                }
            }
            if (llvm::any_of(bodyAssignedDecls, [&](auto& path) { return isAssignedOnPath(decl, path); })) movedDecls.insert(decl);
        }
    }
    if ((hadDefault || stmt.coversAllEnumCases) && !bodyAssignedDecls.empty()) {
        intersectDefinitelyAssigned(bodyAssignedDecls);
    }
    warnAboutUnhandledEnumCases(stmt, conditionType, hadDefault);
}

Type Typechecker::typecheckSwitchExpr(SwitchExpr& expr, Type expectedType) {
    Type conditionType = typecheckSwitchCondition(expr.condition);

    // As in switch statements, variables assigned in any value or arm are un-narrowed,
    // since values run before every arm and arms may or may not run.
    llvm::StringSet<> assignedNames;
    for (auto& arm : expr.arms) {
        collectAssignedNames(*arm.value, assignedNames);
        collectAssignedNames(*arm.expr, assignedNames);
    }
    if (expr.defaultExpr) collectAssignedNames(*expr.defaultExpr, assignedNames);
    dropNarrowingsForNames(assignedNames);

    std::vector<DeclSet> armAssignedDecls;
    std::vector<DeclSet> armMovedDecls, armMaybeMovedDecls;
    DeclSet armWarnedDecls;
    size_t branchEntryLocalCount = localVarDecls.size();

    for (auto& arm : expr.arms) {
        auto* enumCase = typecheckSwitchCaseValue(arm.value, conditionType);

        Scope scope(nullptr, &currentModule->symbolTable);
        NarrowMap outerNarrowings = narrowedTypes;
        BranchStateScope branchState(*this);

        typecheckSwitchCaseBinding(arm.associatedValue, enumCase, expr.condition);
        if (!arm.associatedValue && enumCase) {
            narrowEnumSubjectToCase(expr.condition, *enumCase);
        }
        typecheckExpr(*arm.expr, false, expectedType);
        narrowedTypes = outerNarrowings;
        if (!arm.expr->type.isNeverType()) {
            armWarnedDecls.insert(condWarnedDecls.begin(), condWarnedDecls.end());
            recordBranchEnd(armAssignedDecls, armMovedDecls, armMaybeMovedDecls, definitelyAssignedDecls, movedDecls, maybeMovedDecls);
        }
    }

    if (expr.defaultExpr) {
        NarrowMap outerNarrowings = narrowedTypes;
        BranchStateScope branchState(*this);
        typecheckExpr(*expr.defaultExpr, false, expectedType);
        narrowedTypes = outerNarrowings;
        if (!expr.defaultExpr->type.isNeverType()) {
            armWarnedDecls.insert(condWarnedDecls.begin(), condWarnedDecls.end());
            recordBranchEnd(armAssignedDecls, armMovedDecls, armMaybeMovedDecls, definitelyAssignedDecls, movedDecls, maybeMovedDecls);
        }
    }

    // Arm-local warnings merge back so nested moves warn once; without this
    // the merge below would repeat a nested warning at the same site.
    condWarnedDecls.insert(armWarnedDecls.begin(), armWarnedDecls.end());

    if (!armAssignedDecls.empty()) {
        intersectDefinitelyAssigned(armAssignedDecls);
    }

    // Every valid switch expression covers all paths (a default or all enum
    // cases), so there is no implicit path; on error below the merge is moot.
    // Arms are expressions, so unlike statements there is nowhere to destroy
    // live paths: partial moves keep the maybe state and warn.
    if (!armMovedDecls.empty()) {
        mergeExpressionMoves(armMovedDecls, armMaybeMovedDecls, ConditionalMoveSite::SwitchExpr, branchEntryLocalCount);
    }

    if (!expr.defaultExpr) {
        if (!conditionType.isEnumType()) {
            ERROR_RANGE(getExprRangeStart(expr), expr.endLocation, "switch expression on '" << conditionType << "' must have a default case");
        }
        std::vector<Expr*> caseValues;
        for (auto& arm : expr.arms) {
            caseValues.push_back(arm.value);
        }
        auto handledCases = getHandledEnumCases(caseValues, conditionType);
        auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
        if (!handledCases || handledCases->size() != enumDecl->cases.size()) {
            std::string missing;
            for (auto& enumCase : enumDecl->cases) {
                if (!handledCases || !handledCases->contains(&enumCase)) {
                    if (!missing.empty()) missing += ", ";
                    missing += enumCase.getName().str();
                }
            }
            ERROR_RANGE(getExprRangeStart(expr), expr.endLocation,
                        "switch expression must handle all cases of enum '" << enumDecl->getName() << "' (missing: " << missing << ")");
        }
    }

    std::vector<Expr**> armExprs;
    for (auto& arm : expr.arms) {
        armExprs.push_back(&arm.expr);
    }
    if (expr.defaultExpr) armExprs.push_back(&expr.defaultExpr);

    // Never arms diverge, so they contribute no value to the join.
    // Null joins upward to the optional of the other side; unwrapping it would be nonsense.
    Type resultType;
    std::vector<Expr**> joinedArms;
    for (Expr** armExpr : armExprs) {
        Type armType = (*armExpr)->type;
        if (armType.isNeverType()) {
            if (!resultType) resultType = armType;
            continue;
        }
        bool armIsNull = armType.isNull() || (*armExpr)->isNullLiteralExpr();
        if (!resultType || resultType.isNeverType()) {
            resultType = armType;
        } else if (!armType.isVoid() && !resultType.isVoid() && (armIsNull != resultType.isNull())) {
            // Null is a value, so a borrow joining with null produces an owned optional, not an optional borrow.
            Type other = (armIsNull ? resultType : armType).removeReference();
            Type target = other.isOptionalType() ? other : OptionalType::get(other);
            if (auto convertedArm = convert(*armExpr, target)) {
                *armExpr = convertedArm;
                bool upgraded = true;
                for (Expr** joined : joinedArms) {
                    if (auto upgradedArm = convert(*joined, target)) {
                        *joined = upgradedArm;
                    } else {
                        upgraded = false;
                        break;
                    }
                }
                if (upgraded) {
                    resultType = target;
                    joinedArms.push_back(armExpr);
                    continue;
                }
            }
            ERROR_RANGE(getExprRangeStart(**armExpr), (*armExpr)->endLocation, "incompatible arm types ('" << resultType << "' and '" << armType << "')");
        } else if (auto converted = convert(*armExpr, resultType)) {
            *armExpr = converted;
        } else {
            for (Expr** joined : joinedArms) {
                if (auto upgraded = convert(*joined, armType)) {
                    *joined = upgraded;
                } else {
                    ERROR_RANGE(getExprRangeStart(**joined), (*joined)->endLocation, "incompatible arm types ('" << resultType << "' and '" << armType << "')");
                }
            }
            resultType = armType;
        }
        joinedArms.push_back(armExpr);
    }

    if (resultType.isVoid()) {
        ERROR_RANGE(getExprRangeStart(expr), expr.endLocation, "switch expression arms must produce a value; use a switch statement for side effects");
    }
    return resultType;
}

void Typechecker::warnAboutUnhandledEnumCases(const SwitchStmt& stmt, Type conditionType, bool hadDefault) const {
    if (hadDefault) return;

    auto handledCases = getHandledEnumCases(stmt, conditionType);
    if (!handledCases) return;

    // Don't warn when over half of the cases are missing; partial matching is then assumed intentional.
    auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
    size_t totalCases = enumDecl->cases.size();
    size_t missingCases = totalCases - handledCases->size();
    if (missingCases == 0 || missingCases * 2 > totalCases) return;

    for (auto& enumCase : enumDecl->cases) {
        if (!handledCases->contains(&enumCase)) {
            WARN_RANGE(getExprRangeStart(*stmt.condition), stmt.condition->endLocation,
                       "enumeration value '" << enumCase.getName() << "' not handled in switch");
        }
    }
}

bool Typechecker::tryDesugarEnumIteration(ForEachStmt& forEachStmt) {
    auto* varExpr = llvm::dyn_cast<VarExpr>(forEachStmt.range);
    if (!varExpr) return false;
    Decl* target = findDecl(varExpr->identifier, varExpr->location, varExpr->endLocation);
    if (auto* alias = llvm::dyn_cast<TypeAliasDecl>(target)) {
        Type aliasedType = resolveTypeAliases(alias->aliasedType);
        typecheckType(aliasedType, AccessLevel::None);
        target = aliasedType.getDecl();
        if (!target) return false;
    }
    if (auto* typeTemplate = llvm::dyn_cast<TypeTemplate>(target)) {
        if (llvm::isa<EnumDecl>(typeTemplate->typeDecl)) {
            ERROR_RANGE(getExprRangeStart(*varExpr), varExpr->endLocation, "cannot iterate cases of generic enum '" << typeTemplate->getName() << "'");
        }
        return false;
    }
    auto* enumDecl = llvm::dyn_cast<EnumDecl>(target);
    if (!enumDecl) return false;
    if (enumDecl->instantiatedFrom) {
        ERROR_RANGE(getExprRangeStart(*varExpr), varExpr->endLocation, "cannot iterate cases of generic enum '" << enumDecl->getName() << "'");
    }
    checkHasAccess(*enumDecl, varExpr->location, AccessLevel::None);
    for (auto& enumCase : enumDecl->cases) {
        if (enumCase.associatedType) {
            ERROR_RANGE(getExprRangeStart(*varExpr), varExpr->endLocation,
                        "cannot iterate cases of enum '" << enumDecl->getName() << "' because case '" << enumCase.getName() << "' has associated values");
        }
    }
    AstVector<Expr*> elements;
    for (auto& enumCase : enumDecl->cases) {
        // Base each case on the written name, which may be an alias; the enum's own
        // name could resolve to something else if shadowed by a local.
        elements.push_back(makeAST<MemberExpr>(makeAST<VarExpr>(varExpr->identifier, varExpr->location), enumCase.getName(), varExpr->location));
    }
    auto* array = makeAST<ArrayLiteralExpr>(std::move(elements), varExpr->location);
    if (array->elements.empty()) {
        typecheckExpr(*array, false, BasicType::getArray(enumDecl->getType(), 0));
    } else {
        typecheckExpr(*array);
    }
    forEachStmt.range = array;
    return true;
}

void Typechecker::typecheckForStmt(ForStmt& forStmt) {
    Scope scope(currentFunction, &currentModule->symbolTable);

    if (forStmt.variable) {
        typecheckVarStmt(*forStmt.variable);
    }

    llvm::SaveAndRestore saveLoopEntryLocalCount(loopEntryLocalCount, std::optional<size_t>(localVarDecls.size()));

    // Assignments in the condition, body, or increment also execute on later iterations,
    // so narrowings for variables assigned there don't hold on loop entry or after the loop.
    llvm::StringSet<> assignedNames;
    collectAssignedNames(forStmt.condition, forStmt.body, assignedNames);
    for (auto* increment : forStmt.increments)
        collectAssignedNames(*increment, assignedNames);
    NarrowMap outerNarrowings = narrowedTypes;
    dropNarrowingsForNames(assignedNames);

    if (forStmt.condition) {
        typecheckExpr(*forStmt.condition);
        typecheckImplicitlyBoolConvertibleExpr(forStmt.condition);
    }

    // The body and increment may not execute, so assignments there don't hold after the loop.
    llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
    currentControlStmts.push_back(&forStmt);

    if (forStmt.condition) applyNarrowings(*forStmt.condition, true);

    for (auto& stmt : forStmt.body) {
        typecheckStmt(stmt);
    }

    currentControlStmts.pop_back();

    for (auto* increment : forStmt.increments) {
        typecheckExpr(*increment);
    }

    narrowedTypes = outerNarrowings;
    dropNarrowingsForNames(assignedNames);
}

void Typechecker::typecheckDoWhileStmt(DoWhileStmt& doWhileStmt) {
    // Like for statements, assignments in the condition or body execute on later
    // iterations, so narrowings for variables assigned there don't hold after the loop.
    // Unlike while, the body runs before the first check, so the condition's
    // narrowings don't apply to the body.
    llvm::StringSet<> assignedNames;
    collectAssignedNames(doWhileStmt.condition, doWhileStmt.body, assignedNames);
    NarrowMap outerNarrowings = narrowedTypes;
    dropNarrowingsForNames(assignedNames);

    llvm::SaveAndRestore saveLoopEntryLocalCount(loopEntryLocalCount, std::optional<size_t>(localVarDecls.size()));

    // The body runs before the first check, so unlike while loops its assignments hold after.
    currentControlStmts.push_back(&doWhileStmt);

    for (auto& stmt : doWhileStmt.body) {
        typecheckStmt(stmt);
    }

    currentControlStmts.pop_back();

    typecheckExpr(*doWhileStmt.condition);
    typecheckImplicitlyBoolConvertibleExpr(doWhileStmt.condition);

    narrowedTypes = outerNarrowings;
    dropNarrowingsForNames(assignedNames);
}

void Typechecker::typecheckBreakStmt(BreakStmt& breakStmt) {
    auto innermostBreakable = std::find_if(currentControlStmts.rbegin(), currentControlStmts.rend(), [](const Stmt* stmt) { return stmt->isBreakable(); });
    if (innermostBreakable == currentControlStmts.rend()) {
        ERROR_RANGE(breakStmt.location, getIdentifierEndLocation(breakStmt.location, "break"),
                    "'break' is only allowed inside 'while', 'do-while', 'for', and 'switch' statements");
    }
    // A break out of a switch arm reaches past the switch; capture its move
    // state for the switch merge (the if-merge drops it as diverging).
    if ((*innermostBreakable)->kind == StmtKind::SwitchStmt) {
        switchBreakPaths.push_back({movedDecls, maybeMovedDecls, definitelyAssignedDecls, &breakStmt});
    }
}

void Typechecker::typecheckContinueStmt(ContinueStmt& continueStmt) {
    if (llvm::none_of(currentControlStmts, [](const Stmt* stmt) { return stmt->isContinuable(); })) {
        ERROR_RANGE(continueStmt.location, getIdentifierEndLocation(continueStmt.location, "continue"),
                    "'continue' is only allowed inside 'while', 'do-while', and 'for' statements");
    }
}

void Typechecker::typecheckCompoundStmt(CompoundStmt& compoundStmt) {
    Scope scope(currentFunction, &currentModule->symbolTable);

    for (auto& stmt : compoundStmt.body) {
        typecheckStmt(stmt);
    }
}

bool Typechecker::typecheckStmt(Stmt*& stmt) {
    try {
        switch (stmt->kind) {
        case StmtKind::ReturnStmt:
            typecheckReturnStmt(llvm::cast<ReturnStmt>(*stmt));
            break;
        case StmtKind::VarStmt:
            typecheckVarStmt(llvm::cast<VarStmt>(*stmt));
            break;
        case StmtKind::ExprStmt: {
            auto& exprStmt = *llvm::cast<ExprStmt>(stmt);
            Type type = typecheckExpr(*exprStmt.expr);
            if (!exprStmt.discardsResult) warnIfUnusedResult(*exprStmt.expr, type);
            break;
        }
        case StmtKind::DeferStmt: {
            auto* deferStmt = llvm::cast<DeferStmt>(stmt);
            if (llvm::isa<ConstructorDecl>(currentFunction))
                ERROR_RANGE(deferStmt->location, getIdentifierEndLocation(deferStmt->location, "defer"), "'defer' is not allowed in constructors");
            // Deferred expressions run at scope exit, when narrowings established here may no longer hold.
            llvm::SaveAndRestore saveNarrowings(narrowedTypes, NarrowMap{});
            llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
            auto& expr = *deferStmt->expr;
            warnIfUnusedResult(expr, typecheckExpr(expr));
            break;
        }
        case StmtKind::IfStmt:
            typecheckIfStmt(llvm::cast<IfStmt>(*stmt));
            break;
        case StmtKind::SwitchStmt:
            typecheckSwitchStmt(llvm::cast<SwitchStmt>(*stmt));
            break;
        case StmtKind::WhileStmt: {
            auto* whileStmt = llvm::cast<WhileStmt>(stmt);
            // The lowered ForStmt installs the loop-entry move snapshot.
            stmt = whileStmt->lower();
            typecheckStmt(stmt);
            break;
        }
        case StmtKind::DoWhileStmt:
            typecheckDoWhileStmt(llvm::cast<DoWhileStmt>(*stmt));
            break;
        case StmtKind::ForStmt:
            typecheckForStmt(llvm::cast<ForStmt>(*stmt));
            break;
        case StmtKind::ForEachStmt: {
            auto* forEachStmt = llvm::cast<ForEachStmt>(stmt);
            bool checkLoop = !tryDesugarEnumIteration(*forEachStmt);
            if (checkLoop) {
                typecheckExpr(*forEachStmt->range);
            }
            auto nestLevel = llvm::count_if(currentControlStmts, [](auto* stmt) { return stmt->isForStmt(); });
            // The lowered ForStmt installs the loop-entry move snapshot.
            // Call chains count: elements of `arr.filter(...)` alias frozen
            // storage, while materializing calls such as toList() start fresh.
            size_t mutationStart = viewRootMutations.size();
            size_t candidateStart = viewCallCandidates.size();
            stmt = forEachStmt->lower(nestLevel, exprIsConst(*forEachStmt->range, /*followCalls=*/true));
            typecheckStmt(stmt);
            if (checkLoop) checkLoopBodyFreezes(*forEachStmt->range, mutationStart, candidateStart);
            break;
        }
        case StmtKind::BreakStmt:
            typecheckBreakStmt(llvm::cast<BreakStmt>(*stmt));
            break;
        case StmtKind::ContinueStmt:
            typecheckContinueStmt(llvm::cast<ContinueStmt>(*stmt));
            break;
        case StmtKind::CompoundStmt:
            typecheckCompoundStmt(llvm::cast<CompoundStmt>(*stmt));
            break;
        }
    } catch (const CompileError& error) {
        error.report();
        return false;
    }

    return true;
}
