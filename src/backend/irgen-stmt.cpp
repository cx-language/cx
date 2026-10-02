#include "irgen.h"
#include "../ast/module.h"
#include "../driver/driver.h"
#include <llvm/Support/SaveAndRestore.h>

using namespace cx;

void IRGenerator::emitLeakCheckIfNeeded() {
    if (options.mode != BuildMode::Debug || options.noLeakCheck) return;
    auto* stdlib = Module::getStdlibModule();
    if (!stdlib) return;
    auto* decl = llvm::dyn_cast_or_null<FunctionDecl>(stdlib->symbolTable.findOne("checkLeaks"));
    if (!decl) return;
    checkImplicitCalleeIsChecked(*decl, "checkLeaks");
    createCall(getFunction(*decl), {}, nullptr);
}

void IRGenerator::emitReturnStmt(const ReturnStmt& stmt) {
    // Evaluate the return value first: it may depend on values that the deferred
    // expressions and/or destructor calls deallocate.
    Value* returnValue = nullptr;
    if (stmt.value) {
        returnValue = emitExprForPassing(*stmt.value, insertBlock->parent->returnType);
    }

    // Returning abandons every enclosing statement, so all pending temporaries die
    // here, before the terminator; the emitStmt frame then pops an empty scope.
    destroyAllTempScopes();

    emitDeferredExprsAndDestructorCallsForReturn(&stmt.movedDecls);

    if (llvm::cast<FunctionDecl>(currentDecl)->isEntryPoint) emitLeakCheckIfNeeded();

    // A void function has no value to return; a void-typed return value was
    // already evaluated above for its side effects (`return f();`).
    if (stmt.value && !insertBlock->parent->returnType->isVoid()) {
        createReturn(returnValue);
    } else {
        createReturn(llvm::cast<FunctionDecl>(currentDecl)->isEntryPoint ? createConstantInt(Type::getInt32(), 0) : nullptr);
    }
}

void IRGenerator::emitBlock(llvm::ArrayRef<Stmt*> stmts, BasicBlock* continuation) {
    beginScope();
    emitStmts(stmts);
    endScope();

    if (insertBlock->body.empty() || !insertBlock->body.back()->isTerminator()) {
        createBr(continuation);
    }
}

template<typename EmitThen, typename EmitElse> static void emitIfDiamond(IRGenerator& ir, Value* condition, EmitThen&& emitThen, EmitElse&& emitElse) {
    auto* function = ir.insertBlock->parent;
    auto* thenBlock = new BasicBlock("if.then", function);
    auto* elseBlock = new BasicBlock("if.else", function);
    auto* endIfBlock = new BasicBlock("if.end", function);
    ir.createCondBr(condition, thenBlock, elseBlock);

    ir.setInsertPoint(thenBlock);
    emitThen(endIfBlock);
    ir.setInsertPoint(elseBlock);
    emitElse(endIfBlock);
    ir.setInsertPoint(endIfBlock);
}

void IRGenerator::emitIfStmt(const IfStmt& ifStmt) {
    // `if s is Case name` binds the payload for the then-branch.
    if (ifStmt.isBinding) {
        auto& isExpr = llvm::cast<BinaryExpr>(*ifStmt.condition);
        Value* enumValue = nullptr;
        Value* tag = emitExprOrEnumTag(isExpr.getLHS(), &enumValue);
        Value* caseTag = emitExprOrEnumTag(isExpr.getRHS(), nullptr);
        auto* condition = createBinaryOp(Token::Equal, tag, caseTag, &isExpr);

        emitIfDiamond(
            *this, condition,
            [&](BasicBlock* endIfBlock) {
                bindBorrowedEnumPayload(enumValue, ifStmt.isBinding);
                emitBlock(ifStmt.thenBody, endIfBlock);
            },
            [&](BasicBlock* endIfBlock) { emitBlock(ifStmt.elseBody, endIfBlock); });
        return;
    }

    // FIXME: Lower implicit null checks such as `if (ptr)` and `if (!ptr)` to null comparisons.
    auto* condition = emitBoolConvertibleOperand(*ifStmt.condition);

    emitIfDiamond(
        *this, condition, [&](BasicBlock* endIfBlock) { emitBlock(ifStmt.thenBody, endIfBlock); },
        [&](BasicBlock* endIfBlock) { emitBlock(ifStmt.elseBody, endIfBlock); });
}

