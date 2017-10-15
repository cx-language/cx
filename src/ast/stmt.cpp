#include "stmt.h"
#include "decl.h"

using namespace delta;

std::unique_ptr<Stmt> Stmt::instantiate(const llvm::StringMap<Type>& genericArgs) const {
    switch (getKind()) {
        case StmtKind::ReturnStmt: {
            auto* returnStmt = llvm::cast<ReturnStmt>(this);
            auto returnValues = map(returnStmt->getValues(), [&](const std::unique_ptr<Expr>& returnValue) {
                return returnValue->instantiate(genericArgs);
            });
            return llvm::make_unique<ReturnStmt>(std::move(returnValues), returnStmt->getLocation());
        }
        case StmtKind::VarStmt: {
            auto* varStmt = llvm::cast<VarStmt>(this);
            auto instantiation = varStmt->getDecl().instantiate(genericArgs, {});
            return llvm::make_unique<VarStmt>(llvm::cast<VarDecl>(std::move(instantiation)));
        }
        case StmtKind::IncrementStmt: {
            auto* incrementStmt = llvm::cast<IncrementStmt>(this);
            return llvm::make_unique<IncrementStmt>(incrementStmt->getOperand().instantiate(genericArgs),
                                                    incrementStmt->getLocation());
        }
        case StmtKind::DecrementStmt: {
            auto* decrementStmt = llvm::cast<DecrementStmt>(this);
            return llvm::make_unique<DecrementStmt>(decrementStmt->getOperand().instantiate(genericArgs),
                                                    decrementStmt->getLocation());
        }
        case StmtKind::ExprStmt: {
            auto* exprStmt = llvm::cast<ExprStmt>(this);
            return llvm::make_unique<ExprStmt>(exprStmt->getExpr().instantiate(genericArgs));
        }
        case StmtKind::DeferStmt: {
            auto* deferStmt = llvm::cast<DeferStmt>(this);
            return llvm::make_unique<DeferStmt>(deferStmt->getExpr().instantiate(genericArgs));
        }
        case StmtKind::IfStmt: {
            auto* ifStmt = llvm::cast<IfStmt>(this);
            auto condition = ifStmt->getCondition().instantiate(genericArgs);
            auto thenBody = ::instantiate(ifStmt->getThenBody(), genericArgs);
            auto elseBody = ::instantiate(ifStmt->getElseBody(), genericArgs);
            return llvm::make_unique<IfStmt>(std::move(condition), std::move(thenBody),
                                             std::move(elseBody));
        }
        case StmtKind::SwitchStmt: {
            auto* switchStmt = llvm::cast<SwitchStmt>(this);
            auto condition = switchStmt->getCondition().instantiate(genericArgs);
            auto cases = map(switchStmt->getCases(), [&](const SwitchCase& switchCase) {
                auto value = switchCase.getValue()->instantiate(genericArgs);
                auto stmts = ::instantiate(switchCase.getStmts(), genericArgs);
                return SwitchCase(std::move(value), std::move(stmts));
            });
            auto defaultStmts = ::instantiate(switchStmt->getDefaultStmts(), genericArgs);
            return llvm::make_unique<SwitchStmt>(std::move(condition), std::move(cases),
                                                 std::move(defaultStmts));
        }
        case StmtKind::WhileStmt: {
            auto* whileStmt = llvm::cast<WhileStmt>(this);
            auto condition = whileStmt->getCondition().instantiate(genericArgs);
            auto body = ::instantiate(whileStmt->getBody(), genericArgs);
            return llvm::make_unique<WhileStmt>(std::move(condition), std::move(body));
        }
        case StmtKind::ForStmt: {
            auto* forStmt = llvm::cast<ForStmt>(this);
            // The second argument can be empty because VarDecl instantiation doesn't use it. 
            auto variable = llvm::cast<VarDecl>(forStmt->getVariable()->instantiate(genericArgs, {}));
            auto range = forStmt->getRangeExpr().instantiate(genericArgs);
            auto body = ::instantiate(forStmt->getBody(), genericArgs);
            return llvm::make_unique<ForStmt>(std::move(variable), std::move(range),
                                              std::move(body), forStmt->getLocation());
        }
        case StmtKind::BreakStmt: {
            auto* breakStmt = llvm::cast<BreakStmt>(this);
            return llvm::make_unique<BreakStmt>(breakStmt->getLocation());
        }
        case StmtKind::AssignStmt: {
            auto* assignStmt = llvm::cast<AssignStmt>(this);
            auto lhs = assignStmt->getLHS()->instantiate(genericArgs);
            auto rhs = assignStmt->getRHS()->instantiate(genericArgs);
            return llvm::make_unique<AssignStmt>(std::move(lhs), std::move(rhs),
                                                 assignStmt->isCompoundAssignment(),
                                                 assignStmt->getLocation());
        }
        case StmtKind::CompoundStmt: {
            auto* compoundStmt = llvm::cast<CompoundStmt>(this);
            auto body = ::instantiate(compoundStmt->getBody(), genericArgs);
            return llvm::make_unique<CompoundStmt>(std::move(body));
        }
    }
    llvm_unreachable("all cases handled");
}

