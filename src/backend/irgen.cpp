#include "irgen.h"
#include "../ast/arena.h"
#include "../ast/module.h"

using namespace cx;

void IRGenScope::onScopeEnd(const llvm::SmallPtrSetImpl<const Decl*>* returnMovedDecls) {
    for (const Expr* expr : reverse(deferredExprs)) {
        irGenerator->emitExpr(*expr);
    }

    for (auto& p : reverse(destructorsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        if (p.decl && returnMovedDecls && returnMovedDecls->contains(p.decl)) continue;
        Value* receiver = p.value;
        for (int index : p.indexes) {
            receiver = irGenerator->createGEP(receiver, index);
        }
        irGenerator->createDestructorCall(p.function, receiver);
    }
}

void IRGenScope::clear() {
    deferredExprs.clear();
    destructorsToCall.clear();
}

IRGenerator::IRGenerator(const CompileOptions& options) : options(options) {
    scopes.push_back(IRGenScope(*this));
}

void IRGenerator::setLocalValue(Value* value, const VariableDecl* decl) {
    auto it = scopes.back().valuesByDecl.try_emplace(decl, value);
    ASSERT(it.second);

    if (decl) {
        deferDestructorCall(value, decl);
    }
}

// Binds a switch associated value, which borrows the enum payload storage and
// must not be destroyed (the payload is owned by the switched enum value).
void IRGenerator::setBorrowedValue(Value* value, const VariableDecl* decl) {
    auto it = scopes.back().valuesByDecl.try_emplace(decl, value);
    ASSERT(it.second);
}

Value* IRGenerator::getValueOrNull(const Decl* decl) {
    for (auto& scope : llvm::reverse(scopes)) {
        auto it = scope.valuesByDecl.find(decl);
        if (it != scope.valuesByDecl.end()) {
            return it->second;
        }
    }

    return nullptr;
}

Value* IRGenerator::getValue(const Decl* decl) {
    if (auto* value = getValueOrNull(decl)) {
        return value;
    }

    switch (decl->kind) {
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

    for (auto& field : typeDecl.fields) {
        if (field.type.getDestructor() || anonymousStructNeedsDestruction(field.type)) {
            auto destructor = makeAST<DestructorDecl>(typeDecl, typeDecl.getLocation());
            destructor->body = std::vector<Stmt*>();
            return destructor;
        }
    }

    return nullptr;
}

// True when an anonymous struct transitively contains an element that needs
// destruction: an explicit destructor, a named struct with an explicit (or
// shallow default) destructor, or a nested anonymous struct that qualifies.
bool IRGenerator::anonymousStructNeedsDestruction(Type type) {
    if (!type.isAnonymousStructType()) return false;
    for (auto& element : type.getAnonymousStructElements()) {
        if (element.type.getDestructor()) return true;
        if (anonymousStructNeedsDestruction(element.type)) return true;
        if (auto* elementDecl = element.type.getDecl()) {
            if (elementDecl->getDestructor()) return true;
            for (auto& field : elementDecl->fields) {
                if (field.type.getDestructor() || anonymousStructNeedsDestruction(field.type)) return true;
            }
        }
    }
    return false;
}

bool IRGenerator::typeNeedsDestruction(Type type) {
    if (type.getDestructor()) return true;
    if (anonymousStructNeedsDestruction(type)) return true;
    if (auto* typeDecl = type.getDecl()) {
        // Preserve the existing shallow check for named structs; anonymous
        // struct awareness is handled via anonymousStructNeedsDestruction above
        // and in getDefaultDestructor.
        for (auto& field : typeDecl->fields) {
            if (field.type.getDestructor() || anonymousStructNeedsDestruction(field.type)) return true;
        }
    }
    return false;
}

void IRGenerator::deferDestructionForType(Value* base, Type type, const VariableDecl* owner, std::vector<int> indexes) {
    if (auto* destructor = type.getDestructor()) {
        scopes.back().destructorsToCall.push_back({getFunction(*destructor), base, owner, std::move(indexes)});
    } else if (type.isAnonymousStructType()) {
        int index = 0;
        for (auto& element : type.getAnonymousStructElements()) {
            if (typeNeedsDestruction(element.type)) {
                auto elementIndexes = indexes;
                elementIndexes.push_back(index);
                deferDestructionForType(base, element.type, owner, std::move(elementIndexes));
            }
            ++index;
        }
    } else if (auto* typeDecl = type.getDecl()) {
        if (auto defaultDestructor = getDefaultDestructor(*typeDecl)) {
            scopes.back().destructorsToCall.push_back({getFunction(*defaultDestructor), base, owner, std::move(indexes)});
        }
    }
}

void IRGenerator::deferDestructorCall(Value* receiver, const VariableDecl* decl) {
    ASSERT(decl->type);
    deferDestructionForType(receiver, decl->type, decl, {});
}

// Destroys explicit-destructor elements before overwriting an anonymous struct
// on assignment, mirroring emitAssignmentLHS for named structs (which only
// destroys explicit destructors, not default ones). GEPs are emitted eagerly:
// unlike scope-exit destruction, the calls immediately follow in the same block.
void IRGenerator::destroyExplicitElementsForAssignment(Value* base, Type type) {
    if (auto* destructor = type.getDestructor()) {
        createDestructorCall(getFunction(*destructor), base);
    } else if (type.isAnonymousStructType()) {
        int index = 0;
        for (auto& element : type.getAnonymousStructElements()) {
            if (element.type.getDestructor() || anonymousStructHasExplicitDestruction(element.type)) {
                destroyExplicitElementsForAssignment(createGEP(base, index, nullptr, element.name), element.type);
            }
            ++index;
        }
    }
}

// True when an anonymous struct transitively contains an explicit destructor,
// used to avoid emitting dead GEPs on assignment when there is nothing to destroy.
bool IRGenerator::anonymousStructHasExplicitDestruction(Type type) {
    if (!type.isAnonymousStructType()) return false;
    for (auto& element : type.getAnonymousStructElements()) {
        if (element.type.getDestructor()) return true;
        if (anonymousStructHasExplicitDestruction(element.type)) return true;
    }
    return false;
}

void IRGenerator::emitDeferredExprsAndDestructorCallsForReturn(const llvm::SmallPtrSetImpl<const Decl*>* returnMovedDecls) {
    for (auto& scope : llvm::reverse(scopes)) {
        scope.onScopeEnd(returnMovedDecls);
    }
    scopes.back().clear();
}

AllocaInst* IRGenerator::createEntryBlockAlloca(IRType* type, const llvm::Twine& name) {
    auto alloca = new AllocaInst{ValueKind::AllocaInst, type, name.str()};
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
    return insertBlock->add(new LoadInst{ValueKind::LoadInst, value, expr, value->getName() + ".load"});
}

void IRGenerator::createStore(Value* value, Value* pointer) {
    ASSERT(pointer->getType()->isPointerType());
    ASSERT(pointer->getType()->getPointee()->equals(value->getType()));
    insertBlock->add(new StoreInst{ValueKind::StoreInst, value, pointer});
}

Value* IRGenerator::createCall(Value* function, llvm::ArrayRef<Value*> args, const CallExpr* expr) {
    ASSERT(function->kind == ValueKind::Function || (function->getType()->isPointerType() && function->getType()->getPointee()->isFunctionType()));
    return insertBlock->add(new CallInst{ValueKind::CallInst, function, args, expr, ""});
}

Value* IRGenerator::emitAssignmentLHS(const Expr& lhs, bool skipDestructor) {
    Value* value = emitLvalueExpr(lhs);

    if (skipDestructor) return value;

    // Don't call destructor for LHS when assigning to fields in constructor.
    if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(currentDecl)) {
        Decl* referencedDecl = nullptr;

        if (auto* varExpr = llvm::dyn_cast<VarExpr>(&lhs)) {
            referencedDecl = varExpr->decl;
        } else if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&lhs); memberExpr && memberExpr->base->isThis()) {
            referencedDecl = memberExpr->decl;
        }

        if (auto* fieldDecl = llvm::dyn_cast_or_null<FieldDecl>(referencedDecl)) {
            if (fieldDecl->getParentDecl() == constructorDecl->getTypeDecl()) {
                return value;
            }
        }
    }

    // Call destructor for LHS.
    if (auto* basicType = llvm::dyn_cast<BasicType>(lhs.type.typeBase)) {
        if (auto* typeDecl = basicType->decl) {
            if (auto* destructor = typeDecl->getDestructor()) {
                createDestructorCall(getFunction(*destructor), value);
            }
        }
    } else if (lhs.type.isAnonymousStructType()) {
        destroyExplicitElementsForAssignment(value, lhs.type);
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
    const Decl* decl = call.calleeDecl;
    if (!decl) return nullptr;

    switch (decl->kind) {
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl:
        return getFunction(*llvm::cast<FunctionDecl>(decl));
    case DeclKind::VarDecl:
    case DeclKind::ParamDecl:
        return getValue(decl);
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
    module->name = sourceModule.name;

    for (auto& sourceFile : sourceModule.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            // C globals are always extern references; emitting them here would define them in
            // the wrong module. They materialize on demand in using modules instead.
            if (sourceModule.isCHeaderImport && decl->kind == DeclKind::VarDecl) continue;
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
