#include "irgen.h"
#include "../ast/module.h"

using namespace cx;

void IRGenScope::onScopeEnd() {
    for (const Expr* expr : reverse(deferredExprs)) {
        irGenerator->emitExpr(*expr);
    }

    for (auto& p : reverse(destructorsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        irGenerator->createDestructorCall(p.function, p.value);
    }
}

void IRGenScope::clear() {
    deferredExprs.clear();
    destructorsToCall.clear();
}

IRGenerator::IRGenerator() {
    scopes.push_back(IRGenScope(*this));
}

void IRGenerator::setLocalValue(Value* value, const VariableDecl* decl) {
    auto it = scopes.back().valuesByDecl.try_emplace(decl, value);
    ASSERT(it.second);

    if (decl) {
        deferDestructorCall(value, decl);
    }
}

Value* IRGenerator::getValueOrNull(const Decl* decl) {
    for (auto& scope : llvm::reverse(scopes)) {
        if (auto* value = scope.valuesByDecl.lookup(decl)) {
            return value;
        }
    }

    return nullptr;
}

Value* IRGenerator::getValue(const Decl* decl) {
    if (auto* value = getValueOrNull(decl)) {
        return value;
    }

    switch (decl->getKind()) {
        case DeclKind::VarDecl:
            return emitVarDecl(*llvm::cast<VarDecl>(decl));
        case DeclKind::FieldDecl:
            return emitMemberAccess(getThis(), llvm::cast<FieldDecl>(decl));
        case DeclKind::FunctionDecl:
            return getFunction(*llvm::cast<FunctionDecl>(decl));
        default:
            llvm_unreachable("all cases handled");
    }
}

Value* IRGenerator::getThis(IRType* targetType) {
    auto value = getValue(nullptr);

    // TODO: Handle this casting in a more general place?
    if (targetType && !value->getType()->equals(targetType)) {
        value = createCast(value, targetType);
    }

    return value;
}

void IRGenerator::beginScope() {
    scopes.push_back(IRGenScope(*this));
}

void IRGenerator::endScope() {
    scopes.back().onScopeEnd();
    scopes.pop_back();
}

void IRGenerator::deferEvaluationOf(const Expr& expr) {
    scopes.back().deferredExprs.push_back(&expr);
}

/// Returns a destructor that only calls the destructors of the member variables, or null if
/// no such destructor is needed because none of the member variables have destructors.
DestructorDecl* IRGenerator::getDefaultDestructor(TypeDecl& typeDecl) {
    ASSERT(!typeDecl.getDestructor());

    for (auto& field : typeDecl.getFields()) {
        if (field.getType().getDestructor()) {
            auto destructor = new DestructorDecl(typeDecl, typeDecl.getLocation());
            destructor->setBody({});
            return destructor;
        }
    }

    return nullptr;
}

void IRGenerator::deferDestructorCall(Value* receiver, const VariableDecl* decl) {
    ASSERT(decl->getType());
    Function* function = nullptr;

    if (auto* destructor = decl->getType().getDestructor()) {
        function = getFunction(*destructor);
    } else if (auto* typeDecl = decl->getType().getDecl()) {
        if (auto defaultDestructor = getDefaultDestructor(*typeDecl)) {
            function = getFunction(*defaultDestructor);
        }
    }

    if (function) {
        scopes.back().destructorsToCall.push_back({ function, receiver, decl });
    }
}

void IRGenerator::emitDeferredExprsAndDestructorCallsForReturn() {
    for (auto& scope : llvm::reverse(scopes)) {
        scope.onScopeEnd();
    }
    scopes.back().clear();
}

AllocaInst* IRGenerator::createEntryBlockAlloca(IRType* type, const llvm::Twine& name) {
    auto alloca = new AllocaInst { ValueKind::AllocaInst, type, name.str() };
    auto& entryBlock = currentFunction->body.front()->body;
    auto insertPosition = entryBlock.end();

    for (auto it = entryBlock.begin(), end = entryBlock.end(); it != end; ++it) {
        if (!llvm::isa<AllocaInst>(*it)) {
            insertPosition = it;
            break;
        }
    }

    entryBlock.insert(insertPosition, alloca);
    return alloca;
}

AllocaInst* IRGenerator::createTempAlloca(Value* value) {
    auto alloca = createEntryBlockAlloca(value->getType());
    createStore(value, alloca);
    return alloca;
}

Value* IRGenerator::createLoad(Value* value, const Expr* expr) {
    return insertBlock->add(new LoadInst { ValueKind::LoadInst, value, expr, value->getName() + ".load" });
}

void IRGenerator::createStore(Value* value, Value* pointer) {
    ASSERT(pointer->getType()->isPointerType());
    ASSERT(pointer->getType()->getPointee()->equals(value->getType()));
    insertBlock->add(new StoreInst { ValueKind::StoreInst, value, pointer });
}

Value* IRGenerator::createCall(Value* function, llvm::ArrayRef<Value*> args, const CallExpr* expr) {
    ASSERT(function->kind == ValueKind::Function || (function->getType()->isPointerType() && function->getType()->getPointee()->isFunctionType()));
    return insertBlock->add(new CallInst { ValueKind::CallInst, function, args, expr, "" });
}

Value* IRGenerator::emitAssignmentLHS(const Expr& lhs) {
    Value* value = emitLvalueExpr(lhs);

    // Don't call destructor for LHS when assigning to fields in constructor.
    if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(currentDecl)) {
        Decl* referencedDecl = nullptr;

        if (auto* varExpr = llvm::dyn_cast<VarExpr>(&lhs)) {
            referencedDecl = varExpr->getDecl();
        } else if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&lhs); memberExpr && memberExpr->getBaseExpr()->isThis()) {
            referencedDecl = memberExpr->getDecl();
        }

        if (auto* fieldDecl = llvm::dyn_cast_or_null<FieldDecl>(referencedDecl)) {
            if (fieldDecl->getParentDecl() == constructorDecl->getTypeDecl()) {
                return value;
            }
        }
    }

    // Call destructor for LHS.
    if (auto* basicType = llvm::dyn_cast<BasicType>(lhs.getType().getBase())) {
        if (auto* typeDecl = basicType->getDecl()) {
            if (auto* destructor = typeDecl->getDestructor()) {
                createDestructorCall(getFunction(*destructor), value);
            }
        }
    }

    return value;
}

