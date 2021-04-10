#pragma once

#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#pragma warning(pop)

namespace llvm {
template<typename T>
class Optional;
}

namespace cx {

struct IRModule;
struct BasicBlock;
struct Value;
struct Instruction;

enum class Nullability { DefinitelyNullable, IndefiniteNullability, DefinitelyNotNull };

struct NullAnalyzer {
    llvm::SmallPtrSet<BasicBlock*, 16> visited;

    void analyze(IRModule* module);
    void analyze(Value* value);
    Nullability analyzeNullability(Value* nullableValue, Instruction* startFrom);
    Nullability analyzeNullability_recursive(Value* nullableValue, Instruction* startFrom, int gepIndex = -1);
    Nullability analyzeNullability_fromPredecessor(Value* nullableValue, BasicBlock* predecessor, BasicBlock* destination, int gepIndex);
};

} // namespace cx
