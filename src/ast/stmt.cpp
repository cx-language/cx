#include "stmt.h"
#include "arena.h"
#include "ast.h"
#include "decl.h"

using namespace cx;

bool Stmt::isBreakable() const {
    switch (kind) {
    case StmtKind::WhileStmt:
    case StmtKind::DoWhileStmt:
    case StmtKind::ForStmt:
    case StmtKind::ForEachStmt:
    case StmtKind::SwitchStmt:
        return true;
    default:
        return false;
    }
}

bool Stmt::isContinuable() const {
    switch (kind) {
    case StmtKind::WhileStmt:
    case StmtKind::DoWhileStmt:
    case StmtKind::ForStmt:
    case StmtKind::ForEachStmt:
        return true;
    default:
        return false;
    }
}

Stmt* Stmt::instantiate(const llvm::StringMap<GenericArg>& genericArgs) const {
    Stmt* result = instantiateImpl(genericArgs);
    result->disabledChecks = disabledChecks;
    return result;
}

Stmt* Stmt::instantiateImpl(const llvm::StringMap<GenericArg>& genericArgs) const {
    switch (kind) {
    case StmtKind::ReturnStmt: {
        auto* returnStmt = llvm::cast<ReturnStmt>(this);
        auto returnValue = returnStmt->value ? returnStmt->value->instantiate(genericArgs) : nullptr;
        return makeAST<ReturnStmt>(returnValue, returnStmt->location);
    }
    case StmtKind::VarStmt: {
        auto* varStmt = llvm::cast<VarStmt>(this);
        llvm::SmallVector<VarDecl*, 1> decls;
        for (auto* decl : varStmt->decls) {
            decls.push_back(llvm::cast<VarDecl>(decl->instantiate(genericArgs, {})));
        }
        return makeAST<VarStmt>(std::move(decls));
    }
    case StmtKind::ExprStmt: {
        auto* exprStmt = llvm::cast<ExprStmt>(this);
        return makeAST<ExprStmt>(exprStmt->expr->instantiate(genericArgs), exprStmt->discardsResult);
    }
    case StmtKind::DeferStmt: {
        auto* deferStmt = llvm::cast<DeferStmt>(this);
        return makeAST<DeferStmt>(deferStmt->expr->instantiate(genericArgs));
    }
    case StmtKind::IfStmt: {
        auto* ifStmt = llvm::cast<IfStmt>(this);
        auto condition = ifStmt->condition->instantiate(genericArgs);
        auto thenBody = ::instantiate(ifStmt->thenBody, genericArgs);
        auto elseBody = ::instantiate(ifStmt->elseBody, genericArgs);
        auto* result = makeAST<IfStmt>(condition, std::move(thenBody), std::move(elseBody), ifStmt->elseLocation);
        result->isBinding = ifStmt->isBinding ? llvm::cast<VarDecl>(ifStmt->isBinding->instantiate(genericArgs, {})) : nullptr;
        return result;
    }
    case StmtKind::SwitchStmt: {
        auto* switchStmt = llvm::cast<SwitchStmt>(this);
        auto condition = switchStmt->condition->instantiate(genericArgs);
        auto cases = mapAst(switchStmt->cases, [&](const SwitchCase& switchCase) {
            auto value = switchCase.value->instantiate(genericArgs);
            auto associatedValue = switchCase.associatedValue ? llvm::cast<VarDecl>(switchCase.associatedValue->instantiate(genericArgs, {})) : nullptr;
            auto stmts = ::instantiate(switchCase.stmts, genericArgs);
            return SwitchCase(value, associatedValue, std::move(stmts));
        });
        auto defaultStmts = ::instantiate(switchStmt->defaultStmts, genericArgs);
        return makeAST<SwitchStmt>(condition, std::move(cases), std::move(defaultStmts));
    }
    case StmtKind::WhileStmt: {
        auto* whileStmt = llvm::cast<WhileStmt>(this);
        auto condition = whileStmt->condition->instantiate(genericArgs);
        auto body = ::instantiate(whileStmt->body, genericArgs);
        return makeAST<WhileStmt>(condition, std::move(body), whileStmt->location);
    }
    case StmtKind::DoWhileStmt: {
        auto* doWhileStmt = llvm::cast<DoWhileStmt>(this);
        auto condition = doWhileStmt->condition->instantiate(genericArgs);
        auto body = ::instantiate(doWhileStmt->body, genericArgs);
        return makeAST<DoWhileStmt>(condition, std::move(body), doWhileStmt->location);
    }
    case StmtKind::ForStmt: {
        auto* forStmt = llvm::cast<ForStmt>(this);
        auto variable = forStmt->variable ? llvm::cast<VarStmt>(forStmt->variable->instantiate(genericArgs)) : nullptr;
        auto condition = forStmt->condition ? forStmt->condition->instantiate(genericArgs) : nullptr;
        auto increments = ::instantiate(forStmt->increments, genericArgs);
        auto body = ::instantiate(forStmt->body, genericArgs);
        return makeAST<ForStmt>(variable, condition, std::move(increments), std::move(body), forStmt->location);
    }
    case StmtKind::ForEachStmt: {
        auto* forEachStmt = llvm::cast<ForEachStmt>(this);
        // The second argument can be empty because VarDecl instantiation doesn't use it.
        auto variable = llvm::cast<VarDecl>(forEachStmt->variable->instantiate(genericArgs, {}));
        auto indexVariable = forEachStmt->indexVariable ? llvm::cast<VarDecl>(forEachStmt->indexVariable->instantiate(genericArgs, {})) : nullptr;
        auto range = forEachStmt->range->instantiate(genericArgs);
        auto body = ::instantiate(forEachStmt->body, genericArgs);
        return makeAST<ForEachStmt>(variable, indexVariable, range, std::move(body), forEachStmt->location);
    }
    case StmtKind::BreakStmt: {
        auto* breakStmt = llvm::cast<BreakStmt>(this);
        return makeAST<BreakStmt>(breakStmt->location);
    }
    case StmtKind::ContinueStmt: {
        auto* continueStmt = llvm::cast<ContinueStmt>(this);
        return makeAST<ContinueStmt>(continueStmt->location);
    }
    case StmtKind::CompoundStmt: {
        auto* compoundStmt = llvm::cast<CompoundStmt>(this);
        auto body = ::instantiate(compoundStmt->body, genericArgs);
        return makeAST<CompoundStmt>(std::move(body));
    }
    }
    llvm_unreachable("all cases handled");
}

