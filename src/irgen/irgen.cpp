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

    for (auto& p : llvm::reverse(destructorsToCall)) {
        if (p.decl && p.decl->hasBeenMoved()) continue;
        irGenerator->createDestructorCall(p.function, p.value);
    }
}

void IRGenScope::clear() {
    deferredExprs.clear();
    destructorsToCall.clear();
}

IRGenerator::IRGenerator() : builder(ctx) {
    scopes.push_back(IRGenScope(*this));
}

void IRGenerator::setLocalValue(llvm::Value* value, const VariableDecl* decl) {
    auto it = scopes.back().valuesByDecl.try_emplace(decl, value);
    ASSERT(it.second);

    if (decl) {
        deferDestructorCall(value, decl);
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

llvm::Value* IRGenerator::getThis(llvm::Type* targetType) {
    auto value = getValue(nullptr);

    if (targetType) {
        value = builder.CreatePointerCast(value, targetType);
    }

    return value;
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

llvm::Type* IRGenerator::getEnumType(const EnumDecl& enumDecl) {
    auto* tagType = getLLVMType(enumDecl.getTagType());
    if (!enumDecl.hasAssociatedValues()) return tagType;

    auto structType = llvm::StructType::create(ctx, enumDecl.getQualifiedName());
    structs.try_emplace(structType->getName(), std::make_pair(structType, &enumDecl));

    unsigned maxSize = 0;
    for (auto& enumCase : enumDecl.getCases()) {
        if (auto associatedType = enumCase.getAssociatedType()) {
            auto size = module->getDataLayout().getTypeAllocSize(getLLVMType(associatedType));
            if (size > maxSize) maxSize = size;
        }
    }

    structType->setBody(tagType, llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), maxSize));
    return structType;
}

llvm::Type* IRGenerator::getLLVMType(Type type, SourceLocation location) {
    switch (type.getKind()) {
        case TypeKind::BasicType: {
            if (auto* builtinType = getBuiltinType(type.getName())) return builtinType;

            auto it = structs.find(type.getQualifiedTypeName());
            if (it != structs.end()) return it->second.first;

            if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(type.getDecl())) {
                return getEnumType(*enumDecl);
            } else {
                return codegenTypeDecl(*type.getDecl());
            }
        }
        case TypeKind::ArrayType:
            switch (type.getArraySize()) {
                case ArrayType::runtimeSize:
                    return getLLVMType(BasicType::get("ArrayRef", type.getElementType()), location);
                case ArrayType::unknownSize:
                    return llvm::PointerType::get(getLLVMType(type.getElementType(), location), 0);
                default:
                    return llvm::ArrayType::get(getLLVMType(type.getElementType(), location), type.getArraySize());
            }
        case TypeKind::TupleType: {
            auto elementTypes = map(type.getTupleElements(), [&](const TupleElement& element) { return getLLVMType(element.type); });
            return llvm::StructType::get(ctx, elementTypes);
        }
        case TypeKind::FunctionType: {
            auto paramTypes = map(type.getParamTypes(), [&](Type type) { return getLLVMType(type); });
            auto* returnType = getLLVMType(type.getReturnType());
            return llvm::FunctionType::get(returnType, paramTypes, false)->getPointerTo();
        }
        case TypeKind::PointerType: {
            auto* pointeeType = getLLVMType(type.getPointee(), location);
            return llvm::PointerType::get(pointeeType->isVoidTy() ? llvm::Type::getInt8Ty(ctx) : pointeeType, 0);
        }
        case TypeKind::OptionalType:
            if (type.getWrappedType().isPointerType() || type.getWrappedType().isFunctionType() ||
                type.getWrappedType().isArrayWithUnknownSize()) {
                return getLLVMType(type.getWrappedType());
            }
            llvm_unreachable("IRGen doesn't support non-pointer optional types yet");

        case TypeKind::UnresolvedType:
            llvm_unreachable("invalid unresolved type");
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

/// Returns a destructor that only calls the destructors of the member variables, or null if
/// no such destructor is needed because none of the member variables have destructors.
DestructorDecl* IRGenerator::getDefaultDestructor(const TypeDecl& typeDecl) {
    ASSERT(typeDecl.getDestructor() == nullptr);

    for (auto& field : typeDecl.getFields()) {
        if (field.getType().getDestructor() != nullptr) {
            auto destructor = new DestructorDecl(const_cast<TypeDecl&>(typeDecl), typeDecl.getLocation());
            destructor->setBody({});
            return destructor;
        }
    }

    return nullptr;
}

void IRGenerator::deferDestructorCall(llvm::Value* receiver, const VariableDecl* decl) {
    auto type = decl->getType();
    llvm::Function* proto = nullptr;

    if (auto* destructor = type.getDestructor()) {
        proto = getFunctionProto(*destructor);
    } else if (auto* typeDecl = type.getDecl()) {
        if (auto defaultDestructor = getDefaultDestructor(*typeDecl)) {
            proto = getFunctionProto(*defaultDestructor);
        }
    }

    if (proto) {
        scopes.back().destructorsToCall.push_back({ proto, receiver, decl });
    }
}

void IRGenerator::codegenDeferredExprsAndDestructorCallsForReturn() {
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
    // Don't call destructor for LHS when assigning to fields in constructor.
    if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(currentDecl)) {
        if (auto* varExpr = llvm::dyn_cast<VarExpr>(&lhs)) {
            if (auto* fieldDecl = llvm::dyn_cast<FieldDecl>(varExpr->getDecl())) {
                if (fieldDecl->getParent() == constructorDecl->getTypeDecl()) {
                    return codegenLvalueExpr(lhs);
                }
            }
        }
    }

    // Call destructor for LHS.
    if (auto* basicType = llvm::dyn_cast<BasicType>(lhs.getType().getBase())) {
        if (auto* typeDecl = basicType->getDecl()) {
            if (auto* destructor = typeDecl->getDestructor()) {
                llvm::Value* value = codegenLvalueExpr(lhs);
                createDestructorCall(getFunctionProto(*destructor), value);
                return value;
            }
        }
    }

    return codegenLvalueExpr(lhs);
}

void IRGenerator::createDestructorCall(llvm::Function* destructor, llvm::Value* receiver) {
    if (!receiver->getType()->isPointerTy()) {
        receiver = createTempAlloca(receiver);
    }
    builder.CreateCall(destructor, receiver);
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
        case DeclKind::ConstructorDecl:
        case DeclKind::DestructorDecl:
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

        for (auto& instantiation : currentFunctionInstantiations) {
            if (instantiation.decl->isExtern() || !instantiation.function->empty()) continue;

            currentDecl = instantiation.decl;
            codegenFunctionBody(*instantiation.decl, *instantiation.function);
            ASSERT(!llvm::verifyFunction(*instantiation.function, &llvm::errs()));
        }

        if (functionInstantiations.size() == currentFunctionInstantiations.size()) break;
    }

    ASSERT(!llvm::verifyModule(*module, &llvm::errs()));
    generatedModules.push_back(module);
    module = nullptr;
    return *generatedModules.back();
}
