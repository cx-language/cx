#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/mangle.h"
#include "../ast/module.h"

using namespace cx;

Function* IRGenerator::getFunction(const FunctionDecl& decl) {
    auto mangledName = mangleFunctionDecl(decl);

    for (auto* function : module->functions) {
        if (function->mangledName == mangledName) {
            return function;
        }
    }

    auto params = map(decl.getParams(), [](const ParamDecl& p) { return Parameter{ValueKind::Parameter, getIRType(p.type), p.getName().str()}; });

    if (decl.isMain() && !decl.isMethodDecl() && !decl.getParams().empty()) {
        // The OS passes argc/argv; the declared args array is materialized from them in emitFunctionBody.
        params = {Parameter{ValueKind::Parameter, getIRType(Type::getInt()), "argc"},
                  Parameter{ValueKind::Parameter, getIRType(BasicType::get("char", {}).getPointerTo().getPointerTo()), "argv"}};
    }

    if (!decl.captures.empty()) {
        auto captureParams = map(decl.captures, [](const VariableDecl* c) {
            return Parameter{ValueKind::Parameter, getIRType(c->getCaptureType()), ("__capture_" + c->getName()).str()};
        });
        params.insert(params.begin(), captureParams.begin(), captureParams.end());
    }

    if (decl.isMethodDecl()) {
        params.insert(params.begin(), Parameter{ValueKind::Parameter, getIRType(decl.getTypeDecl()->getType().getPointerTo()), "this"});
    }

    auto returnType = getIRType(decl.isMain() ? Type::getInt() : decl.getReturnType());

    // Definitions are emitted once per program: a function referenced from several
    // modules reuses the first module's object, so later modules call it as an
    // external declaration instead of emitting a duplicate definition. Reuse only
    // when the signature matches; same-named externs with different types (e.g. a
    // C import shadowing a std extern) keep separate objects so each call uses
    // its own module's parameter types.
    for (auto& instantiation : functionInstantiations) {
        auto* existing = instantiation.function;
        if (existing->mangledName != mangledName) continue;
        if (existing->isVariadic != decl.isVariadic()) continue;
        if (!existing->returnType->equals(returnType)) continue;
        if (existing->params.size() != params.size()) continue;
        bool signatureMatch = true;
        for (size_t i = 0; i < params.size(); ++i) {
            if (!existing->params[i].type->equals(params[i].type)) {
                signatureMatch = false;
                break;
            }
        }
        if (signatureMatch) return existing;
    }

    auto function = new Function{
        ValueKind::Function, mangledName, decl.getName().str(), returnType, std::move(params), {}, decl.isExtern(), decl.isVariadic(), decl.getLocation(),
    };
    module->functions.push_back(function);

    functionInstantiations.push_back({&decl, function});
    return function;
}

