#include "irgen.h"

using namespace delta;

void IRGenerator::codegenReturnStmt(const ReturnStmt& stmt) {
    // TODO: Emit deferred expressions and deinitializer calls after the evaluation of the return
    // value, but before emitting the return instruction. The return value expression may depend on
    // the values that the deferred expressions and/or deinitializer calls could deallocate.
    codegenDeferredExprsAndDeinitCallsForReturn();

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
    auto* alloca = createEntryBlockAlloca(stmt.getDecl().getType(), &stmt.getDecl(), nullptr, stmt.getDecl().getName());
    auto* initializer = stmt.getDecl().getInitializer();

    if (auto* callExpr = llvm::dyn_cast<CallExpr>(initializer)) {
        if (callExpr->getCalleeDecl()) {
            if (auto* initDecl = llvm::dyn_cast<InitDecl>(callExpr->getCalleeDecl())) {
                if (initDecl->getTypeDecl()->getType() == stmt.getDecl().getType().asImmutable()) {
                    codegenCallExpr(*callExpr, alloca);
                    return;
                }
            }
        }
    }

    if (!initializer->isUndefinedLiteralExpr()) {
        builder.CreateStore(codegenExprForPassing(*initializer, alloca->getAllocatedType()), alloca);
    }
}

void IRGenerator::codegenBlock(llvm::ArrayRef<std::unique_ptr<Stmt>> stmts, llvm::BasicBlock* destination,
                               llvm::BasicBlock* continuation) {
    builder.SetInsertPoint(destination);

    beginScope();
    for (const auto& stmt : stmts) {
        codegenStmt(*stmt);
        if (stmt->isReturnStmt() || stmt->isBreakStmt()) break;
    }
    endScope();

    llvm::BasicBlock* insertBlock = builder.GetInsertBlock();
    if (insertBlock->empty() ||
        (!llvm::isa<llvm::ReturnInst>(insertBlock->back()) && !llvm::isa<llvm::BranchInst>(insertBlock->back())))
        builder.CreateBr(continuation);
}

void IRGenerator::codegenIfStmt(const IfStmt& ifStmt) {
    auto* condition = codegenExpr(ifStmt.getCondition());
    if (condition->getType()->isPointerTy()) {
        condition = codegenImplicitNullComparison(condition);
    }
    auto* function = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(ctx, "then", function);
    auto* elseBlock = llvm::BasicBlock::Create(ctx, "else", function);
    auto* endIfBlock = llvm::BasicBlock::Create(ctx, "endif", function);
    builder.CreateCondBr(condition, thenBlock, elseBlock);
    codegenBlock(ifStmt.getThenBody(), thenBlock, endIfBlock);
    codegenBlock(ifStmt.getElseBody(), elseBlock, endIfBlock);
    builder.SetInsertPoint(endIfBlock);
}

void IRGenerator::codegenSwitchStmt(const SwitchStmt& switchStmt) {
    auto* condition = codegenExpr(switchStmt.getCondition());
    auto* function = builder.GetInsertBlock()->getParent();
    auto* insertBlockBackup = builder.GetInsertBlock();

    auto cases = map(switchStmt.getCases(), [&](const SwitchCase& switchCase) {
        auto* value = codegenExpr(*switchCase.getValue());
        auto* block = llvm::BasicBlock::Create(ctx, "", function);
        return std::make_pair(llvm::cast<llvm::ConstantInt>(value), block);
    });

    builder.SetInsertPoint(insertBlockBackup);
    auto* defaultBlock = llvm::BasicBlock::Create(ctx, "default", function);
    auto* end = llvm::BasicBlock::Create(ctx, "endswitch", function);
    breakTargets.push_back(end);
    auto* switchInst = builder.CreateSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (auto& switchCase : switchStmt.getCases()) {
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        codegenBlock(switchCase.getStmts(), block, end);
        switchInst->addCase(value, block);
        ++casesIterator;
    }

    codegenBlock(switchStmt.getDefaultStmts(), defaultBlock, end);
    breakTargets.pop_back();
    builder.SetInsertPoint(end);
}

void IRGenerator::codegenWhileStmt(const WhileStmt& whileStmt) {
    auto* increment = whileStmt.getIncrement();
    auto* function = builder.GetInsertBlock()->getParent();
    auto* condition = llvm::BasicBlock::Create(ctx, "while", function);
    auto* body = llvm::BasicBlock::Create(ctx, "body", function);
    auto* afterBody = increment ? llvm::BasicBlock::Create(ctx, "loop.increment", function) : condition;
    auto* end = llvm::BasicBlock::Create(ctx, "endwhile", function);

    breakTargets.push_back(end);
    continueTargets.push_back(afterBody);
    builder.CreateBr(condition);

    builder.SetInsertPoint(condition);
    auto* conditionValue = codegenExpr(whileStmt.getCondition());
    if (conditionValue->getType()->isPointerTy()) {
        conditionValue = codegenImplicitNullComparison(conditionValue);
    }
    builder.CreateCondBr(conditionValue, body, end);
    codegenBlock(whileStmt.getBody(), body, afterBody);

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
            codegenExpr(llvm::cast<ExprStmt>(stmt).getExpr());
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
            codegenWhileStmt(llvm::cast<WhileStmt>(stmt));
            break;
        case StmtKind::ForStmt:
            llvm_unreachable("ForStmt should be lowered into a WhileStmt");
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
