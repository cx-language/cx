#include "typecheck.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"

using namespace cx;

void Typechecker::checkReturnPointerToLocal(const Expr* returnValue) const {
    if (auto* unaryExpr = llvm::dyn_cast<UnaryExpr>(returnValue)) {
        if (unaryExpr->op == Token::And) {
            returnValue = &unaryExpr->getOperand();
        }
    }

    Type localVariableType;
    const Expr* operand = returnValue;

    if (auto implicitCastExpr = llvm::dyn_cast<ImplicitCastExpr>(returnValue)) {
        operand = implicitCastExpr->operand;
    }

    if (auto varExpr = llvm::dyn_cast<VarExpr>(operand)) {
        switch (varExpr->decl->kind) {
        case DeclKind::VarDecl: {
            auto* varDecl = llvm::cast<VarDecl>(varExpr->decl);
            if (varDecl->parent && varDecl->parent->isFunctionDecl()) {
                localVariableType = varDecl->type;
            }
            break;
        }
        case DeclKind::ParamDecl:
            localVariableType = llvm::cast<ParamDecl>(varExpr->decl)->type;
            break;

        default:
            break;
        }
    }

    if (localVariableType && currentFunction->getReturnType().removeOptional().isPointerType()
        && currentFunction->getReturnType().removeOptional().getPointee().equalsIgnoreTopLevelMutable(localVariableType)) {
        WARN(returnValue->location, "returning pointer to local variable (local variables will not exist after the function returns)");
    }
}

void Typechecker::typecheckReturnStmt(ReturnStmt& stmt) {
    Type returnValueType = stmt.value ? typecheckExpr(*stmt.value, false, currentFunction->getReturnType()) : Type::getVoid();

    if (!currentFunction->getReturnType()) {
        ASSERT(currentFunction->isLambda());
        currentFunction->proto.returnType = returnValueType;
    }

    if (!stmt.value) {
        if (!currentFunction->getReturnType().isVoid()) {
            ERROR(stmt.location, "expected return statement to return a value of type '" << currentFunction->getReturnType() << "'");
        }
        return;
    }

    if (auto converted = convert(stmt.value, currentFunction->getReturnType())) {
        stmt.value = converted;
    } else {
        ERROR(stmt.location, "mismatching return type '" << returnValueType << "', expected '" << currentFunction->getReturnType() << "'");
    }

    checkReturnPointerToLocal(stmt.value);
    setMoved(stmt.value, true);
}

void Typechecker::typecheckVarStmt(VarStmt& stmt) {
    typecheckVarDecl(*stmt.decl);
}

void Typechecker::typecheckIfStmt(IfStmt& ifStmt) {
    Type conditionType = typecheckExpr(*ifStmt.condition);
    typecheckImplicitlyBoolConvertibleExpr(conditionType, ifStmt.condition->location);
    currentControlStmts.push_back(&ifStmt);

    // A value moved in every branch is moved after the if statement. Moves from only one
    // branch are discarded: the value may still be live on the other path. An empty else
    // body moves nothing, so then-only moves never propagate.
    llvm::SmallPtrSet<Decl*, 32> thenMovedDecls, elseMovedDecls;

    {
        llvm::SaveAndRestore saveMovedDecls(movedDecls);
        for (auto& stmt : ifStmt.thenBody) {
            typecheckStmt(stmt);
        }
        thenMovedDecls = movedDecls;
    }

    {
        llvm::SaveAndRestore saveMovedDecls(movedDecls);
        for (auto& stmt : ifStmt.elseBody) {
            typecheckStmt(stmt);
        }
        elseMovedDecls = movedDecls;
    }

    llvm::SmallPtrSet<Decl*, 32> mergedMovedDecls;
    for (auto* decl : thenMovedDecls) {
        if (elseMovedDecls.count(decl)) {
            mergedMovedDecls.insert(decl);
        }
    }
    movedDecls = std::move(mergedMovedDecls);

    currentControlStmts.pop_back();
}