// Lowers 'for (let id in range) { ... }' into:
//
// {
//     var __iterator = range.iterator();
//
//     while (__iterator.hasNext()) {
//         let id = __iterator.next();
//         ...
//     }
// }
std::unique_ptr<Stmt> ForStmt::lower() {
    static uint64_t iteratorVariableNameCounter = 0;
    auto iteratorVariableName = "__iterator" + std::to_string(iteratorVariableNameCounter++);
    auto location = getLocation();

    std::vector<std::unique_ptr<Stmt>> stmts;

    auto iteratorMemberExpr = llvm::make_unique<MemberExpr>(std::move(range), "iterator", location);
    auto iteratorCallExpr = llvm::make_unique<CallExpr>(std::move(iteratorMemberExpr), std::vector<Argument>(),
                                                        std::vector<Type>(), location);
    auto iteratorVarDecl = llvm::make_unique<VarDecl>(Type(nullptr, true), std::string(iteratorVariableName),
                                                      std::move(iteratorCallExpr),
                                                      *variable->getModule(), location);
    auto iteratorVarStmt = llvm::make_unique<VarStmt>(std::move(iteratorVarDecl));
    stmts.push_back(std::move(iteratorVarStmt));

    auto iteratorVarExpr = llvm::make_unique<VarExpr>(std::string(iteratorVariableName), location);
    auto hasNextMemberExpr = llvm::make_unique<MemberExpr>(std::move(iteratorVarExpr), "hasNext", location);
    auto hasNextCallExpr = llvm::make_unique<CallExpr>(std::move(hasNextMemberExpr), std::vector<Argument>(),
                                                       std::vector<Type>(), location);

    auto iteratorVarExpr2 = llvm::make_unique<VarExpr>(std::string(iteratorVariableName), location);
    auto nextMemberExpr = llvm::make_unique<MemberExpr>(std::move(iteratorVarExpr2), "next", location);
    auto nextCallExpr = llvm::make_unique<CallExpr>(std::move(nextMemberExpr), std::vector<Argument>(),
                                                    std::vector<Type>(), location);
    auto loopVariableVarDecl = llvm::make_unique<VarDecl>(variable->getType(), variable->getName(),
                                                          std::move(nextCallExpr), *variable->getModule(),
                                                          variable->getLocation());
    auto loopVariableVarStmt = llvm::make_unique<VarStmt>(std::move(loopVariableVarDecl));

    std::vector<std::unique_ptr<Stmt>> forStmtBody;
    forStmtBody.push_back(std::move(loopVariableVarStmt));

    for (auto& stmt : this->body) {
        forStmtBody.push_back(std::move(stmt));
    }

    auto whileStmt = llvm::make_unique<WhileStmt>(std::move(hasNextCallExpr), std::move(forStmtBody));
    stmts.push_back(std::move(whileStmt));

    return llvm::make_unique<CompoundStmt>(std::move(stmts));
}
