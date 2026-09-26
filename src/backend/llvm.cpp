#include "llvm.h"
#pragma warning(push, 0)
#include <llvm/ADT/SmallString.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/IR/CFG.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SaveAndRestore.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/TargetParser/Host.h>
#pragma warning(pop)

#include "../ast/expr.h"
#include "ir.h"

using namespace cx;

static const llvm::DataLayout& getHostDataLayout() {
    static const llvm::DataLayout* dataLayout = [] {
        llvm::InitializeNativeTarget();
        llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
        std::string errorMessage;
        auto* target = llvm::TargetRegistry::lookupTarget(triple, errorMessage);
        ASSERT(target && "couldn't lookup native target");
        llvm::TargetOptions options;
        auto* targetMachine = target->createTargetMachine(triple, "generic", "", options, llvm::Reloc::PIC_);
        ASSERT(targetMachine && "couldn't create target machine for host data layout");
        return new llvm::DataLayout(targetMachine->createDataLayout());
    }();
    return *dataLayout;
}

llvm::Type* LLVMGenerator::getBuiltinType(llvm::StringRef name) {
    // c_size_t matches C's size_t (pointer-sized); host width is target width.
    if (name == "c_size_t") return sizeof(void*) == 8 ? llvm::Type::getInt64Ty(ctx) : llvm::Type::getInt32Ty(ctx);
    // c_long matches C's long; the host long has the target width.
    if (name == "c_long" || name == "c_ulong") return sizeof(long) == 8 ? llvm::Type::getInt64Ty(ctx) : llvm::Type::getInt32Ty(ctx);
    return llvm::StringSwitch<llvm::Type*>(name)
        .Case("void", llvm::Type::getVoidTy(ctx))
        .Case("bool", llvm::Type::getInt1Ty(ctx))
        .Case("char", llvm::Type::getInt8Ty(ctx))
        .Case("int8", llvm::Type::getInt8Ty(ctx))
        .Case("int16", llvm::Type::getInt16Ty(ctx))
        .Case("int32", llvm::Type::getInt32Ty(ctx))
        .Case("int64", llvm::Type::getInt64Ty(ctx))
        .Case("int128", llvm::Type::getInt128Ty(ctx))
        .Case("uint8", llvm::Type::getInt8Ty(ctx))
        .Case("uint16", llvm::Type::getInt16Ty(ctx))
        .Case("uint32", llvm::Type::getInt32Ty(ctx))
        .Case("uint64", llvm::Type::getInt64Ty(ctx))
        .Case("uint128", llvm::Type::getInt128Ty(ctx))
        .Case("c_schar", llvm::Type::getInt8Ty(ctx))
        .Case("c_uchar", llvm::Type::getInt8Ty(ctx))
        .Case("c_short", llvm::Type::getInt16Ty(ctx))
        .Case("c_ushort", llvm::Type::getInt16Ty(ctx))
        .Case("c_int", llvm::Type::getInt32Ty(ctx))
        .Case("c_uint", llvm::Type::getInt32Ty(ctx))
        .Case("c_longlong", llvm::Type::getInt64Ty(ctx))
        .Case("c_ulonglong", llvm::Type::getInt64Ty(ctx))
        .Case("float32", llvm::Type::getFloatTy(ctx))
        .Case("float64", llvm::Type::getDoubleTy(ctx))
        .Case("float80", llvm::Type::getX86_FP80Ty(ctx))
        .Case("c_float", llvm::Type::getFloatTy(ctx))
        .Case("c_double", llvm::Type::getDoubleTy(ctx))
        .Default(nullptr);
}

llvm::Type* LLVMGenerator::getStructType(IRStructType* type) {
    auto it = structs.find(type);
    if (it != structs.end()) return NOTNULL(it->second);

    if (type->name.empty()) {
        auto fields = map(type->fields, [&](const IRField& field) { return getLLVMType(field.type); });
        auto* llvmStruct = llvm::StructType::get(ctx, std::move(fields), type->packed);
        structs.try_emplace(type, llvmStruct);
        return llvmStruct;
    }

    auto llvmStruct = llvm::StructType::create(ctx, type->getName());
    structs.try_emplace(type, llvmStruct);
    auto fields = map(type->fields, [&](const IRField& field) { return getLLVMType(field.type); });
    llvmStruct->setBody(std::move(fields), type->packed);
    return llvmStruct;
}

