#include "typecheck.h"
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringSet.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../driver/driver.h"

using namespace cx;

// True when a switch case body may fall through to the code after the switch.
// Unlike allPathsDiverge, 'break' falls through (it exits the switch), while
// 'continue' doesn't (it skips past the switch to the loop increment).
static bool switchCaseMayFallThrough(llvm::ArrayRef<Stmt*> block) {
    if (block.empty()) return true;

    switch (block.back()->kind) {
    case StmtKind::ReturnStmt:
    case StmtKind::ContinueStmt:
        return false;
    case StmtKind::BreakStmt:
        return true;
    case StmtKind::ExprStmt: {
        auto& exprStmt = llvm::cast<ExprStmt>(*block.back());
        auto* call = llvm::dyn_cast<CallExpr>(exprStmt.expr);
        return !call || !call->type || !call->type.isNeverType();
    }
    case StmtKind::IfStmt: {
        auto& ifStmt = llvm::cast<IfStmt>(*block.back());
        return switchCaseMayFallThrough(ifStmt.thenBody) || switchCaseMayFallThrough(ifStmt.elseBody);
    }
    case StmtKind::SwitchStmt:
        return true;
    case StmtKind::CompoundStmt:
        return switchCaseMayFallThrough(llvm::cast<CompoundStmt>(*block.back()).body);
    default:
        return true;
    }
}

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
        if (!call->isMethodCall() && call->getFunctionName() == "assert" && !call->args.empty()) {
            for (size_t i = 0; i < call->args.size() && i < call->argParamIndices.size(); ++i) {
                if (call->argParamIndices[i] != 0) continue;
                if (auto* condition = llvm::dyn_cast<BoolLiteralExpr>(call->args[i].value)) {
                    return !condition->value;
                }
            }
            if (call->argParamIndices.empty()) {
                if (auto* condition = llvm::dyn_cast<BoolLiteralExpr>(call->args[0].value)) {
                    return !condition->value;
                }
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
        if (!llvm::all_of(switchStmt.cases, [&](SwitchCase& c) { return allPathsDiverge(c.stmts, nestLevel + 1); })) return false;
        if (switchStmt.defaultStmts.empty()) return switchStmt.coversAllEnumCases;
        return allPathsDiverge(switchStmt.defaultStmts, nestLevel + 1);
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
        collectAssignedNames(*whileStmt.condition, names);
        for (auto& bodyStmt : whileStmt.body)
            collectAssignedNames(bodyStmt, names);
        return;
    }
    case StmtKind::DoWhileStmt: {
        auto& doWhileStmt = llvm::cast<DoWhileStmt>(*stmt);
        collectAssignedNames(*doWhileStmt.condition, names);
        for (auto& bodyStmt : doWhileStmt.body)
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

void Typechecker::warnIfUnusedResult(const Expr& expr, Type type) const {
    // Anchor on the enclosing function so generic stdlib code instantiated from
    // user code stays exempt; currentModule is the instantiation site there.
    Module* module = currentFunction ? currentFunction->getModule() : currentModule;
    if (!options.warnUnusedResult || module->name == "std") return;
    if (!type || type.isVoid() || type.isNeverType()) return;
    WARN(expr.location, "unused result of type '" << type << "'");
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
        diagnoseClosureConversion(returnValueType, currentFunction->getReturnType(), stmt.location);
        ERROR(stmt.location, "mismatching return type '" << returnValueType << "', expected '" << currentFunction->getReturnType() << "'"
                                                         << narrowingHint(returnValueType, currentFunction->getReturnType()));
    }

    checkReturnPointerToLocal(stmt.value);
    setMoved(stmt.value, true);
    stmt.movedDecls.insert(movedDecls.begin(), movedDecls.end());
}

void Typechecker::typecheckVarStmt(VarStmt& stmt) {
    for (auto* decl : stmt.decls) {
        typecheckVarDecl(*decl);
    }
}

void Typechecker::typecheckIfStmt(IfStmt& ifStmt) {
    Type conditionType = typecheckExpr(*ifStmt.condition);
    typecheckImplicitlyBoolConvertibleExpr(conditionType, ifStmt.condition->location, ifStmt.condition->endLocation);
    currentControlStmts.push_back(&ifStmt);

    // A value moved in every branch is moved after the if statement. Moves from only one
    // branch are discarded: the value may still be live on the other path. An empty else
    // body moves nothing, so then-only moves never propagate.
    llvm::SmallPtrSet<Decl*, 32> thenMovedDecls, elseMovedDecls;
    NarrowMap outerNarrowings = narrowedTypes;
    NarrowMap thenNarrowings, elseNarrowings;
    llvm::SmallPtrSet<Decl*, 32> thenAssignedDecls, elseAssignedDecls;

    {
        Scope scope(currentFunction, &currentModule->symbolTable);
        llvm::SaveAndRestore saveMovedDecls(movedDecls);
        llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
        applyNarrowings(*ifStmt.condition, true);
        for (auto& stmt : ifStmt.thenBody) {
            typecheckStmt(stmt);
        }
        thenMovedDecls = movedDecls;
        thenNarrowings = narrowedTypes;
        thenAssignedDecls = definitelyAssignedDecls;
        narrowedTypes = outerNarrowings;
    }

    {
        Scope scope(currentFunction, &currentModule->symbolTable);
        llvm::SaveAndRestore saveMovedDecls(movedDecls);
        llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
        applyNarrowings(*ifStmt.condition, false);
        for (auto& stmt : ifStmt.elseBody) {
            typecheckStmt(stmt);
        }
        elseMovedDecls = movedDecls;
        elseNarrowings = narrowedTypes;
        elseAssignedDecls = definitelyAssignedDecls;
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
        definitelyAssignedDecls = elseAssignedDecls;
    } else if (elseDiverges && !thenDiverges) {
        narrowedTypes = thenNarrowings;
        definitelyAssignedDecls = thenAssignedDecls;
    } else if (!thenDiverges && !elseDiverges) {
        narrowedTypes = thenNarrowings;
        intersectNarrowings(elseNarrowings);
        llvm::SmallPtrSet<Decl*, 32> mergedAssignedDecls;
        for (auto* decl : thenAssignedDecls) {
            if (elseAssignedDecls.count(decl)) {
                mergedAssignedDecls.insert(decl);
            }
        }
        definitelyAssignedDecls = std::move(mergedAssignedDecls);
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
                value = makeAST<MemberExpr>(makeAST<VarExpr>(std::string(enumDecl->getName()), varExpr->location), std::string(varExpr->identifier),
                                            varExpr->location);
            }
        }
    }

    Type caseType = typecheckExpr(*value, false, conditionType);

    if (auto converted = convert(value, conditionType)) {
        value = converted;
    } else {
        ERROR(value->location, "case value type '" << caseType << "' doesn't match switch condition type '" << conditionType << "'");
    }

    auto* memberExpr = llvm::dyn_cast<MemberExpr>(value);
    auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
    if (!enumCase && !value->isConstant()) {
        ERROR(value->location, "case value must be constant");
    }
    return enumCase;
}

