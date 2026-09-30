#pragma once

#pragma warning(push, 0)
#include <llvm/ADT/DenseMap.h>
#include <llvm/ADT/SmallPtrSet.h>
#include <llvm/ADT/SmallSet.h>
#include <llvm/ADT/SmallVector.h>
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

enum class Nullability { DefinitelyNullable, DefinitelyNull, IndefiniteNullability, DefinitelyNotNull };

struct NullAnalyzer {
    llvm::SmallPtrSet<BasicBlock*, 16> visited;
    // Ranges already warned for nullability in the current function, so
    // paired checks (e.g. the receiver and unwrap checks on one method
    // call) warn only once.
    llvm::SmallSet<std::pair<const char*, uint64_t>, 4> warnedRanges;
    // Switch edges are not recorded in BasicBlock::predecessors (the LLVM
    // backend reads those for PHI incoming values, and a switch terminator
    // carries no branch argument), so the analyzer maps them itself.
    llvm::DenseMap<BasicBlock*, llvm::SmallVector<BasicBlock*, 2>> switchPredecessors;

    void analyze(IRModule* module);
    void analyze(Value* value);
    void warnNullabilityOnce(Location begin, Location end, const char* message);
    void warnForNullability(Location begin, Location end, Nullability nullability, const char* nullMessage, const char* nullableMessage);
    Nullability analyzeNullability(Value* nullableValue, Instruction* startFrom);
    Nullability analyzeNullability_recursive(Value* nullableValue, Instruction* startFrom, int gepIndex = -1);
    Nullability analyzeNullability_fromPredecessor(Value* nullableValue, BasicBlock* predecessor, BasicBlock* destination, int gepIndex);
};

} // namespace cx