llvm::Type* LLVMGenerator::getLLVMType(IRType* type, bool* isSret) {
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
        std::vector<llvm::Type*> paramTypes;
        paramTypes.reserve(functionType->paramTypes.size() + 1);
        for (IRType* param : functionType->paramTypes) {
            auto paramLLVMType = getLLVMType(param);
            // Larger aggregates are passed indirectly to avoid materializing
            // large SSA copies that expand during codegen.
            if (shouldPassIndirectly(paramLLVMType)) {
                paramTypes.push_back(llvm::PointerType::get(ctx, 0));
            } else {
                paramTypes.push_back(paramLLVMType);
            }
        }
        // Use hidden sret pointer parameter to return larger structs to be compatible with the C calling convention.
        if (shouldUseSret(returnType)) {
            if (isSret) *isSret = true;
            paramTypes.insert(paramTypes.begin(), llvm::PointerType::get(ctx, 0));
            returnType = llvm::Type::getVoidTy(ctx);
        } else {
            if (isSret) *isSret = false;
        }
        return llvm::FunctionType::get(returnType, paramTypes, functionType->isVariadic);
    }
    case IRTypeKind::IRPointerType: {
        return llvm::PointerType::get(ctx, 0);
    }
    case IRTypeKind::IRStructType: {
        auto structType = llvm::cast<IRStructType>(type);
        return getStructType(structType);
    }
    case IRTypeKind::IRUnionType: {
        auto it = structs.find(type);
        if (it != structs.end()) return it->second;

        auto unionType = llvm::cast<IRUnionType>(type);
        // Anonymous unions still need distinct opaque types: the uniqued empty struct from
        // StructType::get is shared while it has no body, so a nested union lowered during
        // field lowering would alias it and the outer setBody would not take effect.
        auto structType = unionType->name.empty() ? llvm::StructType::create(ctx) : llvm::StructType::create(ctx, unionType->name);
        structs.try_emplace(unionType, structType);

        llvm::Type* largestFieldType = nullptr;
        uint64_t largestFieldSize = 0;
        for (auto& field : unionType->getFields()) {
            auto fieldType = getLLVMType(field.type);
            auto size = getHostDataLayout().getTypeAllocSize(fieldType);
            if (size > largestFieldSize) {
                largestFieldType = fieldType;
                largestFieldSize = size;
            }
        }

        if (largestFieldType) {
            structType->setBody(largestFieldType, false);
        } else {
            structType->setBody({}, false);
        }
        return structType;
    }
    }

    llvm_unreachable("all cases handled");
}

bool LLVMGenerator::shouldUseSret(llvm::Type* returnType) {
    return !returnType->isVoidTy() && getHostDataLayout().getTypeAllocSize(returnType) > 16;
}

bool LLVMGenerator::shouldPassIndirectly(llvm::Type* type) {
    if (type->isVoidTy()) return false;
    // Only aggregates can be large; scalars are always passed directly.
    if (!type->isStructTy() && !type->isArrayTy()) return false;
    return getHostDataLayout().getTypeAllocSize(type) > 16;
}

void LLVMGenerator::emitMemcpy(llvm::Value* dest, llvm::Value* src, llvm::Type* type) {
    auto& layout = getHostDataLayout();
    auto size = layout.getTypeAllocSize(type);
    auto align = layout.getABITypeAlign(type).value();
    builder.CreateMemCpy(dest, llvm::MaybeAlign(align), src, llvm::MaybeAlign(align), llvm::ConstantInt::get(llvm::Type::getInt64Ty(ctx), size));
}

llvm::Value* LLVMGenerator::materializeConstant(llvm::Constant* constant, llvm::Type* type) {
    auto* alloca = builder.CreateAlloca(type, nullptr, "const.alloca");
    // Undef needs no store: uninitialized memory already represents it.
    if (!llvm::isa<llvm::UndefValue>(constant)) {
        builder.CreateStore(constant, alloca);
    }
    return alloca;
}

llvm::Function* LLVMGenerator::getFunction(const Function* function) {
    if (auto* llvmFunction = module->getFunction(function->mangledName)) return llvmFunction;

    bool isSret;
    auto llvmFunctionType = llvm::cast<llvm::FunctionType>(getLLVMType(function->getType()->getPointee(), &isSret));
    auto* llvmFunction = llvm::Function::Create(llvmFunctionType, llvm::Function::ExternalLinkage, function->mangledName, module);

    // Keep frame pointers so stack traces can always unwind past cx frames.
    // (Matches what Apple Clang emits; without this, backtrace() stops at
    // the first cx frame on platforms using compact unwind tables.)
    llvmFunction->addFnAttr("frame-pointer", "all");

    auto arg = llvmFunction->arg_begin(), argsEnd = llvmFunction->arg_end();
    if (isSret) {
        arg->setName("sret.arg");
        ++arg;
    }
    for (auto param = function->params.begin(); arg != argsEnd; ++param, ++arg) {
        arg->setName(param->name);
        auto paramLLVMType = getLLVMType(param->type);
        if (shouldPassIndirectly(paramLLVMType)) {
            arg->addAttr(llvm::Attribute::get(ctx, llvm::Attribute::ByVal, paramLLVMType));
            auto align = getHostDataLayout().getABITypeAlign(paramLLVMType).value();
            arg->addAttr(llvm::Attribute::getWithAlignment(ctx, llvm::Align(align)));
        }
    }

    if (isSret) {
        auto structType = getLLVMType(function->returnType);
        llvmFunction->getArg(0)->addAttr(llvm::Attribute::get(ctx, llvm::Attribute::StructRet, structType));
        auto align = getHostDataLayout().getABITypeAlign(structType).value();
        llvmFunction->getArg(0)->addAttr(llvm::Attribute::getWithAlignment(ctx, llvm::Align(align)));
    }
    return llvmFunction;
}

