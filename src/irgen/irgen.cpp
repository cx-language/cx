#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/ADT/STLExtras.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/IR/Verifier.h>
#pragma warning(pop)
#include "../ast/module.h"

using namespace delta;

void Scope::onScopeEnd() {
    for (const Expr* expr : llvm::reverse(deferredExprs)) {
        irGenerator->codegenExpr(*expr);
    }

    for (auto& p : llvm::reverse(deinitsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        irGenerator->createDeinitCall(p.function, p.value, p.type, p.decl);
    }
}

void Scope::clear() {
    deferredExprs.clear();
    deinitsToCall.clear();
}

IRGenerator::IRGenerator() : builder(ctx) {
    scopes.push_back(Scope(*this));
}

/// @param type The Delta type of the variable, or null if the variable is 'this'.
void IRGenerator::setLocalValue(Type type, std::string name, llvm::Value* value, const Decl* decl) {
    scopes.back().addLocalValue(std::move(name), value);

    if (type) {
        deferDeinitCall(value, type, decl);
    }
}

llvm::Value* IRGenerator::findValue(llvm::StringRef name, const Decl* decl) {
    llvm::Value* value = nullptr;

    for (const auto& scope : llvm::reverse(scopes)) {
        auto it = scope.getLocalValues().find(name);
        if (it == scope.getLocalValues().end()) continue;
        value = it->second;
        break;
    }

    if (value) {
        return value;
    }

    ASSERT(decl);

    switch (decl->getKind()) {
        case DeclKind::VarDecl:
            return codegenVarDecl(*llvm::cast<VarDecl>(decl));

        case DeclKind::FieldDecl:
            return codegenMemberAccess(findValue("this", nullptr), llvm::cast<FieldDecl>(decl)->getType(), llvm::cast<FieldDecl>(decl)->getName());

        case DeclKind::FunctionDecl:
            return getFunctionProto(*llvm::cast<FunctionDecl>(decl));

        default:
            llvm_unreachable("all cases handled");
    }
}

llvm::Type* IRGenerator::getBuiltinType(llvm::StringRef name) {
    return llvm::StringSwitch<llvm::Type*>(name)
        .Case("void", llvm::Type::getVoidTy(ctx))
        .Case("bool", llvm::Type::getInt1Ty(ctx))
        .Case("char", llvm::Type::getInt8Ty(ctx))
        .Case("int", llvm::Type::getInt32Ty(ctx))
        .Case("int8", llvm::Type::getInt8Ty(ctx))
        .Case("int16", llvm::Type::getInt16Ty(ctx))
        .Case("int32", llvm::Type::getInt32Ty(ctx))
        .Case("int64", llvm::Type::getInt64Ty(ctx))
        .Case("uint", llvm::Type::getInt32Ty(ctx))
        .Case("uint8", llvm::Type::getInt8Ty(ctx))
        .Case("uint16", llvm::Type::getInt16Ty(ctx))
        .Case("uint32", llvm::Type::getInt32Ty(ctx))
        .Case("uint64", llvm::Type::getInt64Ty(ctx))
        .Case("float", llvm::Type::getFloatTy(ctx))
        .Case("float32", llvm::Type::getFloatTy(ctx))
        .Case("float64", llvm::Type::getDoubleTy(ctx))
        .Case("float80", llvm::Type::getX86_FP80Ty(ctx))
        .Default(nullptr);
}

llvm::Type* IRGenerator::toIR(Type type, SourceLocation location) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            if (auto* builtinType = getBuiltinType(type.getName())) return builtinType;

            auto it = structs.find(type.getQualifiedTypeName());
            if (it != structs.end()) return it->second.first;

            auto& basicType = llvm::cast<BasicType>(*type);
            if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(basicType.getDecl())) {
                return toIR(enumDecl->getUnderlyingType());
            }

            return codegenTypeDecl(*basicType.getDecl());
        }
        case TypeKind::ArrayType:
            ASSERT(type.isArrayWithConstantSize(), "unimplemented");
            return llvm::ArrayType::get(toIR(type.getElementType(), location), type.getArraySize());
        case TypeKind::TupleType: {
            auto elementTypes = map(type.getTupleElements(), [&](const TupleElement& element) { return toIR(element.type); });
            return llvm::StructType::get(ctx, elementTypes);
        }
        case TypeKind::FunctionType: {
            auto paramTypes = map(type.getParamTypes(), [&](Type type) { return toIR(type); });
            auto* returnType = toIR(type.getReturnType());
            return llvm::FunctionType::get(returnType, paramTypes, false)->getPointerTo();
        }
        case TypeKind::PointerType: {
            if (type.getPointee().isArrayWithRuntimeSize()) {
                return toIR(BasicType::get("ArrayRef", type.getPointee().getElementType()), location);
            } else if (type.getPointee().isArrayWithUnknownSize()) {
                return llvm::PointerType::get(toIR(type.getPointee().getElementType(), location), 0);
            }

            auto* pointeeType = toIR(type.getPointee(), location);
            return llvm::PointerType::get(pointeeType->isVoidTy() ? llvm::Type::getInt8Ty(ctx) : pointeeType, 0);
        }
        case TypeKind::OptionalType:
            if (type.getWrappedType().isPointerType() || type.getWrappedType().isFunctionType()) {
                return toIR(type.getWrappedType());
            }
            llvm_unreachable("IRGen doesn't support non-pointer optional types yet");
    }
    llvm_unreachable("all cases handled");
}

