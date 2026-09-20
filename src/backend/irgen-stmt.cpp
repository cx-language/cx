#include "irgen.h"
#include "../ast/module.h"
#include "../driver/driver.h"

using namespace cx;

void IRGenerator::emitReturnStmt(const ReturnStmt& stmt) {
    // Evaluate the return value first: it may depend on values that the deferred
    // expressions and/or destructor calls deallocate.
    Value* returnValue = nullptr;
    if (stmt.value) {
        returnValue = emitExprForPassing(*stmt.value, insertBlock->parent->returnType);
    }

    emitDeferredExprsAndDestructorCallsForReturn();

    if (stmt.value) {
        createReturn(returnValue);
    } else {
        createReturn(currentDecl->isMain() ? createConstantInt(Type::getInt(), 0) : nullptr);
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

void IRGenerator::emitIfStmt(const IfStmt& ifStmt) {
    auto* condition = emitExpr(*ifStmt.condition);

    // FIXME: Lower implicit null checks such as `if (ptr)` and `if (!ptr)` to null comparisons.
    if (condition->getType()->isPointerType()) {
        condition = emitImplicitNullComparison(condition);
    } else if (ifStmt.condition->type.isOptionalType() && !ifStmt.condition->type.getWrappedType().isPointerType()) {
        condition = emitOptionalHasValueTest(condition);
    }

    auto* function = insertBlock->parent;
    auto* thenBlock = new BasicBlock("if.then", function);
    auto* elseBlock = new BasicBlock("if.else", function);
    auto* endIfBlock = new BasicBlock("if.end", function);
    createCondBr(condition, thenBlock, elseBlock);

    setInsertPoint(thenBlock);
    emitBlock(ifStmt.thenBody, endIfBlock);

    setInsertPoint(elseBlock);
    emitBlock(ifStmt.elseBody, endIfBlock);

    setInsertPoint(endIfBlock);
}

void IRGenerator::emitSwitchStmt(const SwitchStmt& switchStmt) {
    if (switchStmt.condition->type.isBasicType() && switchStmt.condition->type.getName() == "string") {
        emitStringSwitchStmt(switchStmt);
        return;
    }

    Value* enumValue = nullptr;
    Value* condition = emitExprOrEnumTag(*switchStmt.condition, &enumValue);

    auto* function = insertBlock->parent;
    auto* insertBlockBackup = insertBlock;
    auto caseIndex = 0;

    // A `case null` on an optional pointer keeps the condition optional. Switch instructions
    // can't match null, so branch on null first and switch on the dereferenced value.
    BasicBlock* nullCaseBlock = nullptr;
    Type conditionType = switchStmt.condition->type;
    if (conditionType.isOptionalType() && conditionType.getWrappedType().isPointerType()) {
        auto* switchBlock = new BasicBlock("switch.nonnull", function);
        nullCaseBlock = new BasicBlock("switch.case.null", function);
        createCondBr(emitImplicitNullComparison(condition, Token::Equal), nullCaseBlock, switchBlock);
        setInsertPoint(switchBlock);
        insertBlockBackup = switchBlock;
        condition = createLoad(condition);
    }

    auto cases = map(switchStmt.cases, [&](const SwitchCase& switchCase) {
        if (switchCase.value->isNullLiteralExpr()) {
            ASSERT(nullCaseBlock);
            return std::make_pair((Value*)nullptr, nullCaseBlock);
        }
        auto* value = emitExprOrEnumTag(*switchCase.value, nullptr);
        auto* block = new BasicBlock("switch.case." + std::to_string(caseIndex++), function);
        return std::make_pair(value, block);
    });

    setInsertPoint(insertBlockBackup);
    auto* defaultBlock = new BasicBlock("switch.default", function);
    auto* end = new BasicBlock("switch.end", function);
    breakTargets.push_back(end);
    auto* switchInst = createSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (auto& switchCase : switchStmt.cases) {
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        setInsertPoint(block);

        if (auto* associatedValue = switchCase.associatedValue) {
            auto type = associatedValue->type.getPointerTo();
            auto* associatedValuePtr = createCast(createGEP(enumValue, 1), type, associatedValue->getName());
            setLocalValue(associatedValuePtr, associatedValue);
        }

        emitBlock(switchCase.stmts, end);
        // The null case is routed by the null check, not the switch instruction.
        if (value) switchInst->cases.emplace_back(value, block);
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

    breakTargets.pop_back();
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
        if (params.size() == 2 && params[0].type.isBasicType() && params[0].type.getName() == "string" && params[1].type.isBasicType()
            && params[1].type.getName() == "string") {
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
    breakTargets.push_back(end);

    std::vector<BasicBlock*> caseBlocks;
    for (size_t i = 0; i < switchStmt.cases.size(); ++i) {
        auto* caseBlock = new BasicBlock("switch.case." + std::to_string(i), function);
        auto* nextBlock = i + 1 < switchStmt.cases.size() ? new BasicBlock("switch.test." + std::to_string(i + 1), function) : defaultBlock;
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

    breakTargets.pop_back();
    setInsertPoint(end);
}

Value* IRGenerator::emitLoopConditionValue(const Expr& condition) {
    auto* conditionValue = emitExpr(condition);
    if (conditionValue->getType()->isPointerType()) {
        conditionValue = emitImplicitNullComparison(conditionValue);
    } else if (condition.type.isOptionalType() && !condition.type.getWrappedType().isPointerType()) {
        conditionValue = emitOptionalHasValueTest(conditionValue);
    }
    return conditionValue;
}

void IRGenerator::emitDoWhileStmt(const DoWhileStmt& doWhileStmt) {
    ASSERT(doWhileStmt.condition);
    auto* function = insertBlock->parent;
    auto* body = new BasicBlock("loop.body", function);
    auto* condition = new BasicBlock("loop.condition");
    auto* end = new BasicBlock("loop.end", function);

    breakTargets.push_back(end);
    continueTargets.push_back(condition);
    createBr(body);

    setInsertPoint(body);
    emitBlock(doWhileStmt.body, condition);

    setInsertPoint(condition);
    createCondBr(emitLoopConditionValue(*doWhileStmt.condition), body, end);

    breakTargets.pop_back();
    continueTargets.pop_back();
    setInsertPoint(end);
}

void IRGenerator::emitForStmt(const ForStmt& forStmt) {
    if (forStmt.variable) {
        for (auto* decl : forStmt.variable->decls) {
            emitVarDecl(*decl);
        }
    }

    auto* increment = forStmt.increment;
    auto* function = insertBlock->parent;
    auto* condition = new BasicBlock("loop.condition", function);
    auto* body = new BasicBlock("loop.body", function);
    auto* afterBody = increment ? new BasicBlock("loop.increment", function) : condition;
    auto* end = new BasicBlock("loop.end", function);

    breakTargets.push_back(end);
    continueTargets.push_back(afterBody);
    createBr(condition);

    setInsertPoint(condition);
    if (forStmt.condition) {
        createCondBr(emitLoopConditionValue(*forStmt.condition), body, end);
    } else {
        createBr(body);
    }

    setInsertPoint(body);
    emitBlock(forStmt.body, afterBody);

    if (increment) {
        setInsertPoint(afterBody);
        emitExpr(*increment);
        createBr(condition);
    }

    breakTargets.pop_back();
    continueTargets.pop_back();
    setInsertPoint(end);
}

void IRGenerator::emitBreakStmt(const BreakStmt&) {
    ASSERT(!breakTargets.empty());
    createBr(breakTargets.back());
}

void IRGenerator::emitContinueStmt(const ContinueStmt&) {
    ASSERT(!continueTargets.empty());
    createBr(continueTargets.back());
}

void IRGenerator::emitCompoundStmt(const CompoundStmt& compoundStmt) {
    beginScope();
    emitStmts(compoundStmt.body);
    endScope();
}

void IRGenerator::emitStmt(const Stmt& stmt) {
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
}

void IRGenerator::emitStmts(llvm::ArrayRef<Stmt*> stmts) {
    for (Stmt* stmt : stmts) {
        emitStmt(*stmt);
        if (stmt->isReturnStmt() || stmt->isBreakStmt() || stmt->isContinueStmt()) break;
    }
}