// Pushes a break target for the lifetime of a switch or loop. Destruction order
// matches the previous explicit pops: the targets are gone before the function
// returns, and nothing after the old pop site reads them.
struct BreakTargetScope {
    IRGenerator& ir;
    explicit BreakTargetScope(IRGenerator& ir, BasicBlock* end) : ir(ir) {
        ir.breakTargets.push_back(end);
        ir.breakTempScopeDepths.push_back(ir.tempScopes.size());
    }
    BreakTargetScope(const BreakTargetScope&) = delete;
    ~BreakTargetScope() {
        ir.breakTargets.pop_back();
        ir.breakTempScopeDepths.pop_back();
    }
};

struct LoopTargetScope {
    IRGenerator& ir;
    LoopTargetScope(IRGenerator& ir, BasicBlock* end, BasicBlock* continueTarget) : ir(ir) {
        ir.breakTargets.push_back(end);
        ir.continueTargets.push_back(continueTarget);
        ir.breakTempScopeDepths.push_back(ir.tempScopes.size());
        ir.continueTempScopeDepths.push_back(ir.tempScopes.size());
    }
    LoopTargetScope(const LoopTargetScope&) = delete;
    ~LoopTargetScope() {
        ir.breakTargets.pop_back();
        ir.continueTargets.pop_back();
        ir.breakTempScopeDepths.pop_back();
        ir.continueTempScopeDepths.pop_back();
    }
};

void IRGenerator::emitSwitchStmt(const SwitchStmt& switchStmt) {
    if (switchStmt.condition->type.isString()) {
        emitStringSwitchStmt(switchStmt);
        return;
    }

    Value* enumValue = nullptr;
    Value* condition = emitExprOrEnumTag(*switchStmt.condition, &enumValue);

    auto* function = insertBlock->parent;
    auto* insertBlockBackup = insertBlock;
    auto caseIndex = 0;

    auto cases = map(switchStmt.cases, [&](const SwitchCase& switchCase) {
        auto* value = emitExprOrEnumTag(*switchCase.value, nullptr);
        auto* block = new BasicBlock("switch.case." + std::to_string(caseIndex++), function);
        return std::make_pair(value, block);
    });

    setInsertPoint(insertBlockBackup);
    auto* defaultBlock = new BasicBlock("switch.default", function);
    auto* end = new BasicBlock("switch.end", function);
    BreakTargetScope breakScope(*this, end);
    auto* switchInst = createSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (auto& switchCase : switchStmt.cases) {
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        setInsertPoint(block);

        if (auto* associatedValue = switchCase.associatedValue) {
            bindBorrowedEnumPayload(enumValue, associatedValue);
        }

        emitBlock(switchCase.stmts, end);
        switchInst->cases.emplace_back(value, block);
        ++casesIterator;
    }

    setInsertPoint(defaultBlock);
    bool checkEmitted = false;
    if (switchStmt.defaultStmts.empty()) {
        llvm::SmallVector<Expr*, 8> caseValues;
        for (auto& switchCase : switchStmt.cases) {
            caseValues.push_back(switchCase.value);
        }
        checkEmitted = emitEnumSwitchCheck(*switchStmt.condition, caseValues, *switchInst, end);
    }
    if (!checkEmitted) {
        emitBlock(switchStmt.defaultStmts, end);
    }

    setInsertPoint(end);
}