void IRGenerator::createDestructorCall(Function* destructor, Value* receiver) {
    if (!receiver->getType()->isPointerType()) {
        receiver = createTempAlloca(receiver);
    }

    createCall(destructor, receiver, nullptr);
}

Value* IRGenerator::getFunctionForCall(const CallExpr& call) {
    const Decl* decl = call.getCalleeDecl();
    if (!decl) return nullptr;

    switch (decl->getKind()) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::ConstructorDecl:
        case DeclKind::DestructorDecl:
            return getFunction(*llvm::cast<FunctionDecl>(decl));
        case DeclKind::VarDecl:
        case DeclKind::ParamDecl:
            return getValue(decl);
            // TODO : this should work right?
//        case DeclKind::ParamDecl:
//            return NOTNULL(getValueOrNull(decl));
        case DeclKind::FieldDecl:
            if (call.getReceiver()) {
                return emitMemberAccess(emitLvalueExpr(*call.getReceiver()), llvm::cast<FieldDecl>(decl));
            } else {
                return getValue(decl);
            }
        default:
            llvm_unreachable("invalid callee decl");
    }
}

IRModule& IRGenerator::emitModule(const Module& sourceModule) {
    ASSERT(!module);
    module = new IRModule;
    module->name = sourceModule.getName().str();

    for (auto& sourceFile : sourceModule.getSourceFiles()) {
        for (auto& decl : sourceFile.getTopLevelDecls()) {
            emitDecl(*decl);
        }
    }

    for (size_t i = 0; i < functionInstantiations.size(); ++i) {
        auto& instantiation = functionInstantiations[i];

        if (!instantiation.decl->isExtern() && instantiation.function->body.empty()) {
            currentDecl = instantiation.decl;
            emitFunctionBody(*instantiation.decl, *instantiation.function);
        }
    }

    generatedModules.push_back(module);
    module = nullptr;
    return *generatedModules.back();
}