void IRGenerator::emitFunctionBody(const FunctionDecl& decl, Function& function) {
    currentFunction = &function;
    setInsertPoint(new BasicBlock("", &function));
    beginScope();

    auto arg = function.params.begin();

    if (decl.getTypeDecl()) {
        setLocalValue(&*arg++, nullptr);
    }

    for (auto* captured : decl.captures) {
        if (captured->isReferenceCapture()) {
            // Captured `this` is already a pointer to the caller's object; bind it directly
            // like a method's `this` param so member access aliases the object, not a copy.
            Value* thisParam = &*arg++;
            auto inserted = scopes.back().valuesByDecl.try_emplace(captured, thisParam);
            ASSERT(inserted.second);
            auto thisInserted = scopes.back().valuesByDecl.try_emplace(nullptr, thisParam);
            ASSERT(thisInserted.second);
            continue;
        }
        // Captures spill to allocas so stores, member access, and address-of treat them like
        // locals. The allocas hold per-call copies, so unlike regular locals they get no
        // destructor call here; the closure's stored values are destroyed with the closure.
        auto* spill = createEntryBlockAlloca(captured->type, ("__capture_" + captured->getName()).str());
        createStore(&*arg++, spill);
        auto inserted = scopes.back().valuesByDecl.try_emplace(captured, spill);
        ASSERT(inserted.second);
    }

    Value* mainArgv = nullptr;
    if (decl.isMain() && !decl.isMethodDecl() && decl.getParams().size() == 1) {
        mainArgv = emitMainArgv(&function.params[0], &function.params[1], decl.getParams()[0].type, decl.getLocation());
    }

    for (auto& param : decl.getParams()) {
        // Spill parameters to allocas so they have stable addresses: method receivers and
        // address-of must alias the parameter across uses, not a fresh temporary per use.
        Value* value = &*arg++;
        if (mainArgv) value = mainArgv;
        auto* spill = createEntryBlockAlloca(param.type, param.getName());
        createStore(value, spill);
        setLocalValue(spill, &param);
    }

    if (decl.isDestructorDecl()) {
        for (auto& field : decl.getTypeDecl()->fields) {
            if (!field.type.getDestructor()) continue;
            deferDestructorCall(emitMemberAccess(&function.params[0], &field), &field);
        }
    }

    emitStmts(*decl.body);
    endScope();

    if (insertBlock->body.empty() || !llvm::isa<ReturnInst>(insertBlock->body.back())) {
        if (decl.getReturnType().isVoid()) {
            createReturn(decl.isMain() ? createConstantInt(Type::getInt(), 0) : nullptr);
        } else {
            createUnreachable();
        }
    }
}

Value* IRGenerator::emitMainArgv(Value* argc, Value* argv, Type argvType, Location location) {
    Type stringType = BasicType::get("string", {});
    auto* mallocFunction = getFunction(*llvm::cast<FunctionDecl>(Module::getStdlibModule()->symbolTable.findOne("malloc")));

    // Copy the C strings into a heap array of strings that lives for the whole program run.
    Value* count = createCast(argc, Type::getUInt64(), "argv.count");
    Value* allocSize = createBinaryOp(Token::Star, createSizeof(stringType), count, nullptr, "argv.size");
    Value* storage = createCall(mallocFunction, allocSize, nullptr);
    // malloc(0) may return null, but no storage is needed when there are no arguments.
    Value* hasStorage = createBinaryOp(Token::NotEqual, storage, createConstantNull(storage->getType()), nullptr);
    Value* noArgs = createBinaryOp(Token::Equal, argc, createConstantInt(Type::getInt(), 0), nullptr);
    emitAssert(createBinaryOp(Token::Or, hasStorage, noArgs, nullptr), nullptr, location, "Out of memory", "argv");

    Function* stringInit = nullptr;
    for (auto* decl : Module::getStdlibModule()->symbolTable.findInTopLevelScope("string.init")) {
        auto params = llvm::cast<ConstructorDecl>(decl)->getParams();
        if (params.size() == 1 && params[0].type.isPointerType() && params[0].type.getPointee().isChar()) {
            stringInit = getFunction(*llvm::cast<ConstructorDecl>(decl));
            break;
        }
    }
    ASSERT(stringInit);

    Value* elements = createCast(storage, stringType.getPointerTo(), "argv.elements");
    auto* indexAlloca = createEntryBlockAlloca(Type::getInt(), "argv_i");
    createStore(createConstantInt(Type::getInt(), 0), indexAlloca);

    auto* cond = new BasicBlock("argv.cond");
    auto* body = new BasicBlock("argv.body");
    auto* end = new BasicBlock("argv.end");
    createBr(cond);

    setInsertPoint(cond);
    Value* index = createLoad(indexAlloca);
    createCondBr(createBinaryOp(Token::Less, index, argc, nullptr), body, end);

    setInsertPoint(body);
    Value* cString = createLoad(createGEP(argv, {index}));
    createCall(stringInit, {createGEP(elements, {index}), cString}, nullptr);
    createStore(createBinaryOp(Token::Plus, index, createConstantInt(Type::getInt(), 1), nullptr), indexAlloca);
    createBr(cond);

    setInsertPoint(end);
    Value* array = createUndefined(argvType);
    array = createInsertValue(array, elements, 0);
    return createInsertValue(array, argc, 1);
}

