#include "llvm.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringSwitch.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Verifier.h>
#pragma warning(pop)
#include "ir.h"

using namespace delta;

llvm::Type* LLVMGenerator::getBuiltinType(llvm::StringRef name) {
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

llvm::Type* LLVMGenerator::getStructType(IRStructType* type) {
    if (type->name.empty()) {
        auto elementTypes = map(type->elementTypes, [&](IRType* type) { return getLLVMType(type); });
        return llvm::StructType::get(ctx, std::move(elementTypes));
    }

    auto it = structs.find(type);
    if (it != structs.end()) return NOTNULL(it->second);

    // TODO: Always call StructType::create here?
    auto llvmStruct = type->elementTypes.empty() ? llvm::StructType::get(ctx) : llvm::StructType::create(ctx, type->getName());
    structs.try_emplace(type, llvmStruct);
    auto elementTypes = map(type->elementTypes, [&](IRType* type) { return getLLVMType(type); });
    llvmStruct->setBody(std::move(elementTypes));
    return llvmStruct;
}

llvm::Type* LLVMGenerator::getLLVMType(IRType* type) {
    switch (type->kind) {
        case IRTypeKind::IRBasicType: {
            return NOTNULL(getBuiltinType(type->getName()));
        }
        case IRTypeKind::IRArrayType: {
            auto arrayType = llvm::cast<IRArrayType>(type);
            return llvm::ArrayType::get(getLLVMType(arrayType->elementType), arrayType->size);
        }
        case IRTypeKind::IRFunctionType: {
            auto functionType = llvm::cast<IRFunctionType>(type);
            auto returnType = getLLVMType(functionType->returnType);
            auto paramTypes = map(functionType->paramTypes, [&](IRType* type) { return getLLVMType(type); });
            return llvm::FunctionType::get(returnType, paramTypes, false);
        }
        case IRTypeKind::IRPointerType: {
            auto pointerType = llvm::cast<IRPointerType>(type);
            auto* pointeeType = getLLVMType(pointerType->pointee);
            return llvm::PointerType::get(pointeeType->isVoidTy() ? llvm::Type::getInt8Ty(ctx) : pointeeType, 0);
        }
        case IRTypeKind::IRStructType: {
            auto structType = llvm::cast<IRStructType>(type);
            return getStructType(structType);
        }
        case IRTypeKind::IRUnionType: {
            auto unionType = llvm::cast<IRUnionType>(type);

            auto it = structs.find(unionType);
            if (it != structs.end()) return it->second;

            auto structType = unionType->name.empty() ? llvm::StructType::get(ctx) : llvm::StructType::create(ctx, unionType->name);
            structs.try_emplace(unionType, structType);

            unsigned maxSize = 0;
            for (auto* field : unionType->getElements()) {
                if (!field) continue;
                auto size = module->getDataLayout().getTypeAllocSize(getLLVMType(field));
                if (size > maxSize) maxSize = size;
            }

            structType->setBody(llvm::ArrayType::get(llvm::Type::getInt8Ty(ctx), maxSize));
            return structType;
        }
    }

    llvm_unreachable("all cases handled");
}

llvm::Function* LLVMGenerator::getFunction(const Function* function) {
    if (auto* llvmFunction = module->getFunction(function->mangledName)) return llvmFunction;

    llvm::SmallVector<llvm::Type*, 16> paramTypes;
    for (auto& param : function->params) {
        paramTypes.emplace_back(getLLVMType(param.type));
    }

    auto* returnType = getLLVMType(function->returnType);
    auto* functionType = llvm::FunctionType::get(returnType, paramTypes, function->isVariadic);
    auto* llvmFunction = llvm::Function::Create(functionType, llvm::Function::ExternalLinkage, function->mangledName, &*module);

    auto arg = llvmFunction->arg_begin(), argsEnd = llvmFunction->arg_end();
    ASSERT(function->params.size() == size_t(std::distance(arg, argsEnd)));
    for (auto param = function->params.begin(); arg != argsEnd; ++param, ++arg) {
        arg->setName(param->name);
    }

    return llvmFunction;
}

void LLVMGenerator::codegenFunctionBody(const Function* function, llvm::Function* llvmFunction) {
    llvm::IRBuilder<>::InsertPointGuard insertPointGuard(builder);

    auto arg = llvmFunction->arg_begin();
    for (auto& param : function->params) {
        generatedValues.emplace(&param, &*arg++);
    }

    for (auto* block : function->body) {
        auto llvmBlock = getBasicBlock(block);
        llvmBlock->insertInto(llvmFunction);
        builder.SetInsertPoint(llvmBlock);

        for (auto* inst : block->body) {
            getValue(inst);
        }
    }

    auto insertBlock = builder.GetInsertBlock();
    if (insertBlock && insertBlock != &llvmFunction->getEntryBlock() && llvm::pred_empty(insertBlock)) {
        insertBlock->eraseFromParent();
    }
}

void LLVMGenerator::codegenFunction(const Function* function) {
    auto llvmFunction = getFunction(function);

    if (!function->isExtern && llvmFunction->empty()) {
        codegenFunctionBody(function, llvmFunction);
    }

#ifndef NDEBUG
    if (llvm::verifyFunction(*llvmFunction, &llvm::errs())) {
        llvm::errs() << '\n';
        llvmFunction->print(llvm::errs(), nullptr, false, true);
        llvm::errs() << '\n';
        ASSERT(false && "llvm::verifyFunction failed");
    }
#endif
}

llvm::BasicBlock* LLVMGenerator::getBasicBlock(const BasicBlock* block) {
    return llvm::cast<llvm::BasicBlock>(getValue(block));
}

llvm::Value* LLVMGenerator::codegenAlloca(const AllocaInst* inst) {
    return builder.CreateAlloca(getLLVMType(inst->allocatedType), nullptr, inst->name);
}

llvm::Value* LLVMGenerator::codegenReturn(const ReturnInst* inst) {
    return inst->value ? builder.CreateRet(getValue(inst->value)) : builder.CreateRetVoid();
}

llvm::Value* LLVMGenerator::codegenBranch(const BranchInst* inst) {
    return builder.CreateBr(getBasicBlock(inst->destination));
}

llvm::Value* LLVMGenerator::codegenCondBranch(const CondBranchInst* inst) {
    auto condition = getValue(inst->condition);
    auto trueBlock = getBasicBlock(inst->trueBlock);
    auto falseBlock = getBasicBlock(inst->falseBlock);
    return builder.CreateCondBr(condition, trueBlock, falseBlock);
}

llvm::Value* LLVMGenerator::codegenPhi(const PhiInst* inst) {
    auto type = getLLVMType(inst->valuesAndPredecessors[0].first->getType());
    auto phi = builder.CreatePHI(type, (unsigned) inst->valuesAndPredecessors.size(), inst->name);
    for (auto& p : inst->valuesAndPredecessors) {
        auto value = getValue(p.first);
        auto block = getBasicBlock(p.second);
        phi->addIncoming(value, block);
    }
    return phi;
}

llvm::Value* LLVMGenerator::codegenSwitch(const SwitchInst* inst) {
    auto condition = getValue(inst->condition);
    auto cases = map(inst->cases, [&](auto& p) {
        auto value = llvm::cast<llvm::ConstantInt>(getValue(p.first));
        auto block = getBasicBlock(p.second);
        return std::make_pair(value, block);
    });
    auto defaultBlock = getBasicBlock(inst->defaultBlock);
    auto switchInst = builder.CreateSwitch(condition, defaultBlock);
    for (auto& [value, block] : cases) {
        switchInst->addCase(value, block);
    }
    return switchInst;
}

llvm::Value* LLVMGenerator::codegenLoad(const LoadInst* inst) {
    return builder.CreateLoad(getValue(inst->value), inst->name);
}

llvm::Value* LLVMGenerator::codegenStore(const StoreInst* inst) {
    auto value = getValue(inst->value);
    auto pointer = getValue(inst->pointer);
    return builder.CreateStore(value, pointer);
}

llvm::Value* LLVMGenerator::codegenInsert(const InsertInst* inst) {
    auto aggregate = getValue(inst->aggregate);
    auto value = getValue(inst->value);
    return builder.CreateInsertValue(aggregate, value, inst->index);
}

llvm::Value* LLVMGenerator::codegenExtract(const ExtractInst* inst) {
    auto aggregate = getValue(inst->aggregate);
    return builder.CreateExtractValue(aggregate, inst->index, inst->name);
}

llvm::Value* LLVMGenerator::codegenCall(const CallInst* inst) {
    auto function = getValue(inst->function);
    auto args = map(inst->args, [&](auto* arg) { return getValue(arg); });
    ASSERT(function->getType()->isFunctionTy() || (function->getType()->isPointerTy() && function->getType()->getPointerElementType()->isFunctionTy()));
    return builder.CreateCall(function, args);
}

llvm::Value* LLVMGenerator::codegenBinary(const BinaryInst* inst) {
    auto left = getValue(inst->left);
    auto right = getValue(inst->right);
    auto isFloat = inst->left->getType()->isFloatingPoint();
    auto isSigned = inst->left->getType()->isSignedInteger();

    switch (inst->op) {
        case Token::Plus:
            if (isFloat) return builder.CreateFAdd(left, right);
            return builder.CreateAdd(left, right);
        case Token::Minus:
            if (isFloat) return builder.CreateFSub(left, right);
            return builder.CreateSub(left, right);
        case Token::Star:
            if (isFloat) return builder.CreateFMul(left, right);
            return builder.CreateMul(left, right);
        case Token::Slash:
            if (isFloat) return builder.CreateFDiv(left, right);
            if (isSigned) return builder.CreateSDiv(left, right);
            return builder.CreateUDiv(left, right);
        case Token::Equal:
            if (isFloat) return builder.CreateFCmpOEQ(left, right);
            LLVM_FALLTHROUGH;
        case Token::PointerEqual:
            return builder.CreateICmpEQ(left, right, inst->name);
        case Token::NotEqual:
            if (isFloat) return builder.CreateFCmpONE(left, right);
            LLVM_FALLTHROUGH;
        case Token::PointerNotEqual:
            return builder.CreateICmpNE(left, right);
        case Token::Less:
            if (isFloat) return builder.CreateFCmpOLT(left, right);
            if (isSigned) return builder.CreateICmpSLT(left, right);
            return builder.CreateICmpULT(left, right);
        case Token::LessOrEqual:
            if (isFloat) return builder.CreateFCmpOLE(left, right);
            if (isSigned) return builder.CreateICmpSLE(left, right);
            return builder.CreateICmpULE(left, right);
        case Token::Greater:
            if (isFloat) return builder.CreateFCmpOGT(left, right);
            if (isSigned) return builder.CreateICmpSGT(left, right);
            return builder.CreateICmpUGT(left, right);
        case Token::GreaterOrEqual:
            if (isFloat) return builder.CreateFCmpOGE(left, right);
            if (isSigned) return builder.CreateICmpSGE(left, right);
            return builder.CreateICmpUGE(left, right);
        case Token::Modulo:
            if (isFloat) return builder.CreateFRem(left, right);
            if (isSigned) return builder.CreateSRem(left, right);
            return builder.CreateURem(left, right);
        case Token::RightShift:
            if (isSigned) return builder.CreateAShr(left, right);
            return builder.CreateLShr(left, right);
        case Token::And:
            return builder.CreateAnd(left, right);
        case Token::Or:
            return builder.CreateOr(left, right);
        case Token::Xor:
            return builder.CreateXor(left, right);
        case Token::LeftShift:
            return builder.CreateShl(left, right);
        default:
            llvm_unreachable("invalid binary operation");
    }
}

llvm::Value* LLVMGenerator::codegenUnary(const UnaryInst* inst) {
    auto operand = getValue(inst->operand);
    auto isFloat = inst->operand->getType()->isFloatingPoint();

    switch (inst->op) {
        case Token::Minus:
            if (isFloat) return builder.CreateFNeg(operand);
            return builder.CreateNeg(operand);
        case Token::Not:
            return builder.CreateNot(operand);
        default:
            llvm_unreachable("invalid unary operation");
    }
}

llvm::Value* LLVMGenerator::codegenGEP(const GEPInst* inst) {
    auto pointer = getValue(inst->pointer);
    auto indexes = map(inst->indexes, [&](auto* index) { return getValue(index); });
    return builder.CreateInBoundsGEP(pointer, indexes, inst->name);
}

llvm::Value* LLVMGenerator::codegenConstGEP(const ConstGEPInst* inst) {
    auto pointer = getValue(inst->pointer);
    return builder.CreateConstInBoundsGEP2_32(nullptr, pointer, inst->index0, inst->index1, inst->name);
}

llvm::Value* LLVMGenerator::codegenCast(const CastInst* inst) {
    auto value = getValue(inst->value);
    auto sourceType = inst->value->getType();
    auto type = inst->type;

    if (sourceType->isUnsignedInteger() && type->isInteger()) {
        return builder.CreateZExtOrTrunc(value, getLLVMType(type));
    }

    if (sourceType->isSignedInteger() && type->isInteger()) {
        return builder.CreateSExtOrTrunc(value, getLLVMType(type));
    }

    if (sourceType->isInteger() || sourceType->isChar() || sourceType->isBool()) {
        if (type->isInteger() || type->isChar()) return builder.CreateIntCast(value, getLLVMType(type), sourceType->isSignedInteger());
        if (type->isBool()) return builder.CreateIsNotNull(value);
    }

    if (sourceType->isFloatingPoint()) {
        if (type->isSignedInteger()) return builder.CreateFPToSI(value, getLLVMType(type));
        if (type->isUnsignedInteger()) return builder.CreateFPToUI(value, getLLVMType(type));
        if (type->isFloatingPoint()) return builder.CreateFPCast(value, getLLVMType(type));
    }

    if (type->isFloatingPoint()) {
        if (sourceType->isSignedInteger()) return builder.CreateSIToFP(value, getLLVMType(type));
        if (sourceType->isUnsignedInteger()) return builder.CreateUIToFP(value, getLLVMType(type));
    }

    return builder.CreateBitOrPointerCast(value, getLLVMType(type), inst->name);
}

llvm::Value* LLVMGenerator::codegenUnreachable() {
    return builder.CreateUnreachable();
}

llvm::Value* LLVMGenerator::codegenSizeof(const SizeofInst* inst) {
    return llvm::ConstantExpr::getSizeOf(getLLVMType(inst->type));
}

llvm::Value* LLVMGenerator::codegenBasicBlock(const BasicBlock* block) {
    return llvm::BasicBlock::Create(ctx, block->name);
}

llvm::Value* LLVMGenerator::codegenGlobalVariable(const GlobalVariable* inst) {
    auto linkage = inst->value ? llvm::GlobalValue::PrivateLinkage : llvm::GlobalValue::ExternalLinkage;
    auto initializer = inst->value ? llvm::cast<llvm::Constant>(getValue(inst->value)) : nullptr;
    return new llvm::GlobalVariable(*module, getLLVMType(inst->value->getType()), false, linkage, initializer, inst->name);
}

llvm::Value* LLVMGenerator::codegenConstantString(const ConstantString* inst) {
    return builder.CreateGlobalStringPtr(inst->value);
}

llvm::Value* LLVMGenerator::codegenConstantInt(const ConstantInt* inst) {
    auto type = getLLVMType(inst->type);
    return llvm::ConstantInt::get(type, inst->value.extOrTrunc(type->getIntegerBitWidth()));
}

llvm::Value* LLVMGenerator::codegenConstantFP(const ConstantFP* inst) {
    llvm::SmallString<128> buffer;
    inst->value.toString(buffer);
    return llvm::ConstantFP::get(getLLVMType(inst->type), buffer);
}

llvm::Value* LLVMGenerator::codegenConstantBool(const ConstantBool* inst) {
    return inst->value ? llvm::ConstantInt::getTrue(ctx) : llvm::ConstantInt::getFalse(ctx);
}

llvm::Value* LLVMGenerator::codegenConstantNull(const ConstantNull* inst) {
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(getLLVMType(inst->type)));
}

