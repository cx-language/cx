#pragma once

#pragma warning(push, 0)
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallSet.h>
#pragma warning(pop)

#include <cstdint>

namespace llvm {
template<typename T> class Optional;
}

namespace cx {

struct IRModule;
struct BasicBlock;
struct Value;
struct Instruction;
struct Location;

enum class Nullability { DefinitelyNullable, IndefiniteNullability, DefinitelyNotNull };

struct NullAnalyzer {
    llvm::SmallPtrSet<BasicBlock*, 16> visited;
    // Ranges already warned for nullability in the current function, so
    // paired checks (e.g. the receiver and unwrap checks on one method
    // call) warn only once.
    llvm::SmallSet<std::pair<const char*, uint64_t>, 4> warnedRanges;

    void analyze(IRModule* module);
    void analyze(Value* value);
    void warnNullabilityOnce(Location begin, Location end, const char* message);
    Nullability analyzeNullability(Value* nullableValue, Instruction* startFrom);
    Nullability analyzeNullability_recursive(Value* nullableValue, Instruction* startFrom, int gepIndex = -1);
    Nullability analyzeNullability_fromPredecessor(Value* nullableValue, BasicBlock* predecessor, BasicBlock* destination, int gepIndex);
};

} // namespace cx