void IRGenerator::emitFunctionDecl(const FunctionDecl& decl) {
    auto function = getFunction(decl);

    if (!decl.isExtern() && function->body.empty()) {
        emitFunctionBody(decl, *function);
    }
}

Value* IRGenerator::emitVarDecl(const VarDecl& decl) {
    if (decl.getName() == "this") {
        return getThis();
    }

    if (auto* value = getValueOrNull(&decl)) {
        return value;
    }

    if (decl.isGlobal()) {
        // Global initializers are pure constants, but emission creates value instructions that
        // need an insert block. Give them a dead block (leaked like the rest of the IR) instead
        // of the previous function's block, and null out the current function so any attempt
        // to emit control flow, calls, or allocas fails loudly instead of corrupting codegen.
        // Cyclic constants can leave initializers untyped (a pre-existing sema gap); those keep
        // the previous function so emission limps along exactly like before.
        bool wellFormed = !decl.initializer || decl.initializer->hasType();
        auto* deadBlock = new BasicBlock("global.init");
        llvm::SaveAndRestore saveInsertBlock(insertBlock, deadBlock);
        llvm::SaveAndRestore<Function*> saveCurrentFunction(currentFunction, wellFormed ? nullptr : currentFunction);
        llvm::SaveAndRestore saveEmittingGlobal(emittingGlobalInitializer, true);

        Value* value = decl.initializer ? emitExpr(*decl.initializer) : nullptr;

        if (wellFormed) {
            for (auto* inst : deadBlock->body) {
                ASSERT(llvm::isa<InsertInst>(inst) || llvm::isa<BinaryInst>(inst) || llvm::isa<UnaryInst>(inst));
            }
        }

        if (decl.type.isMutable()) {
            value = createGlobalVariable(value, decl.type, decl.getName());
        }

        auto it = globalScope().valuesByDecl.try_emplace(&decl, value);
        ASSERT(it.second);
        return value;
    } else {
        auto* alloca = createEntryBlockAlloca(decl.type, decl.getName());
        setLocalValue(alloca, &decl);
        auto* initializer = decl.initializer;
        if (!initializer) return alloca;

        if (auto* callExpr = llvm::dyn_cast<CallExpr>(initializer)) {
            if (callExpr->calleeDecl) {
                if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(callExpr->calleeDecl)) {
                    if (constructorDecl->getTypeDecl()->getType() == decl.type) {
                        emitCallExpr(*callExpr, alloca);
                        return alloca;
                    }
                }
            }
        }

        if (!initializer->isUndefinedLiteralExpr()) {
            createStore(emitExprForPassing(*initializer, alloca->allocatedType), alloca);
        }

        return alloca;
    }
}

void IRGenerator::emitDecl(const Decl& decl) {
    llvm::SaveAndRestore setCurrentDecl(currentDecl, &decl);

    switch (decl.kind) {
    case DeclKind::ParamDecl:
        llvm_unreachable("handled via FunctionDecl");
    case DeclKind::FunctionDecl:
    case DeclKind::MethodDecl:
    case DeclKind::ConstructorDecl:
    case DeclKind::DestructorDecl:
        emitFunctionDecl(llvm::cast<FunctionDecl>(decl));
        break;
    case DeclKind::GenericParamDecl:
        llvm_unreachable("cannot emit generic parameter declaration");
    case DeclKind::VarDecl:
        emitVarDecl(llvm::cast<VarDecl>(decl));
        break;
    case DeclKind::FieldDecl:
        llvm_unreachable("handled via TypeDecl");
    case DeclKind::ImportDecl: {
        auto& importDecl = llvm::cast<ImportDecl>(decl);
        if (!importDecl.importedHeaderPath.empty()) {
            module->includedHeaders.push_back(importDecl.importedHeaderPath);
        }
        break;
    }
    case DeclKind::TypeDecl:
    case DeclKind::FunctionTemplate:
    case DeclKind::TypeTemplate:
    case DeclKind::EnumDecl:
    case DeclKind::EnumCase:
        break;
    }
}
