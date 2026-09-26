#pragma once

#include <cassert>
#include <unordered_map>
#include <vector>
#pragma warning(push, 0)
#include <llvm/IR/DIBuilder.h>
#include <llvm/IR/IRBuilder.h>
#pragma warning(pop)
#include "ir.h"

namespace cx {

struct IRModule;
struct IRType;
struct IRStructType;
struct Value;
struct Function;
struct BasicBlock;

struct LLVMGenerator {
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
    llvm::Value* codegenArrayOp(const ArrayOpInst* inst);
    llvm::Value* codegenArrayOpElement(Token::Kind op, llvm::Value* left, llvm::Value* right, IRType* elemType);
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
    llvm::DIFile* getDebugFile(llvm::StringRef path);
    llvm::DILocation* getDebugLocation(Location location);
    llvm::DISubprogram* createDebugSubprogram(const Function* function, llvm::Function* llvmFunction);
    llvm::Type* getLLVMType(IRType* type, bool* isSret = nullptr);
    bool shouldUseSret(llvm::Type* returnType);
    bool shouldPassIndirectly(llvm::Type* type);
    void emitMemcpy(llvm::Value* dest, llvm::Value* src, llvm::Type* type);
    llvm::Value* materializeConstant(llvm::Constant* constant, llvm::Type* type);
    llvm::Type* getBuiltinType(llvm::StringRef name);
    llvm::Type* getStructType(IRStructType* type);

    // False in release modes: skip debug info emission entirely. Frame pointers
    // are still kept so backtrace() unwinds past cx frames.
    bool emitDebugInfo = true;
    // Emit CodeView instead of DWARF (the default on Windows, where debuggers read PDB).
    bool useCodeViewDebugInfo = false;

    // Hands the context to the caller (for JIT execution). The generator must not be used afterwards.
    std::unique_ptr<llvm::LLVMContext> takeContext() {
        assert(!debugBuilder && "cannot take context mid-codegen");
        return std::move(ownedCtx);
    }

    // Heap-owned so `cx run` can hand the context to LLVM's JIT, which takes owning pointers.
    std::unique_ptr<llvm::LLVMContext> ownedCtx = std::make_unique<llvm::LLVMContext>();
    llvm::LLVMContext& ctx = *ownedCtx;
    llvm::IRBuilder<> builder{ctx};
    llvm::Module* module = nullptr;
    std::vector<llvm::Module*> generatedModules;
    std::unordered_map<const Value*, llvm::Value*> generatedValues;
    // A loop-lowered array op splits its cx block; the terminator lands in
    // the continuation block, which successors must reference as the
    // predecessor instead (see codegenArrayOp). Keyed by cx block, which is
    // unique across functions, so no per-function clearing is needed.
    std::unordered_map<const BasicBlock*, llvm::BasicBlock*> blockContinueBlocks;
    std::unordered_map<IRType*, llvm::StructType*> structs;
    bool isCurrentFunctionSret;
    std::unique_ptr<llvm::DIBuilder> debugBuilder;
    llvm::DISubprogram* currentDebugSubprogram = nullptr;
    Location currentDebugFunctionLocation;
    std::unordered_map<std::string, llvm::DIFile*> debugFiles;
};

} // namespace cx