Stmt* WhileStmt::lower() {
    auto* lowered = makeAST<ForStmt>(nullptr, condition, AstVector<Expr*>(), std::move(body), location);
    lowered->disabledChecks = disabledChecks;
    return lowered;
}

// Lowers 'for id in range { ... }' into:
// for (var __iterator = range.iterator(); __iterator.hasValue(); __iterator.increment()) {
//     var id = __iterator.value();
//     ...
// }
// With an index variable ('for id, index in range'), a __index counter is
// declared alongside the iterator, bound to a fresh index variable each
// iteration, and incremented with the iterator.
// The loop variable keeps whatever type value() returns. In particular a borrow is
// aliased, not copied out, so elements are mutated in place. Method resolution cannot
// be relied on here (generic contexts leave it unresolved), so the variable is always
// marked and the typechecker exempts plain borrows from the usual read-out.
Stmt* ForEachStmt::lower(int nestLevel, bool rangeIsConst) {
    auto iteratorVariableName = "__iterator" + (nestLevel > 0 ? std::to_string(nestLevel) : "");

    Expr* iteratorValue;
    Type rangeBaseType = range->type.removePointer();
    auto* rangeTypeDecl = rangeBaseType.getDecl();
    bool isIterator = rangeTypeDecl && llvm::any_of(rangeTypeDecl->interfaces, [](Type interface) { return interface.getName() == "Iterator"; });

    if (isIterator) {
        iteratorValue = range;
    } else {
        auto iteratorMemberExpr = makeAST<MemberExpr>(range, "iterator", location);
        iteratorMemberExpr->endLocation = range->endLocation;
        iteratorValue = makeAST<CallExpr>(iteratorMemberExpr, AstVector<NamedValue>(), AstVector<GenericArg>(), location);
        iteratorValue->endLocation = range->endLocation;
    }

    auto iteratorVarDecl =
        makeAST<VarDecl>(Type(nullptr, location), iteratorVariableName, iteratorValue, variable->parent, AccessLevel::None, *variable->getModule(), location);
    auto iteratorVarStmt = makeAST<VarStmt>(llvm::SmallVector<VarDecl*, 1>{iteratorVarDecl});

    std::string indexCounterName;
    if (indexVariable) {
        indexCounterName = "__index" + (nestLevel > 0 ? std::to_string(nestLevel) : "");
        auto zero = makeAST<IntLiteralExpr>(llvm::APSInt(64, false), location);
        auto counterVarDecl = makeAST<VarDecl>(Type(), indexCounterName, zero, variable->parent, AccessLevel::None, *variable->getModule(), location);
        iteratorVarStmt->decls.push_back(counterVarDecl);
    }

    auto iteratorVarExpr = makeAST<VarExpr>(iteratorVariableName, location);
    auto hasValueMemberExpr = makeAST<MemberExpr>(iteratorVarExpr, "hasValue", location);
    auto hasValueCallExpr = makeAST<CallExpr>(hasValueMemberExpr, AstVector<NamedValue>(), AstVector<GenericArg>(), location);

    auto iteratorVarExpr2 = makeAST<VarExpr>(iteratorVariableName, location);
    auto valueMemberExpr = makeAST<MemberExpr>(iteratorVarExpr2, "value", location);
    auto valueCallExpr = makeAST<CallExpr>(valueMemberExpr, AstVector<NamedValue>(), AstVector<GenericArg>(), location);
    auto loopVariableVarDecl = makeAST<VarDecl>(variable->type, variable->getName(), valueCallExpr, variable->parent, AccessLevel::None, *variable->getModule(),
                                                variable->getLocation());
    loopVariableVarDecl->isForLoopElement = true;
    if (rangeIsConst) {
        // The element borrows frozen storage; mirror its constness like comparison temps do.
        loopVariableVarDecl->isConst = true;
        loopVariableVarDecl->isImplicitlyBound = true;
    }
    auto loopVariableVarStmt = makeAST<VarStmt>(llvm::SmallVector<VarDecl*, 1>{loopVariableVarDecl});

    AstVector<Stmt*> forBody;
    forBody.push_back(loopVariableVarStmt);
    if (indexVariable) {
        auto counterVarExpr = makeAST<VarExpr>(indexCounterName, location);
        auto indexVarDecl = makeAST<VarDecl>(indexVariable->type, indexVariable->getName(), counterVarExpr, variable->parent, AccessLevel::None,
                                             *variable->getModule(), indexVariable->getLocation());
        forBody.push_back(makeAST<VarStmt>(llvm::SmallVector<VarDecl*, 1>{indexVarDecl}));
    }

    for (auto& stmt : body) {
        forBody.push_back(stmt);
    }

    auto iteratorVarExpr3 = makeAST<VarExpr>(iteratorVariableName, location);
    auto incrementMemberExpr = makeAST<MemberExpr>(iteratorVarExpr3, "increment", location);
    auto incrementCallExpr = makeAST<CallExpr>(incrementMemberExpr, AstVector<NamedValue>(), AstVector<GenericArg>(), location);
    AstVector<Expr*> increments{incrementCallExpr};
    if (indexVariable) {
        auto counterVarExpr = makeAST<VarExpr>(indexCounterName, location);
        increments.push_back(makeAST<UnaryExpr>(Token::Increment, counterVarExpr, location));
    }
    auto* lowered = makeAST<ForStmt>(iteratorVarStmt, hasValueCallExpr, std::move(increments), std::move(forBody), location);
    lowered->disabledChecks = disabledChecks;
    return lowered;
}
