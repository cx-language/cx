#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/mangle.h"

using namespace cx;

Function* IRGenerator::getFunction(const FunctionDecl& decl) {
    auto mangledName = mangleFunctionDecl(decl);

    for (auto* function : module->functions) {
        if (function->mangledName == mangledName) {
            return function;
        }
    }

    auto params = map(decl.getParams(), [](const ParamDecl& p) { return Parameter{ValueKind::Parameter, getIRType(p.type), p.getName().str()}; });

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
    auto function = new Function{
        ValueKind::Function, mangledName, decl.getName().str(), returnType, std::move(params), {}, decl.isExtern(), decl.isVariadic(), decl.getLocation(),
    };
    module->functions.push_back(function);

    for (auto& instantiation : functionInstantiations) {
        if (instantiation.function->mangledName == mangledName) {
            return function;
        }
    }

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

    for (auto& param : decl.getParams()) {
        setLocalValue(&*arg++, &param);
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
        Value* value = decl.initializer ? emitExpr(*decl.initializer) : nullptr;

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