void Typechecker::typecheckSwitchCaseBinding(VarDecl* associatedValue, EnumCase* enumCase) {
    if (!associatedValue) return;
    // The parser has no enclosing declaration for bindings in switch expressions; adopt them here.
    associatedValue->parent = currentFunction;
    if (!enumCase) {
        ERROR(associatedValue->location, "only enum cases can bind associated values");
    }
    if (!enumCase->associatedType) {
        ERROR(associatedValue->location, "enum case '" << enumCase->getName() << "' has no associated values to bind");
    }
    Type associatedType = NOTNULL(enumCase->associatedType);
    if (associatedType.isTupleType() && associatedType.getTupleElements().size() == 1) {
        associatedType = associatedType.getTupleElements().front().type;
    }
    associatedValue->type = associatedType;
    typecheckVarDecl(*associatedValue);
    definitelyAssignedDecls.insert(associatedValue);
}

// Switch expressions lower directly to a switch instruction, so unlike switch statements
// they accept neither string conditions nor `case null` on optional pointers.
Type Typechecker::typecheckSwitchCondition(Expr*& condition) {
    Type conditionType = typecheckExpr(*condition);

    if (conditionType.isPointerType()) {
        Type pointeeType = conditionType.getPointee();
        // Automatically dereference pointers to switchable values. Enums with associated values are excluded;
        // they need the address for tag/associated-value access, so dereference those explicitly (e.g. `switch (*p)`).
        bool isPlainEnum = pointeeType.isEnumType() && !llvm::cast<EnumDecl>(pointeeType.getDecl())->hasAssociatedValues();
        if (pointeeType.isInteger() || pointeeType.isChar() || isPlainEnum) {
            if (auto dereferenced = convert(condition, pointeeType)) {
                condition = dereferenced;
                conditionType = pointeeType;
            }
        }
    }

    // Pointer-implemented optionals have no tag to switch on.
    bool isSwitchableEnum = conditionType.isEnumType() && !(conditionType.isOptionalType() && conditionType.isImplementedAsPointer());
    if (!conditionType.isInteger() && !conditionType.isChar() && !isSwitchableEnum) {
        ERROR(condition->location, "switch condition must have integer, char, or enum type, got '" << conditionType << "'");
    }
    return conditionType;
}

