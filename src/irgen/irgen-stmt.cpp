#include "irgen.h"

using namespace delta;

void IRGenerator::codegenReturnStmt(const ReturnStmt& stmt) {
    // TODO: Emit deferred expressions and destructor calls after the evaluation of the return
    // value, but before emitting the return instruction. The return value expression may depend on
    // the values that the deferred expressions and/or destructor calls could deallocate.
    codegenDeferredExprsAndDestructorCallsForReturn();

    if (auto* returnValue = stmt.getReturnValue()) {
        builder.CreateRet(codegenExprForPassing(*returnValue, builder.getCurrentFunctionReturnType()));
    } else {
        if (!currentDecl->isMain()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        }
    }
}

void IRGenerator::codegenVarStmt(const VarStmt& stmt) {
    auto* alloca = createEntryBlockAlloca(getLLVMType(stmt.getDecl().getType()), nullptr, stmt.getDecl().getName());
    setLocalValue(alloca, &stmt.getDecl());
    auto* initializer = stmt.getDecl().getInitializer();

    if (auto* callExpr = llvm::dyn_cast<CallExpr>(initializer)) {
        if (callExpr->getCalleeDecl()) {
            if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(callExpr->getCalleeDecl())) {
                if (constructorDecl->getTypeDecl()->getType() == stmt.getDecl().getType()) {
                    codegenCallExpr(*callExpr, alloca);
                    return;
                }
            }
        }
    }

    if (!initializer->isUndefinedLiteralExpr()) {
        createStore(codegenExprForPassing(*initializer, alloca->getAllocatedType()), alloca);
    }
}

void IRGenerator::codegenBlock(llvm::ArrayRef<Stmt*> stmts, llvm::BasicBlock* continuation) {
    beginScope();
    for (const auto& stmt : stmts) {
        codegenStmt(*stmt);
        if (stmt->isReturnStmt() || stmt->isBreakStmt()) break;
    }
    endScope();

    llvm::BasicBlock* insertBlock = builder.GetInsertBlock();
    if (insertBlock->empty() || (!llvm::isa<llvm::ReturnInst>(insertBlock->back()) && !llvm::isa<llvm::BranchInst>(insertBlock->back()))) {
        builder.CreateBr(continuation);
    }
}

void IRGenerator::codegenIfStmt(const IfStmt& ifStmt) {
    auto* condition = codegenExpr(ifStmt.getCondition());
    if (condition->getType()->isPointerTy()) {
        condition = codegenImplicitNullComparison(condition);
    }
    auto* function = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(ctx, "if.then", function);
    auto* elseBlock = llvm::BasicBlock::Create(ctx, "if.else", function);
    auto* endIfBlock = llvm::BasicBlock::Create(ctx, "if.end", function);
    builder.CreateCondBr(condition, thenBlock, elseBlock);

    builder.SetInsertPoint(thenBlock);
    codegenBlock(ifStmt.getThenBody(), endIfBlock);

    builder.SetInsertPoint(elseBlock);
    codegenBlock(ifStmt.getElseBody(), endIfBlock);

    builder.SetInsertPoint(endIfBlock);
}

void IRGenerator::codegenSwitchStmt(const SwitchStmt& switchStmt) {
    llvm::Value* enumValue = nullptr;
    llvm::Value* condition = codegenExprOrEnumTag(switchStmt.getCondition(), &enumValue);

    auto* function = builder.GetInsertBlock()->getParent();
    auto* insertBlockBackup = builder.GetInsertBlock();
    auto caseIndex = 0;

    auto cases = map(switchStmt.getCases(), [&](const SwitchCase& switchCase) {
        auto* value = codegenExprOrEnumTag(*switchCase.getValue(), nullptr);
        auto* block = llvm::BasicBlock::Create(ctx, llvm::Twine("switch.case.", std::to_string(caseIndex++)), function);
        return std::make_pair(llvm::cast<llvm::ConstantInt>(value), block);
    });

    builder.SetInsertPoint(insertBlockBackup);
    auto* defaultBlock = llvm::BasicBlock::Create(ctx, "switch.default", function);
    auto* end = llvm::BasicBlock::Create(ctx, "switch.end", function);
    breakTargets.push_back(end);
    auto* switchInst = builder.CreateSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (auto& switchCase : switchStmt.getCases()) {
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        builder.SetInsertPoint(block);

        if (auto* associatedValue = switchCase.getAssociatedValue()) {
            auto* type = getLLVMType(associatedValue->getType())->getPointerTo();
            auto* associatedValuePtr = builder.CreatePointerCast(builder.CreateStructGEP(enumValue, 1), type, associatedValue->getName());
            setLocalValue(associatedValuePtr, associatedValue);
        }

        codegenBlock(switchCase.getStmts(), end);
        switchInst->addCase(value, block);
        ++casesIterator;
    }

    builder.SetInsertPoint(defaultBlock);
    codegenBlock(switchStmt.getDefaultStmts(), end);

    breakTargets.pop_back();
    builder.SetInsertPoint(end);
}