void LLVMGenerator::codegenFunctionBody(const Function* function, llvm::Function* llvmFunction) {
    isCurrentFunctionSret = shouldUseSret(getLLVMType(function->returnType));
    llvm::IRBuilder<>::InsertPointGuard insertPointGuard(builder);

    auto arg = llvmFunction->arg_begin();
    if (isCurrentFunctionSret) ++arg;
    for (auto& param : function->params) {
        generatedValues.emplace(&param, &*arg++);
    }

    for (auto* block : function->body) {
        auto llvmBlock = getBasicBlock(block);
        llvmBlock->insertInto(llvmFunction);
        builder.SetInsertPoint(llvmBlock);

        if (block->parameter) {
            auto paramLLVMType = getLLVMType(block->parameter->type);
            // Larger aggregates are represented as pointers to avoid large SSA copies.
            bool indirect = shouldPassIndirectly(paramLLVMType);
            auto phiType = indirect ? llvm::PointerType::get(ctx, 0) : paramLLVMType;
            auto phi = builder.CreatePHI(phiType, 2, block->parameter->name);
            for (auto pred : block->predecessors) {
                auto value = getValue(pred->body.back()->getBranchArgument());
                // A split predecessor's terminator landed in its continuation block.
                auto continueIt = blockContinueBlocks.find(pred);
                auto target = continueIt != blockContinueBlocks.end() ? continueIt->second : getBasicBlock(pred);
                if (indirect && llvm::isa<llvm::Constant>(value)) {
                    // Constants have no dependencies, so materialize them at the top of the
                    // entry block, which dominates the PHI. (The current insert point past
                    // the PHI does not.)
                    llvm::IRBuilder<>::InsertPointGuard guard(builder);
                    auto& entryBlock = llvmFunction->getEntryBlock();
                    builder.SetInsertPoint(&entryBlock, entryBlock.begin());
                    value = materializeConstant(llvm::cast<llvm::Constant>(value), paramLLVMType);
                }
                phi->addIncoming(value, target);
            }
            generatedValues.emplace(block->parameter, phi);
        }

        for (auto* inst : block->body) {
            auto llvmValue = codegenInst(inst);
            generatedValues.emplace(inst, llvmValue);
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
        // Functions without a body are references defined in another module;
        // they stay declarations, so they get no subprogram (a declaration
        // with debug info fails verification). Functions without a location
        // get none either: calls inside couldn't carry locations, which the
        // verifier forbids in functions with debug info.
        const Location& location = function->location;
        bool skipDebugInfo = !emitDebugInfo || function->body.empty() || !location.file || !*location.file || !location.isValid();
        llvm::SaveAndRestore saveSubprogram(currentDebugSubprogram, skipDebugInfo ? nullptr : createDebugSubprogram(function, llvmFunction));
        llvm::SaveAndRestore saveLocation(currentDebugFunctionLocation, function->location);
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
    if (isCurrentFunctionSret) {
        auto currentFunction = builder.GetInsertBlock()->getParent();
        auto sretPtr = currentFunction->getArg(0);
        auto returnLLVMType = getLLVMType(inst->value->getType());
        auto value = getValue(inst->value);
        if (shouldPassIndirectly(returnLLVMType) && !llvm::isa<llvm::Constant>(value)) {
            emitMemcpy(sretPtr, value, returnLLVMType);
        } else {
            builder.CreateStore(value, sretPtr);
        }
        return builder.CreateRetVoid();
    }
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
    auto llvmType = getLLVMType(inst->getType());
    if (shouldPassIndirectly(llvmType)) {
        // Larger aggregates stay in memory to avoid materializing large SSA
        // copies that expand during codegen. Copy to a fresh alloca: returning
        // the source pointer would let later stores observably mutate the value.
        auto dest = builder.CreateAlloca(llvmType, nullptr, inst->name);
        emitMemcpy(dest, getValue(inst->value), llvmType);
        return dest;
    }
    return builder.CreateLoad(llvmType, getValue(inst->value), inst->name);
}

llvm::Value* LLVMGenerator::codegenStore(const StoreInst* inst) {
    if (inst->value->kind == ValueKind::Undefined) {
        return nullptr;
    }
    auto valueLLVMType = getLLVMType(inst->value->getType());
    auto value = getValue(inst->value);
    auto pointer = getValue(inst->pointer);
    if (shouldPassIndirectly(valueLLVMType)) {
        // Constant aggregates store directly: unlike SSA values, they don't
        // expand into scalar operations during codegen.
        if (llvm::isa<llvm::Constant>(value)) {
            return builder.CreateStore(value, pointer);
        }
        emitMemcpy(pointer, value, valueLLVMType);
        return nullptr;
    }
    return builder.CreateStore(value, pointer);
}

llvm::Value* LLVMGenerator::codegenInsert(const InsertInst* inst) {
    auto aggregateLLVMType = getLLVMType(inst->aggregate->getType());
    if (shouldPassIndirectly(aggregateLLVMType)) {
        auto aggregate = getValue(inst->aggregate);
        auto value = getValue(inst->value);
        if (llvm::isa<llvm::Constant>(aggregate) && llvm::isa<llvm::Constant>(value)) {
            // Fold to a constant instead of emitting code. Global initializers
            // have no insert block, so this path must not emit instructions.
            return builder.CreateInsertValue(aggregate, value, inst->index);
        }
        ASSERT(builder.GetInsertBlock());
        auto tempAlloca = builder.CreateAlloca(aggregateLLVMType, nullptr, "insert.alloca");
        if (inst->aggregate->kind != ValueKind::Undefined) {
            if (auto* constant = llvm::dyn_cast<llvm::Constant>(aggregate)) {
                builder.CreateStore(constant, tempAlloca);
            } else {
                emitMemcpy(tempAlloca, aggregate, aggregateLLVMType);
            }
        }
        auto fieldPtr = builder.CreateConstInBoundsGEP2_32(aggregateLLVMType, tempAlloca, 0, inst->index, "insert.gep");
        auto fieldLLVMType = getLLVMType(inst->value->getType());
        if (inst->value->kind == ValueKind::Undefined) {
            // Leave the field uninitialized.
        } else if (shouldPassIndirectly(fieldLLVMType)) {
            if (auto* constant = llvm::dyn_cast<llvm::Constant>(value)) {
                builder.CreateStore(constant, fieldPtr);
            } else {
                emitMemcpy(fieldPtr, value, fieldLLVMType);
            }
        } else {
            builder.CreateStore(value, fieldPtr);
        }
        return tempAlloca;
    }
    auto aggregate = getValue(inst->aggregate);
    auto value = getValue(inst->value);
    return builder.CreateInsertValue(aggregate, value, inst->index);
}

llvm::Value* LLVMGenerator::codegenExtract(const ExtractInst* inst) {
    auto aggregateLLVMType = getLLVMType(inst->aggregate->getType());
    if (shouldPassIndirectly(aggregateLLVMType)) {
        auto aggregatePtr = getValue(inst->aggregate);
        if (llvm::isa<llvm::Constant>(aggregatePtr)) {
            return builder.CreateExtractValue(aggregatePtr, inst->index, inst->name);
        }
        auto fieldPtr = builder.CreateConstInBoundsGEP2_32(aggregateLLVMType, aggregatePtr, 0, inst->index, inst->name);
        auto fieldLLVMType = getLLVMType(inst->getType());
        if (shouldPassIndirectly(fieldLLVMType)) {
            auto tempAlloca = builder.CreateAlloca(fieldLLVMType, nullptr, "extract.alloca");
            emitMemcpy(tempAlloca, fieldPtr, fieldLLVMType);
            return tempAlloca;
        }
        return builder.CreateLoad(fieldLLVMType, fieldPtr, inst->name);
    }
    auto aggregate = getValue(inst->aggregate);
    return builder.CreateExtractValue(aggregate, inst->index, inst->name);
}

llvm::Value* LLVMGenerator::codegenCall(const CallInst* inst) {
    // Calls carry their source location so stack traces attribute frames.
    // The guard restores any enclosing location afterwards for correct nesting.
    struct DebugLocationGuard {
        DebugLocationGuard(llvm::IRBuilder<>& builder, llvm::DILocation* location) : builder(builder), previous(builder.getCurrentDebugLocation()) {
            builder.SetCurrentDebugLocation(location);
        }
        ~DebugLocationGuard() { builder.SetCurrentDebugLocation(previous); }
        llvm::IRBuilder<>& builder;
        llvm::DebugLoc previous;
    };
    llvm::DILocation* location = getDebugLocation(inst->expr ? inst->expr->location : Location());
    DebugLocationGuard debugLocationGuard(builder, location);

    auto function = getValue(inst->function);
    auto cxFunctionType = inst->function->getType();
    if (cxFunctionType->isPointerType()) cxFunctionType = cxFunctionType->getPointee();
    ASSERT(cxFunctionType->isFunctionType());

    bool isSret;
    auto* llvmFunctionType = llvm::cast<llvm::FunctionType>(getLLVMType(cxFunctionType, &isSret));
    auto paramTypes = cxFunctionType->getParamTypes();
    std::vector<llvm::Value*> args;
    args.reserve(inst->args.size() + 1);
    for (size_t i = 0; i < inst->args.size(); ++i) {
        auto value = getValue(inst->args[i]);
        // Named arguments are reordered to parameter order, so fixed parameters
        // line up positionally and variadic extras come last.
        bool isExtra = i >= paramTypes.size();
        auto argLLVMType = isExtra ? getLLVMType(inst->args[i]->getType()) : getLLVMType(paramTypes[i]);
        if (shouldPassIndirectly(argLLVMType)) {
            if (isExtra) {
                // C varargs passes aggregates by value rather than by pointer.
                if (!llvm::isa<llvm::Constant>(value)) {
                    value = builder.CreateLoad(argLLVMType, value);
                }
            } else if (auto* constant = llvm::dyn_cast<llvm::Constant>(value)) {
                value = materializeConstant(constant, argLLVMType);
            }
        }
        args.push_back(value);
    }
    if (isSret) {
        auto sretType = getLLVMType(cxFunctionType->getReturnType());
        auto sretAlloca = builder.CreateAlloca(sretType, nullptr, "sret.alloca");
        args.insert(args.begin(), sretAlloca);
        auto* call = builder.CreateCall(llvmFunctionType, function, args);
        // Direct calls inherit this from the callee, but indirect calls through function pointers can't.
        call->addParamAttr(0, llvm::Attribute::get(ctx, llvm::Attribute::StructRet, sretType));
        auto align = getHostDataLayout().getABITypeAlign(sretType).value();
        call->addParamAttr(0, llvm::Attribute::getWithAlignment(ctx, llvm::Align(align)));
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            auto paramLLVMType = getLLVMType(paramTypes[i]);
            if (shouldPassIndirectly(paramLLVMType)) {
                // +1 for the hidden sret parameter.
                unsigned index = static_cast<unsigned>(i + 1);
                call->addParamAttr(index, llvm::Attribute::get(ctx, llvm::Attribute::ByVal, paramLLVMType));
                auto paramAlign = getHostDataLayout().getABITypeAlign(paramLLVMType).value();
                call->addParamAttr(index, llvm::Attribute::getWithAlignment(ctx, llvm::Align(paramAlign)));
            }
        }
        if (shouldPassIndirectly(sretType)) {
            return sretAlloca;
        }
        return builder.CreateLoad(sretType, sretAlloca, "sret.load");
    } else {
        auto* call = builder.CreateCall(llvmFunctionType, function, args);
        for (size_t i = 0; i < paramTypes.size(); ++i) {
            auto paramLLVMType = getLLVMType(paramTypes[i]);
            if (shouldPassIndirectly(paramLLVMType)) {
                unsigned index = static_cast<unsigned>(i);
                call->addParamAttr(index, llvm::Attribute::get(ctx, llvm::Attribute::ByVal, paramLLVMType));
                auto paramAlign = getHostDataLayout().getABITypeAlign(paramLLVMType).value();
                call->addParamAttr(index, llvm::Attribute::getWithAlignment(ctx, llvm::Align(paramAlign)));
            }
        }
        return call;
    }
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
        return builder.CreateICmpEQ(left, right, inst->name);
    case Token::NotEqual:
        if (isFloat) return builder.CreateFCmpUNE(left, right);
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

llvm::Value* LLVMGenerator::codegenArrayOpElement(Token::Kind op, llvm::Value* left, llvm::Value* right, IRType* elemType) {
    // Mirrors codegenBinary, but polymorphic over scalar and vector LLVM
    // values so one mapping serves vector chunks and the scalar tail.
    bool isFloat = elemType->isFloatingPoint();
    bool isSigned = elemType->isSignedInteger();
    switch (op) {
    case Token::Plus:
        return isFloat ? builder.CreateFAdd(left, right) : builder.CreateAdd(left, right);
    case Token::Minus:
        return isFloat ? builder.CreateFSub(left, right) : builder.CreateSub(left, right);
    case Token::Star:
        return isFloat ? builder.CreateFMul(left, right) : builder.CreateMul(left, right);
    case Token::Slash:
        if (isFloat) return builder.CreateFDiv(left, right);
        return isSigned ? builder.CreateSDiv(left, right) : builder.CreateUDiv(left, right);
    case Token::Modulo:
        if (isFloat) return builder.CreateFRem(left, right);
        return isSigned ? builder.CreateSRem(left, right) : builder.CreateURem(left, right);
    case Token::PositiveModulo: {
        if (elemType->isUnsignedInteger()) return codegenArrayOpElement(Token::Modulo, left, right, elemType);
        // Positive remainder ((a % b) + b) % b, like the scalar rewrite in emitBinaryExpr.
        auto* rem = codegenArrayOpElement(Token::Modulo, left, right, elemType);
        auto* shifted = codegenArrayOpElement(Token::Plus, rem, right, elemType);
        return codegenArrayOpElement(Token::Modulo, shifted, right, elemType);
    }
    case Token::Equal:
        return isFloat ? builder.CreateFCmpOEQ(left, right) : builder.CreateICmpEQ(left, right);
    case Token::NotEqual:
        return isFloat ? builder.CreateFCmpUNE(left, right) : builder.CreateICmpNE(left, right);
    case Token::And:
        return builder.CreateAnd(left, right);
    case Token::Or:
        return builder.CreateOr(left, right);
    case Token::Xor:
        return builder.CreateXor(left, right);
    case Token::LeftShift:
        return builder.CreateShl(left, right);
    case Token::RightShift:
        return isSigned ? builder.CreateAShr(left, right) : builder.CreateLShr(left, right);
    default:
        llvm_unreachable("invalid array operation");
    }
}

llvm::Value* LLVMGenerator::codegenArrayOp(const ArrayOpInst* inst) {
    ASSERT(builder.GetInsertBlock() && "array ops cannot appear in global initializers");
    auto* arrayType = llvm::cast<IRArrayType>(inst->arrayType);
    int size = arrayType->size;
    auto* elemType = arrayType->elementType;
    auto* scalarTy = getLLVMType(elemType);
    auto* arrayLLVMType = getLLVMType(inst->arrayType);
    bool isComparison = inst->op == Token::Equal || inst->op == Token::NotEqual;
    bool foldAnd = inst->op == Token::Equal;
    // IRGen only emits this node for SIMD-friendly elements (see isVectorFriendlyElement).
    unsigned bitWidth = scalarTy->getScalarSizeInBits();
    ASSERT((scalarTy->isIntegerTy() && (bitWidth == 8 || bitWidth == 16 || bitWidth == 32 || bitWidth == 64)) || scalarTy->isFloatTy()
           || scalarTy->isDoubleTy());
    unsigned elemBytes = bitWidth / 8;
    unsigned lanes = 16 / elemBytes; // 128-bit chunks.

    auto isArraySide = [](const Value* side) { return side->getType()->isPointerType() && side->getType()->getPointee()->isArrayType(); };
    bool leftIsArray = isArraySide(inst->left);
    bool rightIsArray = isArraySide(inst->right);
    ASSERT(leftIsArray || rightIsArray);
    llvm::Value* lhsPtr = leftIsArray ? getValue(inst->left) : nullptr;
    llvm::Value* rhsPtr = rightIsArray ? getValue(inst->right) : nullptr;
    llvm::Value* lhsScalar = leftIsArray ? nullptr : getValue(inst->left);
    llvm::Value* rhsScalar = rightIsArray ? nullptr : getValue(inst->right);

    auto* i32 = llvm::Type::getInt32Ty(ctx);
    auto* i1 = llvm::Type::getInt1Ty(ctx);
    auto* zero = llvm::ConstantInt::get(i32, 0);
    auto gepAt = [&](llvm::Value* arrayPtr, llvm::Value* index) { return builder.CreateInBoundsGEP(arrayLLVMType, arrayPtr, {zero, index}); };
    auto loadChunk = [&](llvm::Value* arrayPtr, llvm::Value* index, unsigned count) {
        auto* vecTy = llvm::FixedVectorType::get(scalarTy, count);
        auto* load = builder.CreateLoad(vecTy, gepAt(arrayPtr, index));
        // Vector loads default to the vector's 16-byte alignment, which
        // arrays don't guarantee; use element alignment like scalar loads.
        load->setAlignment(llvm::Align(elemBytes));
        return load;
    };
    auto storeChunk = [&](llvm::Value* vec, llvm::Value* arrayPtr, llvm::Value* index) {
        auto* store = builder.CreateStore(vec, gepAt(arrayPtr, index));
        store->setAlignment(llvm::Align(elemBytes));
    };
    auto splat = [&](llvm::Value* scalar, unsigned count) { return builder.CreateVectorSplat(count, scalar); };
    auto foldMask = [&](llvm::Value* mask, unsigned count, llvm::Value* acc) {
        for (unsigned i = 0; i < count; ++i) {
            auto* lane = builder.CreateExtractElement(mask, i);
            acc = !acc ? lane : foldAnd ? builder.CreateAnd(acc, lane) : builder.CreateOr(acc, lane);
        }
        return acc;
    };

    if (size == 0) {
        if (isComparison) return llvm::ConstantInt::get(i1, foldAnd);
        return builder.CreateAlloca(arrayLLVMType, nullptr, inst->name);
    }

    unsigned count = static_cast<unsigned>(size);
    if (count <= lanes) {
        llvm::Value* l = leftIsArray ? loadChunk(lhsPtr, zero, count) : splat(lhsScalar, count);
        llvm::Value* r = rightIsArray ? loadChunk(rhsPtr, zero, count) : splat(rhsScalar, count);
        llvm::Value* vec = codegenArrayOpElement(inst->op, l, r, elemType);
        if (isComparison) return foldMask(vec, count, nullptr);
        auto* resultAlloca = builder.CreateAlloca(arrayLLVMType, nullptr, inst->name);
        storeChunk(vec, resultAlloca, zero);
        return resultAlloca;
    }

    llvm::Value* resultAlloca = nullptr;
    llvm::Value* accAlloca = nullptr;
    if (isComparison) {
        accAlloca = builder.CreateAlloca(i1, nullptr, "arrayop.acc");
        builder.CreateStore(llvm::ConstantInt::get(i1, foldAnd), accAlloca);
    } else {
        resultAlloca = builder.CreateAlloca(arrayLLVMType, nullptr, inst->name);
    }

    int chunks = size / static_cast<int>(lanes);
    auto* indexAlloca = builder.CreateAlloca(i32, nullptr, "arrayop.i");
    builder.CreateStore(zero, indexAlloca);
    auto* function = builder.GetInsertBlock()->getParent();
    auto* cond = llvm::BasicBlock::Create(ctx, "arrayop.cond", function);
    auto* body = llvm::BasicBlock::Create(ctx, "arrayop.body", function);
    auto* end = llvm::BasicBlock::Create(ctx, "arrayop.end", function);
    builder.CreateBr(cond);

    builder.SetInsertPoint(cond);
    builder.CreateCondBr(builder.CreateICmpSLT(builder.CreateLoad(i32, indexAlloca), llvm::ConstantInt::get(i32, chunks)), body, end);

    builder.SetInsertPoint(body);
    auto* i = builder.CreateLoad(i32, indexAlloca);
    auto* first = builder.CreateMul(i, llvm::ConstantInt::get(i32, lanes));
    llvm::Value* chunkL = leftIsArray ? loadChunk(lhsPtr, first, lanes) : splat(lhsScalar, lanes);
    llvm::Value* chunkR = rightIsArray ? loadChunk(rhsPtr, first, lanes) : splat(rhsScalar, lanes);
    llvm::Value* chunkVec = codegenArrayOpElement(inst->op, chunkL, chunkR, elemType);
    if (isComparison) {
        builder.CreateStore(foldMask(chunkVec, lanes, builder.CreateLoad(i1, accAlloca)), accAlloca);
    } else {
        storeChunk(chunkVec, resultAlloca, first);
    }
    builder.CreateStore(builder.CreateAdd(i, llvm::ConstantInt::get(i32, 1)), indexAlloca);
    builder.CreateBr(cond);

    // Emission continues in end: later instructions execute after the loop.
    // Record the split so successor PHIs reference end as the predecessor.
    builder.SetInsertPoint(end);
    blockContinueBlocks[inst->parent] = end;
    for (int t = chunks * static_cast<int>(lanes); t < size; ++t) {
        auto* index = llvm::ConstantInt::get(i32, t);
        llvm::Value* tailL = leftIsArray ? builder.CreateLoad(scalarTy, gepAt(lhsPtr, index)) : lhsScalar;
        llvm::Value* tailR = rightIsArray ? builder.CreateLoad(scalarTy, gepAt(rhsPtr, index)) : rhsScalar;
        llvm::Value* elem = codegenArrayOpElement(inst->op, tailL, tailR, elemType);
        if (isComparison) {
            auto* acc = builder.CreateLoad(i1, accAlloca);
            builder.CreateStore(foldAnd ? builder.CreateAnd(acc, elem) : builder.CreateOr(acc, elem), accAlloca);
        } else {
            builder.CreateStore(elem, gepAt(resultAlloca, index));
        }
    }
    if (isComparison) return builder.CreateLoad(i1, accAlloca);
    return resultAlloca;
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
    case Token::Star:
        return operand;
    default:
        llvm_unreachable("invalid unary operation");
    }
}

llvm::Value* LLVMGenerator::codegenGEP(const GEPInst* inst) {
    auto pointer = getValue(inst->pointer);
    auto pointeeType = getLLVMType(inst->pointer->getType()->getPointee());
    auto indexes = map(inst->indexes, [&](auto* index) { return getValue(index); });
    return builder.CreateInBoundsGEP(pointeeType, pointer, indexes, inst->name);
}

llvm::Value* LLVMGenerator::codegenConstGEP(const ConstGEPInst* inst) {
    auto pointer = getValue(inst->pointer);
    auto pointeeType = getLLVMType(inst->pointer->getType()->getPointee());
    return builder.CreateConstInBoundsGEP2_32(pointeeType, pointer, 0, inst->index, inst->name);
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
        // char zero-extends like an unsigned integer.
        if (sourceType->isUnsignedInteger() || sourceType->isChar()) return builder.CreateUIToFP(value, getLLVMType(type));
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
    return new llvm::GlobalVariable(*module, getLLVMType(inst->type), false, linkage, initializer, inst->name);
}

llvm::Value* LLVMGenerator::codegenConstantString(const ConstantString* inst) {
    // Pass the module explicitly: globals are emitted without an insert block.
    return builder.CreateGlobalString(inst->value, "", 0, module);
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
    // Functions are shared across IR modules (see IRGenerator::getFunction), so they
    // must resolve against the current LLVM module every time: the cache below would
    // otherwise return another module's function. getFunction already deduplicates
    // within the module, making the bypass equivalent for single-module programs.
    if (value->kind == ValueKind::Function) return getFunction(llvm::cast<Function>(value));
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
    case ValueKind::ArrayOpInst:
        return codegenArrayOp(llvm::cast<ArrayOpInst>(value));
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
    // Set the host target before lowering any types: IRBuilder derives alloca
    // and load/store alignments from the module layout, which must agree with
    // the layout used for indirect-passing decisions and memcpy alignments.
    module->setTargetTriple(llvm::Triple(llvm::sys::getDefaultTargetTriple()));
    module->setDataLayout(getHostDataLayout());

    debugFiles.clear();
    // The compile unit file only anchors module-level metadata; functions
    // carry their own files. Without any located function there is nothing
    // to attribute, so skip debug info entirely (an empty filename fails
    // verification).
    const char* unitPath = nullptr;
    for (auto* function : sourceModule.functions) {
        if (function->location.file && *function->location.file) {
            unitPath = function->location.file;
            break;
        }
    }
    if (unitPath && emitDebugInfo) {
        debugBuilder = std::make_unique<llvm::DIBuilder>(*module);
        if (useCodeViewDebugInfo) {
            module->addModuleFlag(llvm::Module::Warning, "CodeView", 1);
        } else {
            module->addModuleFlag(llvm::Module::Warning, "Dwarf Version", 4);
        }
        module->addModuleFlag(llvm::Module::Warning, "Debug Info Version", llvm::DEBUG_METADATA_VERSION);
        llvm::DIFile* unitFile = getDebugFile(unitPath);
        debugBuilder->createCompileUnit(llvm::dwarf::DW_LANG_C, unitFile, "cx", /* isOptimized */ false, /* Flags */ "", /* RV */ 0);
    }

    for (auto* globalVariable : sourceModule.globalVariables) {
        getValue(globalVariable);
    }

    for (auto* function : sourceModule.functions) {
        codegenFunction(function);
    }

    if (debugBuilder) {
        debugBuilder->finalize();
        debugBuilder.reset();
    }
    debugFiles.clear();
    currentDebugSubprogram = nullptr;
    currentDebugFunctionLocation = Location();

    ASSERT(!llvm::verifyModule(*module, &llvm::errs()));
    generatedModules.push_back(module);
    module = nullptr;
    return *generatedModules.back();
}

/// Returns the debug file for the path. The directory must be non-empty:
/// Apple's linker only emits debug-map entries for compile units with
/// DW_AT_comp_dir, and without those dsymutil collects nothing.
llvm::DIFile* LLVMGenerator::getDebugFile(llvm::StringRef path) {
    std::string key = path.str();
    auto it = debugFiles.find(key);
    if (it != debugFiles.end()) return it->second;
    llvm::SmallString<128> directory = llvm::sys::path::parent_path(path);
    if (directory.empty() && llvm::sys::fs::current_path(directory)) directory = ".";
    auto* file = debugBuilder->createFile(llvm::sys::path::filename(path), directory);
    debugFiles.emplace(std::move(key), file);
    return file;
}

llvm::DISubprogram* LLVMGenerator::createDebugSubprogram(const Function* function, llvm::Function* llvmFunction) {
    llvm::DIFile* file = getDebugFile(function->location.file ? function->location.file : "");
    unsigned line = function->location.isValid() ? static_cast<unsigned>(function->location.line) : 0;
    auto* subroutineType = debugBuilder->createSubroutineType(debugBuilder->getOrCreateTypeArray({}));
    auto* subprogram = debugBuilder->createFunction(file, function->name, function->mangledName, file, line, subroutineType, line, llvm::DINode::FlagZero,
                                                    llvm::DISubprogram::SPFlagDefinition);
    llvmFunction->setSubprogram(subprogram);
    return subprogram;
}

llvm::DILocation* LLVMGenerator::getDebugLocation(Location location) {
    // Synthesized calls without a location inherit their function's, so every
    // call in a function with debug info carries one (required by the verifier).
    if (!location.isValid()) location = currentDebugFunctionLocation;
    if (!currentDebugSubprogram || !location.isValid()) return nullptr;
    return llvm::DILocation::get(ctx, static_cast<unsigned>(location.line), static_cast<unsigned>(location.column), currentDebugSubprogram);
}