void Typechecker::typecheckSwitchStmt(SwitchStmt& stmt) {
    Type conditionType = typecheckExpr(*stmt.condition);
    bool hasNullCase = llvm::any_of(stmt.cases, [](const SwitchCase& switchCase) { return switchCase.value->isNullLiteralExpr(); });

    Type pointerType = conditionType;
    if (pointerType.isOptionalType() && pointerType.getWrappedType().isPointerType()) {
        pointerType = pointerType.getWrappedType();
    }

    // A `case null` on an optional pointer keeps the condition optional; codegen branches
    // on null first and switches on the dereferenced value, instead of trapping on null.
    bool nullRoutedOptional = false;

    if (pointerType.isPointerType()) {
        Type pointeeType = pointerType.getPointee();
        // Automatically dereference pointers to switchable values. Enums with associated values are excluded;
        // they need the address for tag/associated-value access, so dereference those explicitly (e.g. `switch (*p)`).
        bool isPlainEnum = pointeeType.isEnumType() && !llvm::cast<EnumDecl>(pointeeType.getDecl())->hasAssociatedValues();
        if (pointeeType.isInteger() || pointeeType.isChar() || isPlainEnum) {
            if (conditionType.isOptionalType() && hasNullCase) {
                nullRoutedOptional = true;
            } else {
                // Like other implicit unwraps, switching on an optional pointer unwraps it, trapping on null.
                if (conditionType.isOptionalType()) {
                    if (auto unwrapped = convert(stmt.condition, pointerType)) {
                        stmt.condition = unwrapped;
                        conditionType = pointerType;
                    }
                }
                if (auto dereferenced = convert(stmt.condition, pointeeType)) {
                    stmt.condition = dereferenced;
                    conditionType = pointeeType;
                }
            }
        }
    }

    // Pointer-implemented optionals have no tag to switch on.
    bool isSwitchableEnum = conditionType.isEnumType() && !(conditionType.isOptionalType() && conditionType.isImplementedAsPointer());
    bool isString = conditionType.isBasicType() && conditionType.getName() == "string";
    if (!conditionType.isInteger() && !conditionType.isChar() && !isSwitchableEnum && !nullRoutedOptional && !isString) {
        ERROR(stmt.condition->location, "switch condition must have integer, char, string, or enum type, got '" << conditionType << "'");
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

    // Cases of a null-routed optional switch match the dereferenced value; codegen unwraps before switching.
    Type caseTargetType = nullRoutedOptional ? conditionType.getWrappedType().getPointee() : conditionType;
    bool seenNullCase = false;
    std::vector<llvm::SmallPtrSet<Decl*, 32>> bodyAssignedDecls;

    for (auto& switchCase : stmt.cases) {
        // Null-routed switches match against the dereferenced type and accept `case null`,
        // so statements inline this instead of sharing typecheckSwitchCaseValue with expressions.
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

        Type caseType = typecheckExpr(*switchCase.value, false, caseTargetType);

        auto* memberExpr = llvm::dyn_cast<MemberExpr>(switchCase.value);
        auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;

        if (switchCase.value->isNullLiteralExpr()) {
            // Only optional-pointer conditions route null; anything else can't match it.
            // (The null literal adopts an optional condition type, so compare before converting.)
            if (!nullRoutedOptional) {
                ERROR(switchCase.value->location, "case value type 'null' doesn't match switch condition type '" << conditionType << "'");
            }
            if (seenNullCase) {
                ERROR(switchCase.value->location, "duplicate 'case null'");
            }
            seenNullCase = true;
        } else if (auto converted = convert(switchCase.value, caseTargetType)) {
            switchCase.value = converted;
            // Conversions can wrap the value, hiding the enum case from the checks below.
            memberExpr = llvm::dyn_cast<MemberExpr>(switchCase.value);
            enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
        } else {
            ERROR(switchCase.value->location, "case value type '" << caseType << "' doesn't match switch condition type '" << caseTargetType << "'");
        }

        if (!nullRoutedOptional && conditionType.isOptionalType() && !conditionType.getWrappedType().isPointerType() && !enumCase
            && caseType != conditionType) {
            // Value-optional conditions (e.g. int?) only match enum cases (Some/None); a wrapped
            // value has no case representation, so don't silently wrap to the optional type.
            ERROR(switchCase.value->location, "case value type '" << caseType << "' doesn't match switch condition type '" << conditionType << "'");
        }

        if (!enumCase && !switchCase.value->isConstant()) {
            ERROR(switchCase.value->location, "case value must be constant");
        }

        Scope scope(nullptr, &currentModule->symbolTable);
        NarrowMap outerNarrowings = narrowedTypes;
        llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);

        typecheckSwitchCaseBinding(switchCase.associatedValue, enumCase);

        for (auto& caseStmt : switchCase.stmts) {
            typecheckStmt(caseStmt);
        }
        narrowedTypes = outerNarrowings;
        if (switchCaseMayFallThrough(switchCase.stmts)) {
            bodyAssignedDecls.push_back(definitelyAssignedDecls);
        }
    }

    {
        Scope scope(nullptr, &currentModule->symbolTable);
        NarrowMap outerNarrowings = narrowedTypes;
        llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
        for (auto& defaultStmt : stmt.defaultStmts) {
            typecheckStmt(defaultStmt);
        }
        narrowedTypes = outerNarrowings;
        if (!stmt.defaultStmts.empty() && switchCaseMayFallThrough(stmt.defaultStmts)) {
            bodyAssignedDecls.push_back(definitelyAssignedDecls);
        }
    }

    dropNarrowingsForNames(bodyAssignedNames);

    currentControlStmts.pop_back();

    stmt.coversAllEnumCases = coversAllEnumCases(stmt, conditionType);
    if ((!stmt.defaultStmts.empty() || stmt.coversAllEnumCases) && !bodyAssignedDecls.empty()) {
        definitelyAssignedDecls = bodyAssignedDecls.front();
        for (auto& body : llvm::ArrayRef(bodyAssignedDecls).drop_front()) {
            for (auto* decl : llvm::to_vector(definitelyAssignedDecls)) {
                if (!body.count(decl)) {
                    definitelyAssignedDecls.erase(decl);
                }
            }
        }
    }
    warnAboutUnhandledEnumCases(stmt, conditionType);
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

    std::vector<llvm::SmallPtrSet<Decl*, 32>> armAssignedDecls;

    for (auto& arm : expr.arms) {
        auto* enumCase = typecheckSwitchCaseValue(arm.value, conditionType);

        Scope scope(nullptr, &currentModule->symbolTable);
        NarrowMap outerNarrowings = narrowedTypes;
        llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);

        typecheckSwitchCaseBinding(arm.associatedValue, enumCase);
        typecheckExpr(*arm.expr, false, expectedType);
        narrowedTypes = outerNarrowings;
        if (!arm.expr->type.isNeverType()) {
            armAssignedDecls.push_back(definitelyAssignedDecls);
        }
    }

    if (expr.defaultExpr) {
        NarrowMap outerNarrowings = narrowedTypes;
        llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
        typecheckExpr(*expr.defaultExpr, false, expectedType);
        narrowedTypes = outerNarrowings;
        if (!expr.defaultExpr->type.isNeverType()) {
            armAssignedDecls.push_back(definitelyAssignedDecls);
        }
    }

    if (!armAssignedDecls.empty()) {
        definitelyAssignedDecls = armAssignedDecls.front();
        for (auto& arm : llvm::ArrayRef(armAssignedDecls).drop_front()) {
            for (auto* decl : llvm::to_vector(definitelyAssignedDecls)) {
                if (!arm.count(decl)) {
                    definitelyAssignedDecls.erase(decl);
                }
            }
        }
    }

    if (!expr.defaultExpr) {
        if (!conditionType.isEnumType()) {
            ERROR(expr.location, "switch expression on '" << conditionType << "' must have a default case");
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
            ERROR(expr.location, "switch expression must handle all cases of enum '" << enumDecl->getName() << "' (missing: " << missing << ")");
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
            Type other = armIsNull ? resultType : armType;
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
            ERROR(expr.location, "incompatible arm types ('" << resultType << "' and '" << armType << "')");
        } else if (auto converted = convert(*armExpr, resultType)) {
            *armExpr = converted;
        } else {
            for (Expr** joined : joinedArms) {
                if (auto upgraded = convert(*joined, armType)) {
                    *joined = upgraded;
                } else {
                    ERROR(expr.location, "incompatible arm types ('" << resultType << "' and '" << armType << "')");
                }
            }
            resultType = armType;
        }
        joinedArms.push_back(armExpr);
    }

    if (resultType.isVoid()) {
        ERROR(expr.location, "switch expression arms must produce a value; use a switch statement for side effects");
    }
    return resultType;
}

