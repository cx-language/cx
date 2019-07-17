#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringSwitch.h>
#include <llvm/IR/Verifier.h>
#pragma warning(pop)
#include "../ast/module.h"

using namespace delta;

void IRGenScope::onScopeEnd() {
    for (const Expr* expr : llvm::reverse(deferredExprs)) {
        irGenerator->codegenExpr(*expr);
    }

    for (auto& p : llvm::reverse(deinitsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        irGenerator->createDeinitCall(p.function, p.value);
    }
}

void IRGenScope::clear() {
    deferredExprs.clear();
    deinitsToCall.clear();
}

IRGenerator::IRGenerator() : builder(ctx) {
    scopes.push_back(IRGenScope(*this));
}

void IRGenerator::setLocalValue(llvm::Value* value, const VariableDecl* decl) {
    auto it = scopes.back().valuesByDecl.try_emplace(decl, value);
    ASSERT(it.second);

    if (decl) {
        deferDeinitCall(value, decl);
    }
}

llvm::Value* IRGenerator::getValueOrNull(const Decl* decl) {
    for (auto& scope : llvm::reverse(scopes)) {
        if (auto* value = scope.valuesByDecl.lookup(decl)) {
            return value;
        }
    }

    return nullptr;
}

llvm::Value* IRGenerator::getValue(const Decl* decl) {
    if (auto* value = getValueOrNull(decl)) {
        return value;
    }

    switch (decl->getKind()) {
        case DeclKind::VarDecl:
            return codegenVarDecl(*llvm::cast<VarDecl>(decl));
        case DeclKind::FieldDecl: {
            auto* fieldDecl = llvm::cast<FieldDecl>(decl);
            return codegenMemberAccess(getThis(), fieldDecl->getType(), fieldDecl->getName());
        }
        case DeclKind::FunctionDecl:
            return getFunctionProto(*llvm::cast<FunctionDecl>(decl));
        default:
            llvm_unreachable("all cases handled");
    }
}

llvm::Value* IRGenerator::getThis() {
    return getValue(nullptr);
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
    scopes.push_back(IRGenScope(*this));
}

void IRGenerator::endScope() {
    scopes.back().onScopeEnd();
    scopes.pop_back();
}

void IRGenerator::deferEvaluationOf(const Expr& expr) {
    scopes.back().deferredExprs.push_back(&expr);
}

/// Returns a deinitializer that only calls the deinitializers of the member variables, or null if
/// no such deinitializer is needed because none of the member variables have deinitializers.
DeinitDecl* IRGenerator::getDefaultDeinitializer(const TypeDecl& typeDecl) {
    ASSERT(typeDecl.getDeinitializer() == nullptr);

    for (auto& field : typeDecl.getFields()) {
        if (field.getType().getDeinitializer() != nullptr) {
            auto deinitializer = new DeinitDecl(const_cast<TypeDecl&>(typeDecl), typeDecl.getLocation());
            deinitializer->setBody({});
            return deinitializer;
        }
    }

    return nullptr;
}

void IRGenerator::deferDeinitCall(llvm::Value* valueToDeinit, const VariableDecl* decl) {
    auto type = decl->getType();
    llvm::Function* proto = nullptr;

    if (auto* deinitializer = type.getDeinitializer()) {
        proto = getFunctionProto(*deinitializer);
    } else if (auto* typeDecl = type.getDecl()) {
        if (auto deinitializer = getDefaultDeinitializer(*typeDecl)) {
            proto = getFunctionProto(*deinitializer);
        }
    }

    if (proto) {
        scopes.back().deinitsToCall.push_back({ proto, valueToDeinit, decl });
    }
}

void IRGenerator::codegenDeferredExprsAndDeinitCallsForReturn() {
    for (auto& scope : llvm::reverse(scopes)) {
        scope.onScopeEnd();
    }
    scopes.back().clear();
}

llvm::AllocaInst* IRGenerator::createEntryBlockAlloca(llvm::Type* type, llvm::Value* arraySize, const llvm::Twine& name) {
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

    auto* alloca = builder.CreateAlloca(type, arraySize, name);
    lastAlloca = alloca->getIterator();
    builder.SetInsertPoint(insertBlock);
    return alloca;
}

llvm::AllocaInst* IRGenerator::createTempAlloca(llvm::Value* value, const llvm::Twine& name) {
    auto* alloca = createEntryBlockAlloca(value->getType(), nullptr, name);
    builder.CreateStore(value, alloca);
    return alloca;
}

llvm::Value* IRGenerator::createLoad(llvm::Value* value) {
    return builder.CreateLoad(value, value->getName() + ".load");
}

llvm::Value* IRGenerator::codegenAssignmentLHS(const Expr& lhs) {
    // Don't call deinitializer for LHS when assigning to fields in initializer.
    if (auto* initDecl = llvm::dyn_cast<InitDecl>(currentDecl)) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(&lhs)) {
            if (auto* fieldDecl = llvm::dyn_cast<FieldDecl>(varExpr->getDecl())) {
                if (fieldDecl->getParent() == initDecl->getTypeDecl()) {
                    return codegenLvalueExpr(lhs);
                }
            }
        }
    }

    // Call deinitializer for LHS.
    if (auto* basicType = llvm::dyn_cast<BasicType>(lhs.getType().getBase())) {
        if (auto* typeDecl = basicType->getDecl()) {
            if (auto* deinit = typeDecl->getDeinitializer()) {
                llvm::Value* value = codegenLvalueExpr(lhs);
                createDeinitCall(getFunctionProto(*deinit), value);
                return value;
            }
        }
    }

    return codegenLvalueExpr(lhs);
}

void IRGenerator::createDeinitCall(llvm::Function* deinit, llvm::Value* valueToDeinit) {
    if (!valueToDeinit->getType()->isPointerTy()) {
        valueToDeinit = createTempAlloca(valueToDeinit);
    }
    builder.CreateCall(deinit, valueToDeinit);
}

llvm::Type* IRGenerator::getLLVMTypeForPassing(const TypeDecl& typeDecl) {
    return llvm::PointerType::get(toIR(typeDecl.getType()), 0);
}

llvm::Value* IRGenerator::getFunctionForCall(const CallExpr& call) {
    if (!call.callsNamedFunction()) {
        ERROR(call.getLocation(), "anonymous function calls not implemented yet");
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
        case DeclKind::ParamDecl:
            return getValue(decl);
        case DeclKind::FieldDecl:
            if (call.getReceiver()) {
                return codegenMemberAccess(codegenLvalueExpr(*call.getReceiver()), llvm::cast<FieldDecl>(decl)->getType(),
                                           llvm::cast<FieldDecl>(decl)->getName());
            } else {
                return getValue(decl);
            }
        default:
            llvm_unreachable("invalid callee decl");
    }
}

llvm::Module& IRGenerator::codegenModule(const Module& sourceModule) {
    ASSERT(!module);
    module = new llvm::Module(sourceModule.getName(), ctx);

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
    generatedModules.push_back(module);
    module = nullptr;
    return *generatedModules.back();
}
