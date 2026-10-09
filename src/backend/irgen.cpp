#include "irgen.h"
#include "../ast/arena.h"
#include "../ast/module.h"
#include <llvm/Support/SaveAndRestore.h>

using namespace cx;

void IRGenScope::onScopeEnd(llvm::ArrayRef<const Decl*> returnMovedDecls) {
    for (DeferredExpr deferred : reverse(deferredExprs)) {
        llvm::SaveAndRestore saveChecks(irGenerator->disabledChecks, deferred.disabledChecks);
        irGenerator->beginTempScope();
        irGenerator->emitExpr(*deferred.expr);
        irGenerator->endTempScope();
    }

    for (auto& p : reverse(destructorsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        if (p.decl && llvm::is_contained(returnMovedDecls, p.decl)) continue;
        Value* receiver = p.value;
        for (int index : p.indexes) {
            receiver = irGenerator->createGEP(receiver, index);
        }
        if (p.guard) {
            irGenerator->createGuardedDestructorCall(p.function, receiver, p.guard);
        } else {
            irGenerator->createDestructorCall(p.function, receiver);
        }
    }
}

void IRGenScope::clear() {
    deferredExprs.clear();
    destructorsToCall.clear();
}

IRGenerator::IRGenerator(const CompileOptions& options) : options(options) {
    scopes.push_back(IRGenScope(*this));
}

IRGenerator::~IRGenerator() {
    for (auto* module : generatedModules)
        delete module;
}

void IRGenerator::setLocalValue(Value* value, const VariableDecl* decl, bool deferDestructor) {
    auto it = scopes.back().valuesByDecl.try_emplace(decl, value);
    ASSERT(it.second);

    if (decl && deferDestructor) {
        deferDestructorCall(value, decl);
    }
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

    // `this` spills like other by-reference parameters; member access and calls
    // below want the pointer itself.
    if (value->getType()->isPointerType() && value->getType()->getPointee()->isPointerType()) {
        value = createLoad(value);
    }

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

void IRGenerator::beginTempScope() {
    tempScopes.emplace_back();
}

void IRGenerator::destroyTempScope() {
    if (tempScopes.empty()) return;
    unwindTempScopesTo(tempScopes.size() - 1);
}

void IRGenerator::endTempScope() {
    destroyTempScope();
    tempScopes.pop_back();
}

// Destroys every pending temporary: the calls emitted here cover the return
// path, while outer scopes still emit their own calls for paths that fall
// through past the return. Only the return's own scope is cleared, so its
// frame pops silently instead of emitting past the terminator.
void IRGenerator::destroyAllTempScopes() {
    unwindTempScopesTo(0);
}

// Destroys temporaries created after a loop or switch body was entered, keeping
// outer ones alive. Only the jumping statement's own scope is cleared: emitting
// here covers the taken path, while outer scopes still emit for paths that fall
// through. Every frame still pops normally at emission time.
void IRGenerator::unwindTempScopesTo(size_t depth) {
    ASSERT(depth <= tempScopes.size());
    for (auto it = tempScopes.begin() + depth; it != tempScopes.end(); ++it) {
        for (auto& temp : reverse(*it)) {
            Value* receiver = temp.value;
            for (int index : temp.indexes) {
                receiver = createGEP(receiver, index);
            }
            if (temp.guard) {
                createGuardedDestructorCall(temp.function, receiver, temp.guard);
            } else {
                createDestructorCall(temp.function, receiver);
            }
        }
    }
    if (tempScopes.size() > depth) tempScopes.back().clear();
}

// Visits each element of a fixed array whose element type needs destruction.
// Returns false when `type` is not a fixed array, so the caller handles it itself.
template<typename Visit> static bool forEachDestructibleArrayElement(Type type, const std::vector<int>& indexes, Visit&& visit) {
    if (!type.isFixedArray()) return false;
    Type elementType = type.getElementType();
    if (elementType.needsDestruction()) {
        for (int64_t i = 0; i < type.getArraySize(); ++i) {
            auto elementIndexes = indexes;
            elementIndexes.push_back(int(i));
            visit(elementType, std::move(elementIndexes));
        }
    }
    return true;
}

void IRGenerator::registerTempDestructor(Value* base, Type type, std::vector<int> indexes) {
    if (forEachDestructibleArrayElement(
            type, indexes, [&](Type elementType, std::vector<int> elementIndexes) { registerTempDestructor(base, elementType, std::move(elementIndexes)); }))
        return;
    if (emittingReceiver) {
        if (auto* function = getDestructorFunction(type)) {
            scopes.back().destructorsToCall.push_back({function, base, nullptr, std::move(indexes), tempGuard});
        }
        return;
    }
    if (tempScopes.empty()) return;
    if (auto* function = getDestructorFunction(type)) {
        tempScopes.back().push_back({function, base, nullptr, std::move(indexes), tempGuard});
    }
}

Value* IRGenerator::maybeRegisterResultTemp(Value* result, const Expr& expr) {
    // A moved result is owned by its consumer; anything else dies at the
    // end of the enclosing statement. Returning the spill is transparent:
    // emitExpr loads pointers whose pointee matches the expression type.
    // Pointer results are borrows, never fresh values: subscript operators
    // return references that sema erases from the expression type.
    if (!result || result->getType()->isPointerType()) return result;
    if (expr.isMovedFrom || !expr.type.needsDestruction()) return result;
    auto* spill = createTempAlloca(result);
    registerTempDestructor(spill, expr.type);
    return spill;
}

Function* IRGenerator::getDestructorFunction(Type type) {
    if (auto* destructor = type.getDestructor()) {
        checkImplicitCalleeIsChecked(*destructor, "deinit");
        return getFunction(*destructor);
    }
    if (auto* typeDecl = type.getDecl()) {
        if (auto* defaultDestructor = typeDecl->getOrSynthesizeDefaultDestructor()) {
            return getFunction(*defaultDestructor);
        }
    }
    return nullptr;
}

// Creates a conditional-region flag, cleared here and set by the region when it
// runs. The creating block must dominate both the region and the destruction.
Value* IRGenerator::createTempGuard() {
    auto* guard = createEntryBlockAlloca(getIRType(Type::getBool()));
    createStore(createConstantBool(false), guard);
    return guard;
}

void IRGenerator::createGuardedDestructorCall(Function* destructor, Value* receiver, Value* guard) {
    auto* function = insertBlock->parent;
    auto* callBlock = new BasicBlock("temp.dtor", function);
    auto* endBlock = new BasicBlock("temp.dtor.end", function);
    createCondBr(createLoad(guard), callBlock, endBlock);
    setInsertPoint(callBlock);
    createDestructorCall(destructor, receiver);
    createBr(endBlock);
    setInsertPoint(endBlock);
}

void IRGenerator::deferEvaluationOf(const Expr& expr) {
    scopes.back().deferredExprs.push_back({&expr, disabledChecks});
}

void IRGenerator::deferDestructionForType(Value* base, Type type, const VariableDecl* owner, std::vector<int> indexes) {
    if (type.isAnonymousStructType()) {
        int index = 0;
        for (auto& element : type.getAnonymousStructElements()) {
            if (element.type.needsDestruction()) {
                auto elementIndexes = indexes;
                elementIndexes.push_back(index);
                deferDestructionForType(base, element.type, owner, std::move(elementIndexes));
            }
            ++index;
        }
        return;
    }
    if (forEachDestructibleArrayElement(type, indexes, [&](Type elementType, std::vector<int> elementIndexes) {
            deferDestructionForType(base, elementType, owner, std::move(elementIndexes));
        }))
        return;
    if (auto* function = getDestructorFunction(type)) {
        scopes.back().destructorsToCall.push_back({function, base, owner, std::move(indexes)});
    }
}

void IRGenerator::deferDestructorCall(Value* receiver, const VariableDecl* decl) {
    ASSERT(decl->type);
    // '@manuallyDestroy' locals are raw storage: the scope must not destroy them.
    if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
        if (varDecl->isManuallyDestroy) return;
    }
    deferDestructionForType(receiver, decl->type, decl, {});
}

// Destroys elements before overwriting an anonymous struct on assignment.
// GEPs are emitted eagerly: unlike scope-exit destruction, the calls
// immediately follow in the same block.
void IRGenerator::destroyElementsForAssignment(Value* base, Type type) {
    if (type.isAnonymousStructType()) {
        int index = 0;
        for (auto& element : type.getAnonymousStructElements()) {
            if (element.type.needsDestruction()) {
                destroyElementsForAssignment(createGEP(base, index, nullptr, element.name), element.type);
            }
            ++index;
        }
        return;
    }
    if (type.isFixedArray()) {
        Type elementType = type.getElementType();
        if (elementType.needsDestruction()) {
            for (int64_t i = 0; i < type.getArraySize(); ++i) {
                destroyElementsForAssignment(createGEP(base, int(i)), elementType);
            }
        }
        return;
    }
    if (auto* function = getDestructorFunction(type)) {
        createDestructorCall(function, base);
    }
}

void IRGenerator::emitDeferredExprsAndDestructorCallsForReturn(llvm::ArrayRef<const Decl*> returnMovedDecls) {
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

Value* IRGenerator::createCall(Value* function, llvm::ArrayRef<Value*> args, const Expr* expr) {
    ASSERT(function->kind == ValueKind::Function || (function->getType()->isPointerType() && function->getType()->getPointee()->isFunctionType()));
    return insertBlock->add(new CallInst{ValueKind::CallInst, function, args, expr, ""});
}

Value* IRGenerator::createContextCall(Function* callee, llvm::ArrayRef<Value*> args, const Expr* expr) {
    if (!callee->hasContextParam) return createCall(callee, args, expr);
    ASSERT(currentContext);
    llvm::SmallVector<Value*, 16> contextArgs;
    contextArgs.push_back(currentContext);
    llvm::append_range(contextArgs, args);
    return createCall(callee, contextArgs, expr);
}

const FunctionDecl* IRGenerator::findStdlibFunction(const char* name) {
    auto* stdlib = Module::getStdlibModule();
    if (!stdlib) return nullptr;
    return llvm::dyn_cast_or_null<FunctionDecl>(stdlib->symbolTable.findOne(name));
}

const FieldDecl* IRGenerator::findContextAllocatorField() {
    if (Type contextType = getContextStructType()) {
        auto* contextDecl = contextType.getDecl();
        auto field = llvm::find_if(contextDecl->fields, [](const FieldDecl& field) { return field.getName() == "allocator"; });
        if (field != contextDecl->fields.end()) return &*field;
    }
    return nullptr;
}

void IRGenerator::destroyAssignmentLHS(const Expr& lhs, Value* lvalue, bool skipDestructor, bool lhsIsLive) {
    // Assignment into a union never destroys the old value (see Expr::isInsideUnion).
    if (skipDestructor || lhs.isInsideUnion()) return;

    // Don't call destructor for LHS when assigning to fields in constructor.
    // Assignments flagged live overwrite an injected default or a
    // delegation-built value, so the old value is destroyed.
    if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(currentDecl)) {
        Decl* referencedDecl = nullptr;

        if (auto* varExpr = llvm::dyn_cast<VarExpr>(&lhs)) {
            referencedDecl = varExpr->decl;
        } else if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&lhs); memberExpr && memberExpr->base->isThis()) {
            referencedDecl = memberExpr->decl;
        }

        if (auto* fieldDecl = llvm::dyn_cast_or_null<FieldDecl>(referencedDecl)) {
            if (fieldDecl->getParentDecl() == constructorDecl->getTypeDecl() && !lhsIsLive) {
                return;
            }
        }
    }

    // Call destructor for LHS.
    if (auto* destructor = lhs.type.getDestructor()) {
        createDestructorCall(getFunction(*destructor), lvalue);
    } else if (lhs.type.isAnonymousStructType() || lhs.type.isFixedArray()) {
        destroyElementsForAssignment(lvalue, lhs.type);
    } else if (auto* typeDecl = lhs.type.getDecl()) {
        if (auto* defaultDestructor = typeDecl->getOrSynthesizeDefaultDestructor()) {
            createDestructorCall(getFunction(*defaultDestructor), lvalue);
        }
    }
}