void Typechecker::warnAboutUnhandledEnumCases(const SwitchStmt& stmt, Type conditionType) const {
    if (!stmt.defaultStmts.empty()) return;

    auto handledCases = getHandledEnumCases(stmt, conditionType);
    if (!handledCases) return;

    // Don't warn when over half of the cases are missing; partial matching is then assumed intentional.
    auto* enumDecl = llvm::cast<EnumDecl>(conditionType.getDecl());
    size_t totalCases = enumDecl->cases.size();
    size_t missingCases = totalCases - handledCases->size();
    if (missingCases == 0 || missingCases * 2 > totalCases) return;

    for (auto& enumCase : enumDecl->cases) {
        if (!handledCases->contains(&enumCase)) {
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
        typecheckImplicitlyBoolConvertibleExpr(conditionType, forStmt.condition->location, forStmt.condition->endLocation);
    }

    // The body and increment may not execute, so assignments there don't hold after the loop.
    llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
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

void Typechecker::typecheckDoWhileStmt(DoWhileStmt& doWhileStmt) {
    // Like for statements, assignments in the condition or body execute on later
    // iterations, so narrowings for variables assigned there don't hold after the loop.
    // Unlike while, the body runs before the first check, so the condition's
    // narrowings don't apply to the body.
    llvm::StringSet<> assignedNames;
    collectAssignedNames(*doWhileStmt.condition, assignedNames);
    for (auto& stmt : doWhileStmt.body)
        collectAssignedNames(stmt, assignedNames);
    NarrowMap outerNarrowings = narrowedTypes;
    dropNarrowingsForNames(assignedNames);

    // The body runs before the first check, so unlike while loops its assignments hold after.
    currentControlStmts.push_back(&doWhileStmt);

    for (auto& stmt : doWhileStmt.body) {
        typecheckStmt(stmt);
    }

    currentControlStmts.pop_back();

    Type conditionType = typecheckExpr(*doWhileStmt.condition);
    typecheckImplicitlyBoolConvertibleExpr(conditionType, doWhileStmt.condition->location, doWhileStmt.condition->endLocation);

    narrowedTypes = outerNarrowings;
    dropNarrowingsForNames(assignedNames);
}

void Typechecker::typecheckBreakStmt(BreakStmt& breakStmt) {
    if (llvm::none_of(currentControlStmts, [](const Stmt* stmt) { return stmt->isBreakable(); })) {
        ERROR(breakStmt.location, "'break' is only allowed inside 'while', 'do-while', 'for', and 'switch' statements");
    }
}

void Typechecker::typecheckContinueStmt(ContinueStmt& continueStmt) {
    if (llvm::none_of(currentControlStmts, [](const Stmt* stmt) { return stmt->isContinuable(); })) {
        ERROR(continueStmt.location, "'continue' is only allowed inside 'while', 'do-while', and 'for' statements");
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
            // Deferred expressions run at scope exit, when narrowings established here may no longer hold.
            llvm::SaveAndRestore saveNarrowings(narrowedTypes, NarrowMap{});
            llvm::SaveAndRestore saveAssignedDecls(definitelyAssignedDecls);
            auto& expr = *llvm::cast<DeferStmt>(stmt)->expr;
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
