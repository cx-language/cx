#include "null-analyzer.h"
#include "../ast/decl.h"
#include "../backend/ir.h"

using namespace cx;

Nullability NullAnalyzer::analyzeNullability(Value* nullableValue, Instruction* startFrom) {
    visited.clear();
    return analyzeNullability_recursive(nullableValue, startFrom);
}

Nullability NullAnalyzer::analyzeNullability_recursive(Value* nullableValue, Instruction* startFrom, int gepIndex) {
    auto block = NOTNULL(startFrom->parent);
    int startFromIndex = -1;

    for (int i = 0; i < block->body.size(); i++) {
        if (block->body[i] == startFrom) {
            startFromIndex = i;
            break;
        }
    }

    ASSERT(startFromIndex != -1);

    for (int i = startFromIndex - 1; i >= 0; i--) {
        Instruction* currentInst = block->body[i];

        // If gepIndex is specified, i.e. non-negative, we're searching for a GEP accessing nullableValue at that index.
        if (currentInst == nullableValue && gepIndex == -1) {
            if (auto load = llvm::dyn_cast<LoadInst>(nullableValue)) {
                if (auto gep = llvm::dyn_cast<ConstGEPInst>(load->value)) {
                    return analyzeNullability_recursive(gep->pointer, gep, gep->index);
                }
                return analyzeNullability_recursive(load->value, load);
            } else if (auto cast = llvm::dyn_cast<CastInst>(nullableValue)) {
                return analyzeNullability_recursive(cast->value, cast);
            } else if (auto gep = llvm::dyn_cast<ConstGEPInst>(nullableValue)) {
                return analyzeNullability_recursive(gep->pointer, gep, gep->index);
            }
        }

        if (auto store = llvm::dyn_cast<StoreInst>(currentInst)) {
            if (gepIndex == -1) {
                if (store->pointer == nullableValue) {
                    return analyzeNullability_recursive(store->value, store);
                }
            } else {
                if (auto gep = llvm::dyn_cast<ConstGEPInst>(store->pointer)) {
                    if (gep->pointer == nullableValue && gep->index == gepIndex) {
                        return Nullability::DefinitelyNullable;
                    }
                    if (auto valueLoad = llvm::dyn_cast<LoadInst>(nullableValue)) {
                        if (auto gepPointerLoad = llvm::dyn_cast<LoadInst>(gep->pointer)) {
                            if (valueLoad->value == gepPointerLoad->value) {
                                return Nullability::DefinitelyNullable;
                            }
                        }
                    }
                }
            }
        }
    }

    if (block->predecessors.empty()) {
        return Nullability::DefinitelyNullable;
    }

    visited.insert(block);
    int notNullPredecessors = 0;

    for (auto predecessor : block->predecessors) {
        if (visited.count(predecessor)) {
            continue;
        }
        auto nullability = analyzeNullability_fromPredecessor(nullableValue, predecessor, block, gepIndex);
        if (nullability == Nullability::DefinitelyNullable) {
            return nullability;
        } else if (nullability == Nullability::DefinitelyNotNull) {
            notNullPredecessors++;
        }
    }

    if (notNullPredecessors == block->predecessors.size()) {
        return Nullability::DefinitelyNotNull;
    }
    return Nullability::IndefiniteNullability;
}

Nullability NullAnalyzer::analyzeNullability_fromPredecessor(Value* nullableValue, BasicBlock* predecessor, BasicBlock* destination, int gepIndex) {
    auto lastInst = predecessor->body.back();

    if (auto condBr = llvm::dyn_cast<CondBranchInst>(lastInst)) {
        if (auto binary = llvm::dyn_cast<BinaryInst>(condBr->condition)) {
            if ((binary->left == nullableValue || binary->left->loads(nullableValue, gepIndex)) && binary->right->kind == ValueKind::ConstantNull) {
                if (binary->op == Token::Equal) {
                    if (destination == condBr->trueBlock) return Nullability::DefinitelyNullable;
                    if (destination == condBr->falseBlock) return Nullability::DefinitelyNotNull;
                } else if (binary->op == Token::NotEqual) {
                    if (destination == condBr->trueBlock) return Nullability::DefinitelyNotNull;
                    if (destination == condBr->falseBlock) return Nullability::DefinitelyNullable;
                } else {
                    llvm_unreachable("invalid null comparison operator");
                }
                return Nullability::IndefiniteNullability;
            }
        }
    }

    return analyzeNullability_recursive(nullableValue, lastInst, gepIndex);
}

void NullAnalyzer::analyze(IRModule* module) {
    for (auto function : module->functions) {
        for (auto block : function->body) {
            for (auto inst : block->body) {
                analyze(inst);
            }
        }
    }
}

void NullAnalyzer::analyze(Value* value) {
    switch (value->kind) {
        case ValueKind::CallInst: {
            auto call = llvm::cast<CallInst>(value);
            if (call->expr) {
                if (auto receiverType = call->expr->getReceiverType()) {
                    // isConstructorDecl check filters out Optional() calls.
                    if (receiverType.isOptionalType() && !call->expr->getCalleeDecl()->isConstructorDecl() &&
                        analyzeNullability(call->args[0], call) == Nullability::DefinitelyNullable) {
                        // TODO: Store the implicit 'this' receiver to the call expr during typechecking to simplify this code.
                        auto location = call->expr->getReceiver() ? call->expr->getReceiver()->getLocation() : call->expr->getLocation();
                        WARN(location, "receiver may be null; unwrap it with a postfix '!' to silence this warning");
                    }
                }
            }
            break;
        }
        case ValueKind::BinaryInst: {
            auto binary = llvm::cast<BinaryInst>(value);
            if (llvm::isa<ConstantNull>(binary->right)) {
                ASSERT(binary->op == Token::Equal || binary->op == Token::NotEqual);
                if (binary->getExpr() && analyzeNullability(binary->left, binary) == Nullability::DefinitelyNotNull) {
                    WARN(binary->getExpr()->getLocation(), "value cannot be null here; null check can be removed");
                }
            }
            break;
        }
        case ValueKind::LoadInst: {
            auto load = llvm::cast<LoadInst>(value);
            if (auto expr = llvm::dyn_cast_or_null<UnaryExpr>(load->expr)) {
                if (expr->getOperand().getType().isOptionalType() && analyzeNullability(load->value, load) == Nullability::DefinitelyNullable) {
                    WARN(expr->getLocation(), "dereferenced pointer may be null; unwrap it with a postfix '!' to silence this warning");
                }
            }
            break;
        }
        case ValueKind::ConstGEPInst: {
            auto gep = llvm::cast<ConstGEPInst>(value);
            if (gep->expr) {
                if (gep->expr->getBaseExpr()->getType().isOptionalType() && !gep->expr->getBaseExpr()->isThis() &&
                    analyzeNullability(gep->pointer, gep) == Nullability::DefinitelyNullable) {
                    WARN(gep->expr->getBaseExpr()->getLocation(), "value may be null; unwrap it with a postfix '!' to silence this warning");
                }
            }
            break;
        }
        default:
            break;
    }
}