void IRGenerator::beginScope() {
    scopes.push_back(Scope(*this));
}

void IRGenerator::endScope() {
    scopes.back().onScopeEnd();
    scopes.pop_back();
}

void IRGenerator::deferEvaluationOf(const Expr& expr) {
    scopes.back().addDeferredExpr(expr);
}

/// Returns a deinitializer that only calls the deinitializers of the member variables, or null if
/// no such deinitializer is needed because none of the member variables have deinitializers.
DeinitDecl* IRGenerator::getDefaultDeinitializer(const TypeDecl& typeDecl) {
    ASSERT(typeDecl.getDeinitializer() == nullptr);

    for (auto& field : typeDecl.getFields()) {
        if (field.getType().getDeinitializer() != nullptr) {
            auto deinitializer = llvm::make_unique<DeinitDecl>(const_cast<TypeDecl&>(typeDecl), typeDecl.getLocation());
            deinitializer->setBody({});
            helperDecls.push_back(std::move(deinitializer));
            return llvm::cast<DeinitDecl>(helperDecls.back().get());
        }
    }

    return nullptr;
}

void IRGenerator::deferDeinitCall(llvm::Value* valueToDeinit, Type type, const Decl* decl) {
    llvm::Function* proto = nullptr;

    if (auto* deinitializer = type.getDeinitializer()) {
        proto = getFunctionProto(*deinitializer);
    } else if (auto* typeDecl = type.getDecl()) {
        if (auto deinitializer = getDefaultDeinitializer(*typeDecl)) {
            proto = getFunctionProto(*deinitializer);
        }
    }

    if (proto) {
        scopes.back().addDeinitToCall(proto, valueToDeinit, type, decl);
    }
}

void IRGenerator::codegenDeferredExprsAndDeinitCallsForReturn() {
    for (auto& scope : llvm::reverse(scopes)) {
        scope.onScopeEnd();
    }
    scopes.back().clear();
}

llvm::AllocaInst* IRGenerator::createEntryBlockAlloca(Type type, const Decl* decl, llvm::Value* arraySize, const llvm::Twine& name) {
    auto* insertBlock = builder.GetInsertBlock();
    auto* entryBlock = &insertBlock->getParent()->getEntryBlock();

    if (lastAlloca == llvm::BasicBlock::iterator() || lastAlloca->getParent() != entryBlock) {
        if (entryBlock->empty()) {
            builder.SetInsertPoint(entryBlock);
        } else {
            builder.SetInsertPoint(&entryBlock->front());
        }
    } else {
        builder.SetInsertPoint(entryBlock, std::next(lastAlloca));
    }

    auto* llvmType = toIR(type, decl ? decl->getLocation() : SourceLocation());
    auto* alloca = builder.CreateAlloca(llvmType, arraySize, name);
    lastAlloca = alloca->getIterator();
    auto nameString = name.str();
    if (!nameString.empty()) {
        setLocalValue(type, std::move(nameString), alloca, decl);
    }
    builder.SetInsertPoint(insertBlock);
    return alloca;
}

llvm::Value* IRGenerator::createLoad(llvm::Value* value) {
    return builder.CreateLoad(value, value->getName() + ".load");
}