// Emits the safety check for an enum switch without a default: values that are not a tag
// of the enum trap with an error in safe modes, and are assumed impossible in release-fast
// mode. Returns false when the check doesn't apply, in which case the caller falls through
// to the end block as before.
bool IRGenerator::emitEnumSwitchCheck(const Expr& condition, llvm::ArrayRef<Expr*> caseValues, SwitchInst& switchInst, BasicBlock* end) {
    auto* enumDecl = llvm::dyn_cast_or_null<EnumDecl>(condition.type.getDecl());
    if (!enumDecl) return false;

    llvm::SmallVector<llvm::APSInt, 8> handledTags;
    for (auto* value : caseValues) {
        auto* memberExpr = llvm::dyn_cast<MemberExpr>(value);
        auto* enumCase = memberExpr ? llvm::dyn_cast<EnumCase>(memberExpr->decl) : nullptr;
        if (!enumCase || enumCase->getEnumDecl() != enumDecl) return false;
        auto* tag = llvm::dyn_cast<IntLiteralExpr>(enumCase->value);
        if (!tag) return false;
        handledTags.push_back(tag->value);
    }

    // Unhandled but valid values fall through to the end block; only values that are
    // not a tag of the enum reach the default block.
    for (auto& enumCase : enumDecl->cases) {
        auto* tag = llvm::cast<IntLiteralExpr>(enumCase.value);
        if (llvm::none_of(handledTags, [&](auto& handled) { return handled == tag->value; })) {
            switchInst.cases.emplace_back(emitExpr(*enumCase.value), end);
        }
    }

    if (options.mode == BuildMode::ReleaseFast) {
        createUnreachable();
    } else {
        std::string message = "invalid value in switch over enum '" + enumDecl->getName().str() + "'";
        emitAbortWithMessage(message, condition.location);
    }
    return true;
}

// Strings can't use the switch instruction, so compare against each case value in turn.
void IRGenerator::emitStringSwitchStmt(const SwitchStmt& switchStmt) {
    Function* stringEquals = nullptr;
    for (auto* decl : Module::getStdlibModule()->symbolTable.findInTopLevelScope("==")) {
        auto* functionDecl = llvm::dyn_cast<FunctionDecl>(decl);
        if (!functionDecl) continue;
        auto params = functionDecl->getParams();
        if (params.size() == 2 && params[0].type.isString() && params[1].type.isString()) {
            checkImplicitCalleeIsChecked(*functionDecl, "==");
            stringEquals = getFunction(*functionDecl);
            break;
        }
    }
    ASSERT(stringEquals);

    auto* stringType = getIRType(switchStmt.condition->type);
    Value* condition = emitExprForPassing(*switchStmt.condition, stringType);

    auto* function = insertBlock->parent;
    auto* defaultBlock = new BasicBlock("switch.default", function);
    auto* end = new BasicBlock("switch.end", function);
    BreakTargetScope breakScope(*this, end);

    std::vector<BasicBlock*> caseBlocks;
    for (size_t i = 0; i < switchStmt.cases.size(); ++i) {
        auto* caseBlock = new BasicBlock("switch.case." + std::to_string(i), function);
        auto* nextBlock = i + 1 < switchStmt.cases.size() ? new BasicBlock("switch.test." + std::to_string(i + 1), function) : defaultBlock;
        // Case values are constants, so no temporaries can be constructed here.
        Value* caseValue = emitExprForPassing(*switchStmt.cases[i].value, stringType);
        createCondBr(createCall(stringEquals, {condition, caseValue}, nullptr), caseBlock, nextBlock);
        caseBlocks.push_back(caseBlock);
        setInsertPoint(nextBlock);
    }
    if (switchStmt.cases.empty()) {
        createBr(defaultBlock);
    }

    for (size_t i = 0; i < switchStmt.cases.size(); ++i) {
        setInsertPoint(caseBlocks[i]);
        emitBlock(switchStmt.cases[i].stmts, end);
    }

    setInsertPoint(defaultBlock);
    emitBlock(switchStmt.defaultStmts, end);

    setInsertPoint(end);
}

Value* IRGenerator::emitLoopConditionValue(const Expr& condition) {
    beginTempScope();
    auto* conditionValue = emitExpr(condition);
    endTempScope();
    return lowerImplicitBool(conditionValue, condition);
}

void IRGenerator::emitDoWhileStmt(const DoWhileStmt& doWhileStmt) {
    ASSERT(doWhileStmt.condition);
    auto* function = insertBlock->parent;
    auto* body = new BasicBlock("loop.body", function);
    auto* condition = new BasicBlock("loop.condition");
    auto* end = new BasicBlock("loop.end", function);

    LoopTargetScope loopScope(*this, end, condition);
    createBr(body);

    setInsertPoint(body);
    emitBlock(doWhileStmt.body, condition);

    setInsertPoint(condition);
    createCondBr(emitLoopConditionValue(*doWhileStmt.condition), body, end);

    setInsertPoint(end);
}

