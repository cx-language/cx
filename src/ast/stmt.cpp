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
            auto range = forStmt->getRangeExpr().instantiate(genericArgs);
            auto body = ::instantiate(forStmt->getBody(), genericArgs);
            return llvm::make_unique<ForStmt>(forStmt->getLoopVariableName(), std::move(range),
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
    }
    llvm_unreachable("all cases handled");
}
