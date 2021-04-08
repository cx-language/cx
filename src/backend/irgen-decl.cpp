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

    auto params = map(decl.getParams(), [](const ParamDecl& p) { return Parameter { ValueKind::Parameter, getIRType(p.getType()), p.getName().str() }; });

    if (decl.isMethodDecl()) {
        params.insert(params.begin(), Parameter { ValueKind::Parameter, getIRType(decl.getTypeDecl()->getType().getPointerTo()), "this" });
    }

    auto returnType = getIRType(decl.isMain() ? Type::getInt() : decl.getReturnType());
    auto function = new Function {
        ValueKind::Function, mangledName, returnType, std::move(params), {}, decl.isExtern(), decl.isVariadic(), decl.getLocation(),
    };
    module->functions.push_back(function);

    for (auto& instantiation : functionInstantiations) {
        if (instantiation.function->mangledName == mangledName) {
            return function;
        }
    }

    functionInstantiations.push_back({ &decl, function });
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

    for (auto& param : decl.getParams()) {
        setLocalValue(&*arg++, &param);
    }

    if (decl.isDestructorDecl()) {
        for (auto& field : decl.getTypeDecl()->getFields()) {
            if (!field.getType().getDestructor()) continue;
            deferDestructorCall(emitMemberAccess(&function.params[0], &field), &field);
        }
    }

    for (auto& stmt : decl.getBody()) {
        emitStmt(*stmt);
    }

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
        ASSERT(decl.getInitializer());
        Value* value = emitExpr(*decl.getInitializer());

        if (decl.getType().isMutable()) {
            value = createGlobalVariable(value, decl.getName());
        }

        auto it = globalScope().valuesByDecl.try_emplace(&decl, value);
        ASSERT(it.second);
        return value;
    } else {
        auto* alloca = createEntryBlockAlloca(decl.getType(), decl.getName());
        setLocalValue(alloca, &decl);
        auto* initializer = decl.getInitializer();
        if (!initializer) return alloca;

        if (auto* callExpr = llvm::dyn_cast<CallExpr>(initializer)) {
            if (callExpr->getCalleeDecl()) {
                if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(callExpr->getCalleeDecl())) {
                    if (constructorDecl->getTypeDecl()->getType() == decl.getType()) {
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

    switch (decl.getKind()) {
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
        case DeclKind::TypeDecl:
        case DeclKind::ImportDecl:
        case DeclKind::FunctionTemplate:
        case DeclKind::TypeTemplate:
        case DeclKind::EnumDecl:
        case DeclKind::EnumCase:
            break;
    }
}