llvm::Value* LLVMGenerator::codegenUndefined(const Undefined* inst) {
    return llvm::UndefValue::get(getLLVMType(inst->type));
}

llvm::Value* LLVMGenerator::getValue(const Value* value) {
    auto it = generatedValues.find(value);
    if (it != generatedValues.end()) return it->second;
    auto llvmValue = codegenInst(value);
    generatedValues.emplace(value, llvmValue);
    return llvmValue;
}

llvm::Value* LLVMGenerator::codegenInst(const Value* value) {
    switch (value->kind) {
        case ValueKind::AllocaInst:
            return codegenAlloca(llvm::cast<AllocaInst>(value));
        case ValueKind::ReturnInst:
            return codegenReturn(llvm::cast<ReturnInst>(value));
        case ValueKind::BranchInst:
            return codegenBranch(llvm::cast<BranchInst>(value));
        case ValueKind::CondBranchInst:
            return codegenCondBranch(llvm::cast<CondBranchInst>(value));
        case ValueKind::PhiInst:
            return codegenPhi(llvm::cast<PhiInst>(value));
        case ValueKind::SwitchInst:
            return codegenSwitch(llvm::cast<SwitchInst>(value));
        case ValueKind::LoadInst:
            return codegenLoad(llvm::cast<LoadInst>(value));
        case ValueKind::StoreInst:
            return codegenStore(llvm::cast<StoreInst>(value));
        case ValueKind::InsertInst:
            return codegenInsert(llvm::cast<InsertInst>(value));
        case ValueKind::ExtractInst:
            return codegenExtract(llvm::cast<ExtractInst>(value));
        case ValueKind::CallInst:
            return codegenCall(llvm::cast<CallInst>(value));
        case ValueKind::BinaryInst:
            return codegenBinary(llvm::cast<BinaryInst>(value));
        case ValueKind::UnaryInst:
            return codegenUnary(llvm::cast<UnaryInst>(value));
        case ValueKind::GEPInst:
            return codegenGEP(llvm::cast<GEPInst>(value));
        case ValueKind::ConstGEPInst:
            return codegenConstGEP(llvm::cast<ConstGEPInst>(value));
        case ValueKind::CastInst:
            return codegenCast(llvm::cast<CastInst>(value));
        case ValueKind::UnreachableInst:
            return codegenUnreachable();
        case ValueKind::SizeofInst:
            return codegenSizeof(llvm::cast<SizeofInst>(value));
        case ValueKind::BasicBlock:
            return codegenBasicBlock(llvm::cast<BasicBlock>(value));
        case ValueKind::Function:
            return getFunction(llvm::cast<Function>(value));
        case ValueKind::Parameter:
            return generatedValues.at(value);
        case ValueKind::GlobalVariable:
            return codegenGlobalVariable(llvm::cast<GlobalVariable>(value));
        case ValueKind::ConstantString:
            return codegenConstantString(llvm::cast<ConstantString>(value));
        case ValueKind::ConstantInt:
            return codegenConstantInt(llvm::cast<ConstantInt>(value));
        case ValueKind::ConstantFP:
            return codegenConstantFP(llvm::cast<ConstantFP>(value));
        case ValueKind::ConstantBool:
            return codegenConstantBool(llvm::cast<ConstantBool>(value));
        case ValueKind::ConstantNull:
            return codegenConstantNull(llvm::cast<ConstantNull>(value));
        case ValueKind::Undefined:
            return codegenUndefined(llvm::cast<Undefined>(value));
    }

    llvm_unreachable("all cases handled");
}

llvm::Module& LLVMGenerator::codegenModule(const IRModule& sourceModule) {
    ASSERT(!module);
    module = new llvm::Module(sourceModule.name, ctx);

    for (auto* globalVariable : sourceModule.globalVariables) {
        getValue(globalVariable);
    }

    for (auto* function : sourceModule.functions) {
        codegenFunction(function);
    }

    ASSERT(!llvm::verifyModule(*module, &llvm::errs()));
    generatedModules.push_back(module);
    module = nullptr;
    return *generatedModules.back();
}