void IRGenerator::codegenForStmt(const ForStmt& forStmt) {
    if (forStmt.getVariable()) {
        codegenVarStmt(*forStmt.getVariable());
    }

    auto* increment = forStmt.getIncrement();
    auto* function = builder.GetInsertBlock()->getParent();
    auto* condition = llvm::BasicBlock::Create(ctx, "loop.condition", function);
    auto* body = llvm::BasicBlock::Create(ctx, "loop.body", function);
    auto* afterBody = increment ? llvm::BasicBlock::Create(ctx, "loop.increment", function) : condition;
    auto* end = llvm::BasicBlock::Create(ctx, "loop.end", function);

    breakTargets.push_back(end);
    continueTargets.push_back(afterBody);
    builder.CreateBr(condition);

    builder.SetInsertPoint(condition);
    auto* conditionValue = codegenExpr(*forStmt.getCondition());
    if (conditionValue->getType()->isPointerTy()) {
        conditionValue = codegenImplicitNullComparison(conditionValue);
    }
    builder.CreateCondBr(conditionValue, body, end);

    builder.SetInsertPoint(body);
    codegenBlock(forStmt.getBody(), afterBody);

    if (increment) {
        builder.SetInsertPoint(afterBody);
        codegenExpr(*increment);
        builder.CreateBr(condition);
    }

    breakTargets.pop_back();
    continueTargets.pop_back();
    builder.SetInsertPoint(end);
}

void IRGenerator::codegenBreakStmt(const BreakStmt&) {
    ASSERT(!breakTargets.empty());
    builder.CreateBr(breakTargets.back());
}

void IRGenerator::codegenContinueStmt(const ContinueStmt&) {
    ASSERT(!continueTargets.empty());
    builder.CreateBr(continueTargets.back());
}

void IRGenerator::codegenCompoundStmt(const CompoundStmt& compoundStmt) {
    beginScope();

    for (auto& stmt : compoundStmt.getBody()) {
        codegenStmt(*stmt);
    }

    endScope();
}

void IRGenerator::codegenStmt(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:
            codegenReturnStmt(llvm::cast<ReturnStmt>(stmt));
            break;
        case StmtKind::VarStmt:
            codegenVarStmt(llvm::cast<VarStmt>(stmt));
            break;
        case StmtKind::ExprStmt:
            codegenLvalueExpr(llvm::cast<ExprStmt>(stmt).getExpr());
            break;
        case StmtKind::DeferStmt:
            deferEvaluationOf(llvm::cast<DeferStmt>(stmt).getExpr());
            break;
        case StmtKind::IfStmt:
            codegenIfStmt(llvm::cast<IfStmt>(stmt));
            break;
        case StmtKind::SwitchStmt:
            codegenSwitchStmt(llvm::cast<SwitchStmt>(stmt));
            break;
        case StmtKind::WhileStmt:
            llvm_unreachable("WhileStmt should be lowered into a ForStmt");
            break;
        case StmtKind::ForStmt:
            codegenForStmt(llvm::cast<ForStmt>(stmt));
            break;
        case StmtKind::ForEachStmt:
            llvm_unreachable("ForEachStmt should be lowered into a ForStmt");
            break;
        case StmtKind::BreakStmt:
            codegenBreakStmt(llvm::cast<BreakStmt>(stmt));
            break;
        case StmtKind::ContinueStmt:
            codegenContinueStmt(llvm::cast<ContinueStmt>(stmt));
            break;
        case StmtKind::CompoundStmt:
            codegenCompoundStmt(llvm::cast<CompoundStmt>(stmt));
            break;
    }
}