void Typechecker::typecheckSwitchStmt(SwitchStmt& stmt) {
    Type conditionType = typecheckExpr(*stmt.condition);

    if (conditionType.isPointerType()) {
        Type pointeeType = conditionType.getPointee();
        // Automatically dereference pointers to switchable values. Enums with associated values are excluded;
        // they need the address for tag/associated-value access, so dereference those explicitly (e.g. `switch (*p)`).
        bool isPlainEnum = pointeeType.isEnumType() && !llvm::cast<EnumDecl>(pointeeType.getDecl())->hasAssociatedValues();
        if (pointeeType.isInteger() || pointeeType.isChar() || isPlainEnum) {
            if (auto dereferenced = convert(stmt.condition, pointeeType)) {
                stmt.condition = dereferenced;
                conditionType = pointeeType;
            }
        }
    }

    if (!conditionType.isInteger() && !conditionType.isChar() && !conditionType.isEnumType()) {
        ERROR(stmt.condition->location, "switch condition must have integer, char, or enum type, got '" << conditionType << "'");
    }

    currentControlStmts.push_back(&stmt);

    for (auto& switchCase : stmt.cases) {
        if (conditionType.isEnumType()) {
            if (auto* varExpr = llvm::dyn_cast<VarExpr>(switchCase.value)) {
                auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
                if (enumDecl->getCaseByName(varExpr->identifier)) {
                    // A bare `case B:` mirrors the qualified `case E.B:`, so desugar to the qualified form.
                    switchCase.value = makeAST<MemberExpr>(makeAST<VarExpr>(std::string(enumDecl->getName()), varExpr->location),
                                                           std::string(varExpr->identifier), varExpr->location);
                }
            }
        }

        Type caseType = typecheckExpr(*switchCase.value);

        if (auto converted = convert(switchCase.value, conditionType)) {
            switchCase.value = converted;
        } else {
            ERROR(switchCase.value->location, "case value type '" << caseType << "' doesn't match switch condition type '" << conditionType << "'");
        }

        auto* memberExpr = llvm::dyn_cast<MemberExpr>(switchCase.value);
        auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
        if (!enumCase && !switchCase.value->isConstant()) {
            ERROR(switchCase.value->location, "case value must be constant");
        }

        Scope scope(nullptr, &currentModule->symbolTable);

        if (auto* associatedValue = switchCase.associatedValue) {
            if (!enumCase) {
                ERROR(associatedValue->location, "only enum cases can bind associated values");
            }
            if (!enumCase->associatedType) {
                ERROR(associatedValue->location, "enum case '" << enumCase->getName() << "' has no associated values to bind");
            }
            associatedValue->type = NOTNULL(enumCase->associatedType);
            typecheckVarDecl(*associatedValue);
        }

        for (auto& caseStmt : switchCase.stmts) {
            typecheckStmt(caseStmt);
        }
    }

    for (auto& defaultStmt : stmt.defaultStmts) {
        typecheckStmt(defaultStmt);
    }

    currentControlStmts.pop_back();

    warnAboutUnhandledEnumCases(stmt, conditionType);
}

void Typechecker::warnAboutUnhandledEnumCases(const SwitchStmt& stmt, Type conditionType) const {
    if (!conditionType.isEnumType() || !stmt.defaultStmts.empty()) return;

    auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
    llvm::StringSet<> handledCases;
    for (auto& switchCase : stmt.cases) {
        auto* memberExpr = llvm::dyn_cast<MemberExpr>(switchCase.value);
        auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
        if (!enumCase || enumCase->getEnumDecl() != enumDecl) return;
        handledCases.insert(enumCase->getName());
    }

    // Don't warn when over half of the cases are missing; partial matching is then assumed intentional.
    size_t totalCases = enumDecl->cases.size();
    size_t missingCases = totalCases - handledCases.size();
    if (missingCases == 0 || missingCases * 2 > totalCases) return;

    for (auto& enumCase : enumDecl->cases) {
        if (!handledCases.contains(enumCase.getName())) {
            WARN(stmt.condition->location, "enumeration value '" << enumCase.getName() << "' not handled in switch");
        }
    }
}

void Typechecker::typecheckForStmt(ForStmt& forStmt) {
    Scope scope(currentFunction, &currentModule->symbolTable);

    if (forStmt.variable) {
        typecheckVarStmt(*forStmt.variable);
    }

    if (forStmt.condition) {
        Type conditionType = typecheckExpr(*forStmt.condition);
        typecheckImplicitlyBoolConvertibleExpr(conditionType, forStmt.condition->location);
    }

    currentControlStmts.push_back(&forStmt);

    for (auto& stmt : forStmt.body) {
        typecheckStmt(stmt);
    }

    currentControlStmts.pop_back();

    if (auto* increment = forStmt.increment) {
        typecheckExpr(*increment);
    }
}

void Typechecker::typecheckBreakStmt(BreakStmt& breakStmt) {
    if (llvm::none_of(currentControlStmts, [](const Stmt* stmt) { return stmt->isBreakable(); })) {
        ERROR(breakStmt.location, "'break' is only allowed inside 'while', 'for', and 'switch' statements");
    }
}

void Typechecker::typecheckContinueStmt(ContinueStmt& continueStmt) {
    if (llvm::none_of(currentControlStmts, [](const Stmt* stmt) { return stmt->isContinuable(); })) {
        ERROR(continueStmt.location, "'continue' is only allowed inside 'while' and 'for' statements");
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
        case StmtKind::ExprStmt:
            typecheckExpr(*llvm::cast<ExprStmt>(stmt)->expr);
            break;
        case StmtKind::DeferStmt:
            typecheckExpr(*llvm::cast<DeferStmt>(stmt)->expr);
            break;
        case StmtKind::IfStmt:
            typecheckIfStmt(llvm::cast<IfStmt>(*stmt));
            break;
        case StmtKind::SwitchStmt:
            typecheckSwitchStmt(llvm::cast<SwitchStmt>(*stmt));
            break;
        case StmtKind::WhileStmt: {
            auto* whileStmt = llvm::cast<WhileStmt>(stmt);
            stmt = whileStmt->lower();
            typecheckStmt(stmt);
            break;
        }
        case StmtKind::ForStmt:
            typecheckForStmt(llvm::cast<ForStmt>(*stmt));
            break;
        case StmtKind::ForEachStmt: {
            auto* forEachStmt = llvm::cast<ForEachStmt>(stmt);
            typecheckExpr(*forEachStmt->range);
            auto nestLevel = llvm::count_if(currentControlStmts, [](auto* stmt) { return stmt->isForStmt(); });
            stmt = forEachStmt->lower(nestLevel);
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
    } catch (const CompileError& error) {
        error.report();
        return false;
    }

    return true;
}
