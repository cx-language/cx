#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/IR/CFG.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/mangle.h"

using namespace delta;

llvm::Function* IRGenerator::getFunctionProto(const FunctionDecl& decl) {
    auto mangled = mangleFunctionDecl(decl);
    if (auto* function = module->getFunction(mangled)) return function;

    auto* functionType = decl.getFunctionType();
    llvm::SmallVector<llvm::Type*, 16> paramTypes;

    if (decl.isMethodDecl()) {
        paramTypes.emplace_back(llvm::PointerType::get(getLLVMType(decl.getTypeDecl()->getType()), 0));
    }

    for (auto& paramType : functionType->getParamTypes()) {
        paramTypes.emplace_back(getLLVMType(paramType));
    }

    auto* returnType = getLLVMType(functionType->getReturnType());
    if (decl.isMain() && returnType->isVoidTy()) returnType = llvm::Type::getInt32Ty(ctx);

    auto* llvmFunctionType = llvm::FunctionType::get(returnType, paramTypes, decl.isVariadic());
    auto* function = llvm::Function::Create(llvmFunctionType, llvm::Function::ExternalLinkage, mangled, &*module);

    auto arg = function->arg_begin(), argsEnd = function->arg_end();
    if (decl.isMethodDecl()) arg++->setName("this");

    ASSERT(decl.getParams().size() == size_t(std::distance(arg, argsEnd)));
    for (auto param = decl.getParams().begin(); arg != argsEnd; ++param, ++arg) {
        arg->setName(param->getName());
    }

    for (auto& instantiation : functionInstantiations) {
        if (instantiation.function->getName() == mangled) {
            return function;
        }
    }

    functionInstantiations.push_back({ &decl, function });
    return function;
}

void IRGenerator::codegenFunctionBody(const FunctionDecl& decl, llvm::Function& function) {
    builder.SetInsertPoint(llvm::BasicBlock::Create(ctx, "", &function));
    beginScope();
    auto arg = function.arg_begin();
    if (decl.getTypeDecl() != nullptr) setLocalValue(&*arg++, nullptr);
    for (auto& param : decl.getParams()) {
        setLocalValue(&*arg++, &param);
    }
    if (decl.isDestructorDecl()) {
        for (auto& field : decl.getTypeDecl()->getFields()) {
            if (field.getType().getDestructor() == nullptr) continue;
            deferDestructorCall(codegenMemberAccess(function.arg_begin(), &field), &field);
        }
    }
    for (auto& stmt : decl.getBody()) {
        codegenStmt(*stmt);
    }
    endScope();

    auto* insertBlock = builder.GetInsertBlock();
    if (insertBlock != &function.getEntryBlock() && llvm::pred_empty(insertBlock)) {
        insertBlock->eraseFromParent();
        return;
    }

    if (insertBlock->empty() || !llvm::isa<llvm::ReturnInst>(insertBlock->back())) {
        if (!decl.isMain()) {
            builder.CreateRetVoid();
        } else {
            builder.CreateRet(llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0));
        }
    }
}

void IRGenerator::codegenFunctionDecl(const FunctionDecl& decl) {
    llvm::Function* function = getFunctionProto(decl);

    if (!decl.isExtern() && function->empty()) {
        codegenFunctionBody(decl, *function);
    }

#ifndef NDEBUG
    if (llvm::verifyFunction(*function, &llvm::errs())) {
        llvm::errs() << '\n';
        function->print(llvm::errs(), nullptr, false, true);
        llvm::errs() << '\n';
        ASSERT(false && "llvm::verifyFunction failed");
    }
#endif
}

std::vector<llvm::Type*> IRGenerator::getFieldTypes(const TypeDecl& decl) {
    return map(decl.getFields(), [&](auto& field) { return getLLVMType(field.getType(), field.getLocation()); });
}

