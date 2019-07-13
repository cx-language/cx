#include "stmt.h"
#include "decl.h"

using namespace delta;

bool Stmt::isBreakable() const {
    switch (getKind()) {
        case StmtKind::WhileStmt:
        case StmtKind::ForStmt:
        case StmtKind::SwitchStmt:
            return true;
        default:
            return false;
    }
}

bool Stmt::isContinuable() const {
    switch (getKind()) {
        case StmtKind::WhileStmt:
        case StmtKind::ForStmt:
            return true;
        default:
            return false;
    }
}

Stmt* Stmt::instantiate(const llvm::StringMap<Type>& genericArgs) const {
    switch (getKind()) {
        case StmtKind::ReturnStmt: {
            auto* returnStmt = llvm::cast<ReturnStmt>(this);
            auto returnValue = returnStmt->getReturnValue() ? returnStmt->getReturnValue()->instantiate(genericArgs) : nullptr;
            return new ReturnStmt(returnValue, returnStmt->getLocation());
        }
        case StmtKind::VarStmt: {
            auto* varStmt = llvm::cast<VarStmt>(this);
            auto instantiation = varStmt->getDecl().instantiate(genericArgs, {});
            return new VarStmt(llvm::cast<VarDecl>(instantiation));
        }
        case StmtKind::ExprStmt: {
            auto* exprStmt = llvm::cast<ExprStmt>(this);
            return new ExprStmt(exprStmt->getExpr().instantiate(genericArgs));
        }
        case StmtKind::DeferStmt: {
            auto* deferStmt = llvm::cast<DeferStmt>(this);
            return new DeferStmt(deferStmt->getExpr().instantiate(genericArgs));
        }
        case StmtKind::IfStmt: {
            auto* ifStmt = llvm::cast<IfStmt>(this);
            auto condition = ifStmt->getCondition().instantiate(genericArgs);
            auto thenBody = ::instantiate(ifStmt->getThenBody(), genericArgs);
            auto elseBody = ::instantiate(ifStmt->getElseBody(), genericArgs);
            return new IfStmt(condition, std::move(thenBody), std::move(elseBody));
        }
        case StmtKind::SwitchStmt: {
            auto* switchStmt = llvm::cast<SwitchStmt>(this);
            auto condition = switchStmt->getCondition().instantiate(genericArgs);
            auto cases = map(switchStmt->getCases(), [&](const SwitchCase& switchCase) {
                auto value = switchCase.getValue()->instantiate(genericArgs);
                auto stmts = ::instantiate(switchCase.getStmts(), genericArgs);
                return SwitchCase(value, std::move(stmts));
            });
            auto defaultStmts = ::instantiate(switchStmt->getDefaultStmts(), genericArgs);
            return new SwitchStmt(condition, std::move(cases), std::move(defaultStmts));
        }
        case StmtKind::WhileStmt: {
            auto* whileStmt = llvm::cast<WhileStmt>(this);
            auto condition = whileStmt->getCondition().instantiate(genericArgs);
            auto body = ::instantiate(whileStmt->getBody(), genericArgs);
            return new WhileStmt(condition, std::move(body), nullptr);
        }
        case StmtKind::ForStmt: {
            auto* forStmt = llvm::cast<ForStmt>(this);
            // The second argument can be empty because VarDecl instantiation doesn't use it.
            auto variable = llvm::cast<VarDecl>(forStmt->getVariable()->instantiate(genericArgs, {}));
            auto range = forStmt->getRangeExpr().instantiate(genericArgs);
            auto body = ::instantiate(forStmt->getBody(), genericArgs);
            return new ForStmt(variable, range, std::move(body), forStmt->getLocation());
        }
        case StmtKind::BreakStmt: {
            auto* breakStmt = llvm::cast<BreakStmt>(this);
            return new BreakStmt(breakStmt->getLocation());
        }
        case StmtKind::ContinueStmt: {
            auto* continueStmt = llvm::cast<ContinueStmt>(this);
            return new ContinueStmt(continueStmt->getLocation());
        }
        case StmtKind::CompoundStmt: {
            auto* compoundStmt = llvm::cast<CompoundStmt>(this);
            auto body = ::instantiate(compoundStmt->getBody(), genericArgs);
            return new CompoundStmt(std::move(body));
        }
    }
    llvm_unreachable("all cases handled");
}

// Lowers 'for (var id in range) { ... }' into:
// {
//     var __iterator = range.iterator();
//     // or if 'range' is already an iterator:
//     var __iterator = range;
//
//     while (__iterator.hasValue()) {
//         var id = __iterator.value();
//         ...
//         __iterator.increment();
//     }
// }
Stmt* ForStmt::lower(int nestLevel) {
    auto iteratorVariableName = "__iterator" + (nestLevel > 0 ? std::to_string(nestLevel) : "");
    auto location = getLocation();

    std::vector<Stmt*> stmts;

    Expr* iteratorValue;
    auto* rangeTypeDecl = range->getType().removePointer().getDecl();
    bool isIterator = rangeTypeDecl &&
                      llvm::any_of(rangeTypeDecl->getInterfaces(), [](Type interface) { return interface.getName() == "Iterator"; });

    if (isIterator) {
        iteratorValue = range;
    } else {
        auto iteratorMemberExpr = new MemberExpr(range, "iterator", location);
        iteratorValue = new CallExpr(iteratorMemberExpr, std::vector<NamedValue>(), std::vector<Type>(), location);
    }

    auto iteratorVarDecl = new VarDecl(Type(nullptr, Mutability::Mutable, location), std::string(iteratorVariableName), iteratorValue,
                                       variable->getParent(), AccessLevel::None, *variable->getModule(), location);
    auto iteratorVarStmt = new VarStmt(iteratorVarDecl);
    stmts.push_back(iteratorVarStmt);

    auto iteratorVarExpr = new VarExpr(std::string(iteratorVariableName), location);
    auto hasValueMemberExpr = new MemberExpr(iteratorVarExpr, "hasValue", location);
    auto hasValueCallExpr = new CallExpr(hasValueMemberExpr, std::vector<NamedValue>(), std::vector<Type>(), location);

    auto iteratorVarExpr2 = new VarExpr(std::string(iteratorVariableName), location);
    auto valueMemberExpr = new MemberExpr(iteratorVarExpr2, "value", location);
    auto valueCallExpr = new CallExpr(valueMemberExpr, std::vector<NamedValue>(), std::vector<Type>(), location);
    auto loopVariableVarDecl = new VarDecl(variable->getType(), variable->getName(), valueCallExpr, variable->getParent(),
                                           AccessLevel::None, *variable->getModule(), variable->getLocation());
    auto loopVariableVarStmt = new VarStmt(loopVariableVarDecl);

    std::vector<Stmt*> forStmtBody;
    forStmtBody.push_back(loopVariableVarStmt);

    for (auto& stmt : this->body) {
        forStmtBody.push_back(stmt);
    }

    auto iteratorVarExpr3 = new VarExpr(std::string(iteratorVariableName), location);
    auto incrementMemberExpr = new MemberExpr(iteratorVarExpr3, "increment", location);
    auto incrementCallExpr = new CallExpr(incrementMemberExpr, std::vector<NamedValue>(), std::vector<Type>(), location);

    auto whileStmt = new WhileStmt(hasValueCallExpr, std::move(forStmtBody), incrementCallExpr);
    stmts.push_back(whileStmt);

    return new CompoundStmt(std::move(stmts));
}