llvm::Value* IRGenerator::codegenAssignmentLHS(const Expr* lhs, const Expr* rhs) {
    if (auto* initDecl = llvm::dyn_cast<InitDecl>(currentDecl)) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(lhs)) {
            if (auto* fieldDecl = llvm::dyn_cast<FieldDecl>(varExpr->getDecl())) {
                if (fieldDecl->getParent() == initDecl->getTypeDecl()) {
                    return rhs->isUndefinedLiteralExpr() ? nullptr : codegenLvalueExpr(*lhs);
                }
            }
        }
    }

    if (auto* basicType = llvm::dyn_cast<BasicType>(lhs->getType().getBase())) {
        if (auto* typeDecl = basicType->getDecl()) {
            if (auto* deinit = typeDecl->getDeinitializer()) {
                llvm::Value* value = codegenLvalueExpr(*lhs);
                createDeinitCall(getFunctionProto(*deinit), value, lhs->getType(), typeDecl);
                return rhs->isUndefinedLiteralExpr() ? nullptr : value;
            }
        }
    }

    return rhs->isUndefinedLiteralExpr() ? nullptr : codegenLvalueExpr(*lhs);
}

void IRGenerator::createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit, Type type, const Decl* decl) {
    if (!valueToDeinit->getType()->isPointerTy()) {
        auto* alloca = createEntryBlockAlloca(type, decl);
        builder.CreateStore(valueToDeinit, alloca);
        valueToDeinit = alloca;
    }

    builder.CreateCall(deinit, valueToDeinit);
}

llvm::Type* IRGenerator::getLLVMTypeForPassing(const TypeDecl& typeDecl, bool isMutating) {
    auto* structType = toIR(typeDecl.getType());

    if (!isMutating && typeDecl.passByValue()) {
        return structType;
    } else {
        return llvm::PointerType::get(structType, 0);
    }
}

llvm::Value* IRGenerator::getFunctionForCall(const CallExpr& call) {
    if (!call.callsNamedFunction()) {
        error(call.getLocation(), "anonymous function calls not implemented yet");
    }

    const Decl* decl = call.getCalleeDecl();
    if (!decl) return nullptr;

    switch (decl->getKind()) {
        case DeclKind::FunctionDecl:
        case DeclKind::MethodDecl:
        case DeclKind::InitDecl:
        case DeclKind::DeinitDecl:
            return getFunctionProto(*llvm::cast<FunctionDecl>(decl));
        case DeclKind::VarDecl:
            return findValue(llvm::cast<VarDecl>(decl)->getName(), decl);
        case DeclKind::ParamDecl:
            return findValue(llvm::cast<ParamDecl>(decl)->getName(), decl);
        case DeclKind::FieldDecl:
            if (call.getReceiver()) {
                return codegenMemberAccess(codegenLvalueExpr(*call.getReceiver()), llvm::cast<FieldDecl>(decl)->getType(),
                                           llvm::cast<FieldDecl>(decl)->getName());
            } else {
                return findValue(llvm::cast<FieldDecl>(decl)->getName(), decl);
            }
        default:
            llvm_unreachable("invalid callee decl");
    }
}

llvm::Module& IRGenerator::compile(const Module& sourceModule) {
    ASSERT(!module);
    module = llvm::make_unique<llvm::Module>(sourceModule.getName(), ctx);

    for (const auto& sourceFile : sourceModule.getSourceFiles()) {
        for (const auto& decl : sourceFile.getTopLevelDecls()) {
            codegenDecl(*decl);
        }
    }

    while (true) {
        auto currentFunctionInstantiations = functionInstantiations;

        for (auto& p : currentFunctionInstantiations) {
            if (p.second.getDecl().isExtern() || !p.second.getFunction()->empty()) continue;

            currentDecl = &p.second.getDecl();
            codegenFunctionBody(p.second.getDecl(), *p.second.getFunction());
            ASSERT(!llvm::verifyFunction(*p.second.getFunction(), &llvm::errs()));
        }

        if (functionInstantiations.size() == currentFunctionInstantiations.size()) break;
    }

    ASSERT(!llvm::verifyModule(*module, &llvm::errs()));
    generatedModules.push_back(std::move(module));
    return *generatedModules.back();
}
