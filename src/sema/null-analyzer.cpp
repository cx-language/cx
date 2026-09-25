#include "null-analyzer.h"
#include "../ast/decl.h"
#include "../backend/ir.h"
#include "../backend/irgen.h"

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
            } else if (auto extract = llvm::dyn_cast<ExtractInst>(nullableValue)) {
                return analyzeNullability_recursive(extract->aggregate, extract);
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

        // Value-implemented optionals branch on an enum tag comparison, e.g. `if (opt)` lowers to `tag == Some`.
        auto* condition = condBr->condition;
        bool negated = false;

        if (auto unary = llvm::dyn_cast<UnaryInst>(condition)) {
            if (unary->op == Token::Not) {
                condition = unary->operand;
                negated = true;
            }
        }

        if (auto binary = llvm::dyn_cast<BinaryInst>(condition)) {
            auto* extract = llvm::dyn_cast<ExtractInst>(binary->left);
            auto* constant = llvm::dyn_cast<ConstantInt>(binary->right);
            if (extract && constant && extract->index == IRGenerator::optionalTagFieldIndex
                && (extract->aggregate == nullableValue || extract->aggregate->loads(nullableValue, gepIndex))) {
                int64_t tag = constant->value.getSExtValue();
                bool isSomeTag = tag == IRGenerator::getOptionalSomeTag();
                bool isNoneTag = tag == IRGenerator::getOptionalNoneTag();
                if (isSomeTag || isNoneTag) {
                    bool trueMeansNotNull = ((binary->op == Token::Equal) == isSomeTag) != negated;
                    if (destination == condBr->trueBlock) return trueMeansNotNull ? Nullability::DefinitelyNotNull : Nullability::DefinitelyNullable;
                    if (destination == condBr->falseBlock) return trueMeansNotNull ? Nullability::DefinitelyNullable : Nullability::DefinitelyNotNull;
                    return Nullability::IndefiniteNullability;
                }
            }
        }
    }

    return analyzeNullability_recursive(nullableValue, lastInst, gepIndex);
}

// Runs after type-checking (which implicitly unwraps optionals as needed),
// emitting warnings for null-safety violations.
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
        if (auto* callExpr = llvm::dyn_cast_or_null<CallExpr>(call->expr)) {
            if (auto receiverType = callExpr->receiverType) {
                if (receiverType.isOptionalType() && analyzeNullability(call->args[0], call) == Nullability::DefinitelyNullable) {
                    // TODO: Store the implicit 'this' receiver to the call expr during typechecking to simplify this code.
                    auto location = callExpr->getReceiver() ? callExpr->getReceiver()->location : callExpr->location;
                    WARN(location, "receiver may be null; unwrap it with a postfix '!' to silence this warning");
                }
            }
        }
        break;
    }
    case ValueKind::BinaryInst: {
        auto binary = llvm::cast<BinaryInst>(value);
        if (llvm::StringRef(binary->name).starts_with("__implicit_unwrap")) {
            if (binary->getExpr() && analyzeNullability(binary->left, binary) == Nullability::DefinitelyNullable) {
                WARN(binary->getExpr()->location, "value may be null; unwrap it with a postfix '!' to silence this warning");
            }
        } else if (llvm::isa<ConstantNull>(binary->right)) {
            ASSERT(binary->op == Token::Equal || binary->op == Token::NotEqual);
            if (binary->getExpr() && analyzeNullability(binary->left, binary) == Nullability::DefinitelyNotNull) {
                WARN(binary->getExpr()->location, "value cannot be null here; null check can be removed");
            }
        }
        break;
    }
    case ValueKind::LoadInst: {
        auto load = llvm::cast<LoadInst>(value);
        if (auto expr = llvm::dyn_cast_or_null<UnaryExpr>(load->expr)) {
            if (expr->getOperand().type.isOptionalType() && analyzeNullability(load->value, load) == Nullability::DefinitelyNullable) {
                WARN(expr->location, "dereferenced pointer may be null; unwrap it with a postfix '!' to silence this warning");
            }
        }
        break;
    }
    case ValueKind::GEPInst: {
        auto gep = llvm::cast<GEPInst>(value);
        auto* call = gep->expr ? llvm::dyn_cast<CallExpr>(gep->expr) : nullptr;
        if (call && call->isMethodCall() && call->getFunctionName() == "data" && call->type.isOptionalType()
            && analyzeNullability(gep->pointer, gep) == Nullability::DefinitelyNullable) {
            WARN(call->location, "value may be null; unwrap it with a postfix '!' to silence this warning");
        }
        break;
    }
    case ValueKind::ConstGEPInst: {
        auto gep = llvm::cast<ConstGEPInst>(value);
        if (gep->expr) {
            if (gep->expr->base->type.isOptionalType() && !gep->expr->base->isThis()
                && analyzeNullability(gep->pointer, gep) == Nullability::DefinitelyNullable) {
                WARN(gep->expr->base->location, "value may be null; unwrap it with a postfix '!' to silence this warning");
            }
        }
        break;
    }
    default:
        break;
    }
}
