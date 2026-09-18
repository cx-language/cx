#pragma once

#include "ir.h"
#include <unordered_map>
#include <unordered_set>

namespace cx {

struct CGenerator {
    CGenerator() : preludeStream(prelude), stream(result) {}
    // When dispatchMode is true, functions are emitted as goto-free dispatch
    // loops (a while(1)/switch over a program counter) instead of using labels
    // and gotos. This supports C compilers that can't handle arbitrary gotos,
    // such as the one used by the web playground toolchain.
    explicit CGenerator(bool dispatchMode) : dispatchMode(dispatchMode), preludeStream(prelude), stream(result) {}
    void codegenModule(const IRModule& module);
    void codegenAlloca(const AllocaInst* inst);
    void codegenReturn(const ReturnInst* inst);
    void codegenBranch(const BranchInst* inst);
    void codegenCondBranch(const CondBranchInst* inst);
    void codegenSwitch(const SwitchInst* inst);
    void codegenLoad(const LoadInst* inst);
    void codegenStore(const StoreInst* inst);
    void codegenInsert(const InsertInst* inst);
    void codegenExtract(const ExtractInst* inst);
    void codegenCall(const CallInst* inst);
    void codegenBinary(const BinaryInst* inst);
    void codegenUnary(const UnaryInst* inst);
    void codegenGEP(const GEPInst* inst);
    void codegenConstGEP(const ConstGEPInst* inst);
    void codegenCast(const CastInst* inst);
    void codegenUnreachable(const UnreachableInst* inst);
    void codegenSizeof(const SizeofInst* inst);
    void codegenBasicBlock(const BasicBlock* block);
    void codegenGlobalVariable(const GlobalVariable* inst);
    void codegenConstantString(const ConstantString* inst);
    void codegenConstantInt(const ConstantInt* inst);
    void codegenConstantFP(const ConstantFP* inst);
    void codegenConstantBool(const ConstantBool* inst);
    void codegenConstantNull(const ConstantNull* inst);
    void codegenUndefined(const Undefined* inst);
    void codegenInst(const Value* value);
    void codegenInstImpl(const Value* value);
    // Emits a call, branch, or return argument. `undefined` has no C
    // equivalent, so a zero value of the argument type is passed instead
    // (matching LLVM's undef, which the recipient must not meaningfully use).
    void codegenArgument(const Value* value);
    void codegenTempDeclaration(const Value* value, const std::string& name);
    // Emits a `type name;` declaration, using parenthesized declarator syntax
    // for pointers to arrays (e.g. `int (*name)[4]`), which the split
    // codegenType/codegenTypeSuffix pair cannot express.
    void codegenTempDeclarationForType(IRType* type, const std::string& name);
    void codegenFunctionPrototype(const Function* function);
    void codegenFunction(const Function* function);
    void codegenFunctionDispatch(const Function* function);
    void codegenType(llvm::raw_string_ostream& stream, IRType* type, bool needsTypeDefinition);
    void codegenTypeSuffix(llvm::raw_string_ostream& stream, IRType* type, bool needsTypeDefinition);
    void codegenTypeDefinition(llvm::raw_string_ostream& stream, IRType* type);
    const std::string& getBlockLabel(const BasicBlock* block);
    // Returns the C name for a temporary value, assigning (prefix + counter)
    // and registering it on first use. In dispatch mode the hoisting pass
    // assigns all names up front (see codegenFunctionDispatch), and later
    // uses during body emission find the existing registration.
    const std::string& getOrCreateTempName(const Value* inst, llvm::StringRef prefix);
    const std::string& getTempName(const Value* inst, llvm::StringRef prefix);
    // Returns the (sanitized) C name for a basic block parameter, registering
    // and declaring it on first use. Block parameters are compiler-generated
    // (e.g. "and", "or", "if.result"), and their raw names are not all valid
    // C identifiers, so they must go through this function.
    const std::string& getBlockParamName(const Parameter* param);
    // Emits declarations for all basic block parameters of the function. In
    // the default mode this runs at the top of codegenFunction (after which
    // branch sites only assign); in dispatch mode their declarations are
    // hoisted together with the other temporaries.
    void collectBlockParams(const Function* function);
    // Copies array-typed parameters into local arrays on function entry.
    // Array parameters arrive decayed to pointers, so the body uses the
    // copies, which behave like any other array. This also implements
    // by-value semantics.
    void copyArrayParams(const Function* function);
    std::string finish();

    bool dispatchMode = false;
    std::string prelude;
    std::string result;
    llvm::raw_string_ostream preludeStream; // Contains struct definitions
    llvm::raw_string_ostream stream; // Contains functions
    std::unordered_set<IRType*> alreadyEmittedTypes;
    std::unordered_set<std::string> alreadyDefinedFunctions;
    std::unordered_map<const Value*, std::string> emittedValues;
    // Program counter value per basic block, used only in dispatch mode.
    std::unordered_map<const BasicBlock*, int> dispatchBlockIds;
    int valueSuffixCounter = 0;
};

} // namespace cx