void IRGenerator::emitForStmt(const ForStmt& forStmt) {
    if (forStmt.variable) {
        beginTempScope();
        for (auto* decl : forStmt.variable->decls) {
            emitVarDecl(*decl);
        }
        endTempScope();
    }

    auto& increments = forStmt.increments;
    auto* function = insertBlock->parent;
    auto* condition = new BasicBlock("loop.condition", function);
    auto* body = new BasicBlock("loop.body", function);
    auto* afterBody = !increments.empty() ? new BasicBlock("loop.increment", function) : condition;
    auto* end = new BasicBlock("loop.end", function);

    LoopTargetScope loopScope(*this, end, afterBody);
    createBr(condition);

    setInsertPoint(condition);
    if (forStmt.condition) {
        createCondBr(emitLoopConditionValue(*forStmt.condition), body, end);
    } else {
        createBr(body);
    }

    setInsertPoint(body);
    emitBlock(forStmt.body, afterBody);

    if (!increments.empty()) {
        setInsertPoint(afterBody);
        beginTempScope();
        for (auto* increment : increments) {
            emitExpr(*increment);
        }
        endTempScope();
        createBr(condition);
    }

    setInsertPoint(end);
}

void IRGenerator::emitBreakStmt(const BreakStmt&) {
    ASSERT(!breakTargets.empty());
    unwindTempScopesTo(breakTempScopeDepths.back());
    createBr(breakTargets.back());
}

void IRGenerator::emitContinueStmt(const ContinueStmt&) {
    ASSERT(!continueTargets.empty());
    unwindTempScopesTo(continueTempScopeDepths.back());
    createBr(continueTargets.back());
}

void IRGenerator::emitCompoundStmt(const CompoundStmt& compoundStmt) {
    beginScope();
    emitStmts(compoundStmt.body);
    endScope();
}

void IRGenerator::emitStmt(const Stmt& stmt) {
    llvm::SaveAndRestore saveChecks(disabledChecks, disabledChecks | stmt.disabledChecks);
    beginTempScope();
    switch (stmt.kind) {
    case StmtKind::ReturnStmt:
        emitReturnStmt(llvm::cast<ReturnStmt>(stmt));
        break;
    case StmtKind::VarStmt:
        for (auto* decl : llvm::cast<VarStmt>(stmt).decls) {
            emitVarDecl(*decl);
        }
        break;
    case StmtKind::ExprStmt:
        emitPlainExpr(*llvm::cast<ExprStmt>(stmt).expr);
        break;
    case StmtKind::DeferStmt:
        deferEvaluationOf(*llvm::cast<DeferStmt>(stmt).expr);
        break;
    case StmtKind::IfStmt:
        emitIfStmt(llvm::cast<IfStmt>(stmt));
        break;
    case StmtKind::SwitchStmt:
        emitSwitchStmt(llvm::cast<SwitchStmt>(stmt));
        break;
    case StmtKind::WhileStmt:
        llvm_unreachable("WhileStmt should be lowered into a ForStmt");
        break;
    case StmtKind::DoWhileStmt:
        emitDoWhileStmt(llvm::cast<DoWhileStmt>(stmt));
        break;
    case StmtKind::ForStmt:
        emitForStmt(llvm::cast<ForStmt>(stmt));
        break;
    case StmtKind::ForEachStmt:
        llvm_unreachable("ForEachStmt should be lowered into a ForStmt");
        break;
    case StmtKind::BreakStmt:
        emitBreakStmt(llvm::cast<BreakStmt>(stmt));
        break;
    case StmtKind::ContinueStmt:
        emitContinueStmt(llvm::cast<ContinueStmt>(stmt));
        break;
    case StmtKind::CompoundStmt:
        emitCompoundStmt(llvm::cast<CompoundStmt>(stmt));
        break;
    }
    endTempScope();
}

void IRGenerator::emitStmts(llvm::ArrayRef<Stmt*> stmts) {
    for (Stmt* stmt : stmts) {
        emitStmt(*stmt);
        if (stmt->isReturnStmt() || stmt->isBreakStmt() || stmt->isContinueStmt()) break;
    }
}