llvm::StructType* IRGenerator::codegenTypeDecl(const TypeDecl& d) {
    // TODO: Figure out a better way to handle the use of 'This' in interface field types.
    const TypeDecl& decl = d.isInterface() ? *llvm::cast<TypeDecl>(d.instantiate({ { "This", d.getType() } }, {})) : d;

    llvm::StructType* structType;
    auto qualifiedName = decl.getQualifiedName();
    auto it = structs.find(qualifiedName);

    if (it != structs.end()) {
        structType = it->second.first;
    } else {
        if (decl.getFields().empty()) {
            structType = llvm::StructType::get(ctx);
        } else {
            structType = llvm::StructType::create(ctx, qualifiedName);
        }

        structs.try_emplace(qualifiedName, std::make_pair(structType, &decl));
    }

    auto fieldTypes = getFieldTypes(decl);

    if (!structType->isOpaque()) {
        return structType;
    }

    structType->setBody(std::move(fieldTypes));

    auto insertBlockBackup = builder.GetInsertBlock();
    auto insertPointBackup = builder.GetInsertPoint();
    auto scopesBackup = std::move(scopes);

    if (!decl.getFields().empty()) {
        for (llvm::Type* element : structType->elements()) {
            if (auto* elementStruct = llvm::dyn_cast<llvm::StructType>(element)) {
                if (elementStruct->isLiteral()) continue;
                auto it = structs.find(elementStruct->getName());
                if (it != structs.end()) {
                    codegenTypeDecl(*it->second.second);
                }
            }
        }
    }

    if (decl.getDestructor() == nullptr) {
        if (auto destructor = getDefaultDestructor(decl)) {
            codegenDecl(*destructor);
        }
    }

    scopes = std::move(scopesBackup);
    if (insertBlockBackup) builder.SetInsertPoint(insertBlockBackup, insertPointBackup);

    return structType;
}

llvm::Value* IRGenerator::codegenVarDecl(const VarDecl& decl) {
    if (decl.getName() == "this") {
        return getThis();
    }

    if (auto* value = getValueOrNull(&decl)) {
        return value;
    }

    ASSERT(decl.getInitializer());
    llvm::Value* value = codegenExpr(*decl.getInitializer());

    if (decl.getType().isMutable() /* || decl.isPublic() */) {
        auto linkage = value ? llvm::GlobalValue::PrivateLinkage : llvm::GlobalValue::ExternalLinkage;
        auto initializer = value ? llvm::cast<llvm::Constant>(value) : nullptr;
        value = new llvm::GlobalVariable(*module, getLLVMType(decl.getType()), false, linkage, initializer, decl.getName());
    }

    auto it = globalScope().valuesByDecl.try_emplace(&decl, value);
    ASSERT(it.second);
    return value;
}

void IRGenerator::codegenDecl(const Decl& decl) {
    llvm::SaveAndRestore setCurrentDecl(currentDecl, &decl);

    switch (decl.getKind()) {
        case DeclKind::ParamDecl:
            llvm_unreachable("handled via FunctionDecl");
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
            codegenFunctionDecl(llvm::cast<FunctionDecl>(decl));
            break;
        case DeclKind::GenericParamDecl:
            llvm_unreachable("cannot codegen generic parameter declaration");
        case DeclKind::ConstructorDecl:
            codegenFunctionDecl(llvm::cast<ConstructorDecl>(decl));
            break;
        case DeclKind::DestructorDecl:
            codegenFunctionDecl(llvm::cast<DestructorDecl>(decl));
            break;
        case DeclKind::FunctionTemplate:
            break;
        case DeclKind::TypeDecl:
            codegenTypeDecl(llvm::cast<TypeDecl>(decl));
            break;
        case DeclKind::TypeTemplate:
            break;
        case DeclKind::EnumDecl:
            break;
        case DeclKind::EnumCase:
            break;
        case DeclKind::VarDecl:
            codegenVarDecl(llvm::cast<VarDecl>(decl));
            break;
        case DeclKind::FieldDecl:
            llvm_unreachable("handled via TypeDecl");
        case DeclKind::ImportDecl:
            break;
    }
}
