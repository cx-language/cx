#include "typecheck.h"
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"

using namespace cx;

// True when no path through the block falls through to the next statement: every path
// returns, calls a never-returning function, or breaks/continues past the analyzed block.
// `break`/`continue` inside a nested loop or switch target that construct instead.
static bool allPathsDiverge(llvm::ArrayRef<Stmt*> block, int nestLevel = 0) {
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
        if (!call->isMethodCall() && call->getFunctionName() == "assert" && call->args.size() == 1) {
            if (auto* condition = llvm::dyn_cast<BoolLiteralExpr>(call->args[0].value)) {
                return !condition->value;
            }
        }
        return false;
    }
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(*block.back());
        return allPathsDiverge(ifStmt.thenBody, nestLevel) && allPathsDiverge(ifStmt.elseBody, nestLevel);
    }
    case StmtKind::SwitchStmt: {
        auto& switchStmt = llvm::cast<SwitchStmt>(*block.back());
        return llvm::all_of(switchStmt.cases, [&](SwitchCase& c) { return allPathsDiverge(c.stmts, nestLevel + 1); })
            && allPathsDiverge(switchStmt.defaultStmts, nestLevel + 1);
    }
    case StmtKind::CompoundStmt:
        return allPathsDiverge(llvm::cast<CompoundStmt>(*block.back()).body, nestLevel);
    default:
        return false;
    }
}

static void collectAssignedNames(const Expr& expr, llvm::StringSet<>& names);
static void collectAssignedNames(const Stmt* stmt, llvm::StringSet<>& names);

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
    case ExprKind::TupleExpr:
        for (auto& element : llvm::cast<TupleExpr>(expr).elements)
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
        collectAssignedNames(*llvm::cast<UnwrapExpr>(expr).operand, names);
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
        if (auto* initializer = llvm::cast<VarStmt>(stmt)->decl->initializer) collectAssignedNames(*initializer, names);
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
        collectAssignedNames(*whileStmt.condition, names);
        for (auto& bodyStmt : whileStmt.body)
            collectAssignedNames(bodyStmt, names);
        return;
    }
    case StmtKind::ForStmt: {
        auto& forStmt = llvm::cast<ForStmt>(*stmt);
        if (forStmt.variable) collectAssignedNames(forStmt.variable, names);
        if (forStmt.condition) collectAssignedNames(*forStmt.condition, names);
        if (forStmt.increment) collectAssignedNames(*forStmt.increment, names);
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
    NarrowMap outerNarrowings = narrowedTypes;
    NarrowMap thenNarrowings, elseNarrowings;

    {
        llvm::SaveAndRestore saveMovedDecls(movedDecls);
        applyNarrowings(*ifStmt.condition, true);
        for (auto& stmt : ifStmt.thenBody) {
            typecheckStmt(stmt);
        }
        thenMovedDecls = movedDecls;
        thenNarrowings = narrowedTypes;
        narrowedTypes = outerNarrowings;
    }

    {
        llvm::SaveAndRestore saveMovedDecls(movedDecls);
        applyNarrowings(*ifStmt.condition, false);
        for (auto& stmt : ifStmt.elseBody) {
            typecheckStmt(stmt);
        }
        elseMovedDecls = movedDecls;
        elseNarrowings = narrowedTypes;
        narrowedTypes = outerNarrowings;
    }

    llvm::SmallPtrSet<Decl*, 32> mergedMovedDecls;
    for (auto* decl : thenMovedDecls) {
        if (elseMovedDecls.count(decl)) {
            mergedMovedDecls.insert(decl);
        }
    }
    movedDecls = std::move(mergedMovedDecls);

    // A narrowing holds after the if only if it holds on every path reaching past it.
    // When one branch diverges (e.g. `if x == null return;`), the other branch decides.
    bool thenDiverges = allPathsDiverge(ifStmt.thenBody);
    bool elseDiverges = !ifStmt.elseBody.empty() && allPathsDiverge(ifStmt.elseBody);
    if (thenDiverges && !elseDiverges) {
        narrowedTypes = elseNarrowings;
    } else if (elseDiverges && !thenDiverges) {
        narrowedTypes = thenNarrowings;
    } else if (!thenDiverges && !elseDiverges) {
        narrowedTypes = thenNarrowings;
        intersectNarrowings(elseNarrowings);
    }

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
        NarrowMap outerNarrowings = narrowedTypes;

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
        narrowedTypes = outerNarrowings;
    }

    {
        NarrowMap outerNarrowings = narrowedTypes;
        for (auto& defaultStmt : stmt.defaultStmts) {
            typecheckStmt(defaultStmt);
        }
        narrowedTypes = outerNarrowings;
    }

    dropNarrowingsForNames(bodyAssignedNames);

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

    // Assignments in the condition, body, or increment also execute on later iterations,
    // so narrowings for variables assigned there don't hold on loop entry or after the loop.
    llvm::StringSet<> assignedNames;
    if (forStmt.condition) collectAssignedNames(*forStmt.condition, assignedNames);
    for (auto& stmt : forStmt.body)
        collectAssignedNames(stmt, assignedNames);
    if (forStmt.increment) collectAssignedNames(*forStmt.increment, assignedNames);
    NarrowMap outerNarrowings = narrowedTypes;
    dropNarrowingsForNames(assignedNames);

    if (forStmt.condition) {
        Type conditionType = typecheckExpr(*forStmt.condition);
        typecheckImplicitlyBoolConvertibleExpr(conditionType, forStmt.condition->location);
    }

    currentControlStmts.push_back(&forStmt);

    if (forStmt.condition) applyNarrowings(*forStmt.condition, true);

    for (auto& stmt : forStmt.body) {
        typecheckStmt(stmt);
    }

    currentControlStmts.pop_back();

    if (auto* increment = forStmt.increment) {
        typecheckExpr(*increment);
    }

    narrowedTypes = outerNarrowings;
    dropNarrowingsForNames(assignedNames);
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
        case StmtKind::DeferStmt: {
            // Deferred expressions run at scope exit, when narrowings established here may no longer hold.
            llvm::SaveAndRestore saveNarrowings(narrowedTypes, NarrowMap{});
            typecheckExpr(*llvm::cast<DeferStmt>(stmt)->expr);
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
