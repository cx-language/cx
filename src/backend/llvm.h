#pragma once

#include <unordered_map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/IR/IRBuilder.h>
#pragma warning(pop)
#include "ir.h"

namespace delta {

struct IRModule;
struct IRType;
struct IRStructType;
struct Value;
struct Function;
struct BasicBlock;

struct LLVMGenerator {
    LLVMGenerator() : builder(ctx) {}
    llvm::Module& codegenModule(const IRModule& sourceModule);
    llvm::Value* codegenAlloca(const AllocaInst* inst);
    llvm::Value* codegenReturn(const ReturnInst* inst);
    llvm::Value* codegenBranch(const BranchInst* inst);
    llvm::Value* codegenCondBranch(const CondBranchInst* inst);
    llvm::Value* codegenSwitch(const SwitchInst* inst);
    llvm::Value* codegenLoad(const LoadInst* inst);
    llvm::Value* codegenStore(const StoreInst* inst);
    llvm::Value* codegenInsert(const InsertInst* inst);
    llvm::Value* codegenExtract(const ExtractInst* inst);
    llvm::Value* codegenCall(const CallInst* inst);
    llvm::Value* codegenBinary(const BinaryInst* inst);
    llvm::Value* codegenUnary(const UnaryInst* inst);
    llvm::Value* codegenGEP(const GEPInst* inst);
    llvm::Value* codegenConstGEP(const ConstGEPInst* inst);
    llvm::Value* codegenCast(const CastInst* inst);
    llvm::Value* codegenUnreachable();
    llvm::Value* codegenSizeof(const SizeofInst* inst);
    llvm::Value* codegenBasicBlock(const BasicBlock* block);
    llvm::Value* codegenGlobalVariable(const GlobalVariable* inst);
    llvm::Value* codegenConstantString(const ConstantString* inst);
    llvm::Value* codegenConstantInt(const ConstantInt* inst);
    llvm::Value* codegenConstantFP(const ConstantFP* inst);
    llvm::Value* codegenConstantBool(const ConstantBool* inst);
    llvm::Value* codegenConstantNull(const ConstantNull* inst);
    llvm::Value* codegenUndefined(const Undefined* inst);
    llvm::Value* getValue(const Value* value);
    llvm::Value* codegenInst(const Value* value);
    llvm::BasicBlock* getBasicBlock(const BasicBlock* block);
    llvm::Function* getFunction(const Function* function);
    void codegenFunction(const Function* function);
    void codegenFunctionBody(const Function* function, llvm::Function* llvmFunction);
    llvm::Type* getLLVMType(IRType* type);
    llvm::Type* getBuiltinType(llvm::StringRef name);
    llvm::Type* getStructType(IRStructType* type);

    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    llvm::Module* module = nullptr;
    std::vector<llvm::Module*> generatedModules;
    std::unordered_map<const Value*, llvm::Value*> generatedValues;
    std::unordered_map<IRType*, llvm::StructType*> structs;
};

} // namespace delta
