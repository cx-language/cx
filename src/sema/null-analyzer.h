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

struct NullAnalyzer {
    llvm::SmallPtrSet<BasicBlock*, 16> visited;

    void analyze(IRModule* module);
    void analyze(Value* value);
    /// Returns true if the given expression (of optional type) is guaranteed to be non-null, e.g.
    /// if it was previously checked against null, and the type-checker can prove that it wasn't set
    /// back to null after that check.
    bool isDefinitelyNotNull(Value* nullableValue, Instruction* startFrom);
    bool isDefinitelyNotNullRecursive(Value* nullableValue, Instruction* startFrom, int gepIndex = -1);
    bool isNotNullWhenComingFrom(Value* nullableValue, BasicBlock* predecessor, BasicBlock* destination, int gepIndex);
};

} // namespace cx