void IRGenerator::createDestructorCall(Function* destructor, Value* receiver) {
    if (!receiver->getType()->isPointerType()) {
        receiver = createTempAlloca(receiver);
    }

    createContextCall(destructor, receiver, nullptr);
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

void cx::checkImplicitCalleeIsChecked(const Decl& decl, const char* name) {
    if (decl.checkState != Decl::CheckState::Checked) {
        ABORT("implicit runtime use '" << name << "' was not checked (sema usage tracking missed it)");
    }
}

IRModule& IRGenerator::emitModule(const Module& sourceModule) {
    ASSERT(!module);
    module = new IRModule;
    // Registered up front so a mid-emit throw still leaves the shell owned
    // (and freed by ~IRGenerator) instead of leaked.
    generatedModules.push_back(module);
    module->name = sourceModule.name;
    // A C header's own declarations (e.g. autogenerated constructors) precede the
    // importing module's include of it, so include the header here too. C++ headers
    // can't be included in generated C; declarations are emitted instead, as before.
    if (sourceModule.isCHeaderImport && !sourceModule.isCxxHeaderImport && !sourceModule.sourceFiles.empty()) {
        module->includedHeaders.push_back(sourceModule.sourceFiles.front().filePath);
    }

    for (auto& sourceFile : sourceModule.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            // C globals are always extern references; emitting them here would define them in
            // the wrong module. They materialize on demand in using modules instead.
            if (sourceModule.isCHeaderImport && decl->kind == DeclKind::VarDecl) continue;
            emitDecl(*decl);
        }
    }

    // Methods aren't top-level declarations, so visit referenced ones explicitly: they land
    // in their home module instead of the first module that references them. Unreferenced
    // methods stay unemitted like before. Runs after top-level decls to preserve order.
    for (auto& sourceFile : sourceModule.sourceFiles) {
        for (auto& decl : sourceFile.topLevelDecls) {
            if (auto* typeDecl = llvm::dyn_cast<TypeDecl>(decl)) {
                for (auto* method : typeDecl->methods) {
                    if (method->referenced) emitDecl(*method);
                }
            }
        }
    }

    for (size_t i = 0; i < functionInstantiations.size(); ++i) {
        auto& instantiation = functionInstantiations[i];

        if ((!instantiation.decl->isExtern() || instantiation.decl->body) && instantiation.function->body.empty()) {
            currentDecl = instantiation.decl;
            instantiation.function->isExtern = false;
            emitFunctionBody(*instantiation.decl, *instantiation.function);
        }
    }

    module = nullptr;
    return *generatedModules.back();
}
