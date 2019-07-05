#include "typecheck.h"
#include "../ast/module.h"

using namespace delta;

void Typechecker::checkReturnPointerToLocal(const ReturnStmt& stmt) const {
    auto* returnValue = stmt.getReturnValue();

    if (auto* unaryExpr = llvm::dyn_cast<UnaryExpr>(returnValue)) {
        if (unaryExpr->getOperator() == Token::And) {
            returnValue = &unaryExpr->getOperand();
        }
    }

    Type localVariableType;

    if (auto* varExpr = llvm::dyn_cast<VarExpr>(returnValue)) {
        switch (varExpr->getDecl()->getKind()) {
            case DeclKind::VarDecl: {
                auto* varDecl = llvm::cast<VarDecl>(varExpr->getDecl());
                if (varDecl->getParent() && varDecl->getParent()->isFunctionDecl()) {
                    localVariableType = varDecl->getType();
                }
                break;
            }
            case DeclKind::ParamDecl:
                localVariableType = llvm::cast<ParamDecl>(varExpr->getDecl())->getType();
                break;

            default:
                break;
        }
    }

    if (localVariableType && functionReturnType.removeOptional().isPointerType() &&
        functionReturnType.removeOptional().getPointee().equalsIgnoreTopLevelMutable(localVariableType)) {
        warning(returnValue->getLocation(), "returning pointer to local variable ",
                "(local variables will not exist after the function returns)");
    }
}

void Typechecker::typecheckReturnStmt(ReturnStmt& stmt) {
    if (!stmt.getReturnValue()) {
        if (!functionReturnType.isVoid()) {
            error(stmt.getLocation(), "expected return statement to return a value of type '", functionReturnType, "'");
        }
        return;
    }

    Type returnValueType = typecheckExpr(*stmt.getReturnValue());
    Type convertedType;

    if (!isImplicitlyConvertible(stmt.getReturnValue(), returnValueType, functionReturnType, &convertedType)) {
        error(stmt.getLocation(), "mismatching return type '", returnValueType, "', expected '", functionReturnType, "'");
    }

    stmt.getReturnValue()->setType(convertedType ? convertedType : returnValueType);
    checkReturnPointerToLocal(stmt);
    stmt.getReturnValue()->setMoved(true);
}

void Typechecker::typecheckVarStmt(VarStmt& stmt) {
    typecheckVarDecl(stmt.getDecl(), false);
}

void Typechecker::typecheckIfStmt(IfStmt& ifStmt) {
    Type conditionType = typecheckExpr(ifStmt.getCondition());

    if (!conditionType.isBool() && !conditionType.isOptionalType()) {
        error(ifStmt.getCondition().getLocation(), "'if' condition must have type 'bool' or optional type");
    }

    currentControlStmts.push_back(&ifStmt);

    for (auto& stmt : ifStmt.getThenBody()) {
        typecheckStmt(stmt);
    }

    for (auto& stmt : ifStmt.getElseBody()) {
        typecheckStmt(stmt);
    }

    currentControlStmts.pop_back();
}

void Typechecker::typecheckSwitchStmt(SwitchStmt& stmt) {
    Type conditionType = typecheckExpr(stmt.getCondition());

    currentControlStmts.push_back(&stmt);

    for (auto& switchCase : stmt.getCases()) {
        Type caseType = typecheckExpr(*switchCase.getValue());

        if (!isImplicitlyConvertible(switchCase.getValue(), caseType, conditionType, nullptr)) {
            error(switchCase.getValue()->getLocation(), "case value type '", caseType, "' doesn't match switch condition type '",
                  conditionType, "'");
        }
        for (auto& caseStmt : switchCase.getStmts()) {
            typecheckStmt(caseStmt);
        }
    }

    for (auto& defaultStmt : stmt.getDefaultStmts()) {
        typecheckStmt(defaultStmt);
    }

    currentControlStmts.pop_back();
}

void Typechecker::typecheckWhileStmt(WhileStmt& whileStmt) {
    Type conditionType = typecheckExpr(whileStmt.getCondition());

    if (!conditionType.isBool() && !conditionType.isOptionalType()) {
        error(whileStmt.getCondition().getLocation(), "'while' condition must have type 'bool' or optional type");
    }

    currentControlStmts.push_back(&whileStmt);

    for (auto& stmt : whileStmt.getBody()) {
        typecheckStmt(stmt);
    }

    currentControlStmts.pop_back();

    if (auto* increment = whileStmt.getIncrement()) {
        typecheckExpr(*increment);
    }
}

void Typechecker::typecheckBreakStmt(BreakStmt& breakStmt) {
    if (llvm::none_of(currentControlStmts, [](const Stmt* stmt) { return stmt->isBreakable(); })) {
        error(breakStmt.getLocation(), "'break' is only allowed inside 'while', 'for', and 'switch' statements");
    }
}

void Typechecker::typecheckContinueStmt(ContinueStmt& continueStmt) {
    if (llvm::none_of(currentControlStmts, [](const Stmt* stmt) { return stmt->isContinuable(); })) {
        error(continueStmt.getLocation(), "'continue' is only allowed inside 'while' and 'for' statements");
    }
}

void Typechecker::typecheckCompoundStmt(CompoundStmt& compoundStmt) {
    getCurrentModule()->getSymbolTable().pushScope(currentFunction);

    for (auto& stmt : compoundStmt.getBody()) {
        typecheckStmt(stmt);
    }

    getCurrentModule()->getSymbolTable().popScope();
}

void Typechecker::typecheckStmt(std::unique_ptr<Stmt>& stmt) {
    switch (stmt->getKind()) {
        case StmtKind::ReturnStmt:
            typecheckReturnStmt(llvm::cast<ReturnStmt>(*stmt));
            break;
        case StmtKind::VarStmt:
            typecheckVarStmt(llvm::cast<VarStmt>(*stmt));
            break;
        case StmtKind::ExprStmt:
            typecheckExpr(llvm::cast<ExprStmt>(*stmt).getExpr());
            break;
        case StmtKind::DeferStmt:
            typecheckExpr(llvm::cast<DeferStmt>(*stmt).getExpr());
            break;
        case StmtKind::IfStmt:
            typecheckIfStmt(llvm::cast<IfStmt>(*stmt));
            break;
        case StmtKind::SwitchStmt:
            typecheckSwitchStmt(llvm::cast<SwitchStmt>(*stmt));
            break;
        case StmtKind::WhileStmt:
            typecheckWhileStmt(llvm::cast<WhileStmt>(*stmt));
            break;
        case StmtKind::ForStmt: {
            typecheckExpr(llvm::cast<ForStmt>(*stmt).getRangeExpr());
            auto nestLevel = llvm::count_if(currentControlStmts, [](auto* stmt) { return stmt->isWhileStmt(); });
            stmt = llvm::cast<ForStmt>(*stmt).lower(nestLevel);
            typecheckStmt(stmt);
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
}
