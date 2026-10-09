#include "null-analyzer.h"
#include "../ast/decl.h"
#include "../ast/module.h"
#include "../backend/ir.h"
#include "../backend/irgen.h"
#include <optional>

using namespace cx;

Nullability NullAnalyzer::analyzeNullability(Value* nullableValue, Instruction* startFrom) {
    visited.clear();
    return analyzeNullability_recursive(nullableValue, startFrom);
}

static Value* stripCasts(Value* value) {
    while (auto* cast = llvm::dyn_cast<CastInst>(value)) {
        value = cast->value;
    }
    return value;
}

// The hidden context parameter shifts user arguments by one; extern callees have none.
static size_t callUserArgsOffset(CallInst* call) {
    if (auto* function = llvm::dyn_cast<Function>(call->function)) return function->hasContextParam ? 1 : 0;
    auto* type = call->function->getType();
    while (type->isPointerType() && !type->getPointee()->isFunctionType()) {
        type = type->getPointee();
    }
    // Unresolved callees (e.g. calls through a null placeholder after an
    // earlier error) have no function type; analyzing arg 0 keeps going.
    auto* functionType = llvm::dyn_cast_or_null<IRFunctionType>(type->isPointerType() ? type->getPointee() : nullptr);
    return functionType && functionType->hasContextParam ? 1 : 0;
}

// True when two address values denote the same storage: identical, or loads through the same base
// (e.g. separate `load t_0` nodes for one spilled parameter).
static bool valuesAlias(Value* a, Value* b) {
    a = stripCasts(a);
    b = stripCasts(b);
    if (a == b) return true;
    auto* loadA = llvm::dyn_cast<LoadInst>(a);
    auto* loadB = llvm::dyn_cast<LoadInst>(b);
    if (loadA && loadB) return valuesAlias(loadA->value, loadB->value);
    return false;
}

static bool callArgMayNull(Value* arg, Value* nullableValue, int gepIndex) {
    arg = stripCasts(arg);
    if (valuesAlias(arg, nullableValue)) return true;
    auto* argGep = llvm::dyn_cast<ConstGEPInst>(arg);
    auto* valueGep = llvm::dyn_cast<ConstGEPInst>(nullableValue);
    if (argGep && valueGep && argGep->index == valueGep->index && valuesAlias(argGep->pointer, valueGep->pointer)) return true;
    if (!argGep || !valuesAlias(argGep->pointer, nullableValue)) return false;
    if (gepIndex != -1) {
        // Tracking a struct field: only the same field (or the whole base, handled above) aliases.
        return argGep->index == gepIndex;
    }
    // Tracking a whole optional: a callee receiving its tag may null it; a payload-only
    // address cannot, since writing the payload leaves the tag unchanged.
    return argGep->index == IRGenerator::optionalTagFieldIndex;
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

        if (auto* call = llvm::dyn_cast<CallInst>(currentInst)) {
            // Callees may write globals even without receiving them.
            if (nullableValue->isGlobal()) return Nullability::DefinitelyNullable;
            for (Value* arg : call->args) {
                if (callArgMayNull(arg, nullableValue, gepIndex)) {
                    return Nullability::DefinitelyNullable;
                }
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

    llvm::SmallVector<BasicBlock*, 8> predecessors(block->predecessors.begin(), block->predecessors.end());
    if (auto it = switchPredecessors.find(block); it != switchPredecessors.end()) {
        predecessors.append(it->second.begin(), it->second.end());
    }
    if (predecessors.empty()) {
        return Nullability::DefinitelyNullable;
    }

    visited.insert(block);
    int notNullPredecessors = 0;
    int nullPredecessors = 0;

    for (auto predecessor : predecessors) {
        if (visited.count(predecessor)) {
            continue;
        }
        auto nullability = analyzeNullability_fromPredecessor(nullableValue, predecessor, block, gepIndex);
        if (nullability == Nullability::DefinitelyNullable) {
            return nullability;
        } else if (nullability == Nullability::DefinitelyNotNull) {
            notNullPredecessors++;
        } else if (nullability == Nullability::DefinitelyNull) {
            nullPredecessors++;
        }
    }

    if (nullPredecessors == predecessors.size()) {
        return Nullability::DefinitelyNull;
    }
    if (notNullPredecessors == predecessors.size()) {
        return Nullability::DefinitelyNotNull;
    }
    // Null on some path but not all is maybe-null; only unknown everywhere is
    // indefinite.
    if (nullPredecessors > 0) {
        return Nullability::DefinitelyNullable;
    }
    return Nullability::IndefiniteNullability;
}

static bool isTagOf(Value* side, Value* nullableValue, int gepIndex) {
    // The tag describes the whole optional, so payload tracking (which uses a field index)
    // still matches tag checks on the whole value.
    if (auto* extract = llvm::dyn_cast<ExtractInst>(side)) {
        return extract->index == IRGenerator::optionalTagFieldIndex
            && (extract->aggregate == nullableValue || extract->aggregate->loads(nullableValue, -1)
                || (gepIndex != -1 && extract->aggregate->loads(nullableValue, gepIndex)));
    }
    if (auto* load = llvm::dyn_cast<LoadInst>(side)) {
        auto* gep = llvm::dyn_cast<ConstGEPInst>(load->value);
        return gep && gep->index == IRGenerator::optionalTagFieldIndex
            && (gep->pointer == nullableValue || gep->pointer->loads(nullableValue, -1) || (gepIndex != -1 && gep->pointer->loads(nullableValue, gepIndex)));
    }
    return false;
}

// Resolves a temp optional's tag from the constant stored to it before this use
// (how `None` materializes in a condition). Strictly gives up on any other
// call or store first: those could have overwritten the tag.
static std::optional<int64_t> resolveTempTag(AllocaInst* temp, BasicBlock* block, Value* use) {
    int useIndex = -1;
    for (int i = 0; i < (int)block->body.size(); ++i) {
        if (block->body[i] == use) {
            useIndex = i;
            break;
        }
    }
    if (useIndex == -1) return std::nullopt;
    for (int i = useIndex - 1; i >= 0; --i) {
        Value* inst = block->body[i];
        if (llvm::isa<CallInst>(inst)) return std::nullopt;
        if (auto* store = llvm::dyn_cast<StoreInst>(inst)) {
            auto* gep = llvm::dyn_cast<ConstGEPInst>(store->pointer);
            if (gep && gep->pointer == temp && gep->index == IRGenerator::optionalTagFieldIndex) {
                if (auto* constant = llvm::dyn_cast<ConstantInt>(store->value)) return constant->value.getSExtValue();
                return std::nullopt;
            }
            return std::nullopt;
        }
    }
    return std::nullopt;
}

static std::optional<int64_t> resolveTagConstant(Value* side, BasicBlock* block) {
    if (auto* constant = llvm::dyn_cast<ConstantInt>(side)) return constant->value.getSExtValue();
    auto* load = llvm::dyn_cast<LoadInst>(side);
    if (!load) return std::nullopt;
    auto* gep = llvm::dyn_cast<ConstGEPInst>(load->value);
    if (!gep || gep->index != IRGenerator::optionalTagFieldIndex) return std::nullopt;
    auto* temp = llvm::dyn_cast<AllocaInst>(gep->pointer);
    if (!temp) return std::nullopt;
    return resolveTempTag(temp, block, load);
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
            // Either side may hold the tag: `is` compares two loaded tags, so
            // the constant side can be a temp whose stored tag must be resolved.
            Value* sides[] = {binary->left, binary->right};
            for (int s = 0; s < 2; ++s) {
                if (!isTagOf(sides[s], nullableValue, gepIndex)) continue;
                auto tag = resolveTagConstant(sides[1 - s], predecessor);
                if (!tag) continue;
                bool isSomeTag = *tag == IRGenerator::getOptionalSomeTag();
                bool isNoneTag = *tag == IRGenerator::getOptionalNoneTag();
                if (!isSomeTag && !isNoneTag) continue;
                // A resolved tag check is decisive: the None side is null, not maybe-null.
                bool trueMeansNotNull = ((binary->op == Token::Equal) == isSomeTag) != negated;
                if (destination == condBr->trueBlock) return trueMeansNotNull ? Nullability::DefinitelyNotNull : Nullability::DefinitelyNull;
                if (destination == condBr->falseBlock) return trueMeansNotNull ? Nullability::DefinitelyNull : Nullability::DefinitelyNotNull;
                return Nullability::IndefiniteNullability;
            }
        }

        // `x == None` lowers to a stdlib Optional== call on (x, None-temp); the
        // callee check keeps user-defined equality (or shadowed overloads) out.
        if (auto* call = llvm::dyn_cast<CallInst>(condition)) {
            auto* binExpr = llvm::dyn_cast_or_null<BinaryExpr>(call->expr);
            auto* callee = binExpr ? llvm::dyn_cast<FunctionDecl>(binExpr->calleeDecl) : nullptr;
            size_t offset = callUserArgsOffset(call);
            if (binExpr && (binExpr->op == Token::Equal || binExpr->op == Token::NotEqual) && call->args.size() == 2 + offset
                && binExpr->getLHS().type.isOptionalType() && binExpr->getRHS().type.isOptionalType() && callee
                && callee->getModule() == Module::getStdlibModule()) {
                for (int s = 0; s < 2; ++s) {
                    Value* selfSide = call->args[offset + size_t(s)];
                    if (selfSide != nullableValue && !selfSide->loads(nullableValue, gepIndex)) continue;
                    auto* temp = llvm::dyn_cast<AllocaInst>(call->args[offset + size_t(1 - s)]);
                    if (!temp) continue;
                    auto tag = resolveTempTag(temp, predecessor, call);
                    if (!tag || *tag != IRGenerator::getOptionalNoneTag()) continue;
                    bool trueMeansNull = (binExpr->op == Token::Equal) != binExpr->negateResult != negated;
                    if (destination == condBr->trueBlock) return trueMeansNull ? Nullability::DefinitelyNull : Nullability::DefinitelyNotNull;
                    if (destination == condBr->falseBlock) return trueMeansNull ? Nullability::DefinitelyNotNull : Nullability::DefinitelyNull;
                    return Nullability::IndefiniteNullability;
                }
            }
        }
    }

    if (auto switchInst = llvm::dyn_cast<SwitchInst>(lastInst)) {
        // Switches over optionals compare the tag field; cases carry the
        // Optional Some/None tags as integers. (Pointer-implemented optionals
        // cannot be switched on.)
        bool switchesNullable = isTagOf(switchInst->condition, nullableValue, gepIndex);
        if (switchesNullable) {
            int64_t someTag = IRGenerator::getOptionalSomeTag();
            int64_t noneTag = IRGenerator::getOptionalNoneTag();
            bool hasSome = false, hasNone = false, hasOther = false;
            for (auto& [caseValue, caseBlock] : switchInst->cases) {
                if (caseBlock != destination) continue;
                auto* constant = llvm::dyn_cast<ConstantInt>(caseValue);
                if (!constant) {
                    hasOther = true;
                } else if (constant->value.getSExtValue() == someTag) {
                    hasSome = true;
                } else if (constant->value.getSExtValue() == noneTag) {
                    hasNone = true;
                } else {
                    hasOther = true;
                }
            }
            if (hasNone && !hasSome && !hasOther) return Nullability::DefinitelyNull;
            if (hasSome && !hasNone && !hasOther) return Nullability::DefinitelyNotNull;
            if (hasSome || hasNone || hasOther) return Nullability::DefinitelyNullable;
            // Otherwise this is the default block, reached for any tag but the cased
            // ones: with only Some cased the value is None and vice versa (like the
            // tag comparison above, this assumes valid tags).
            if (destination == switchInst->defaultBlock) {
                bool someCased = false, noneCased = false, unknownCased = false;
                for (auto& [caseValue, caseBlock] : switchInst->cases) {
                    auto* constant = llvm::dyn_cast<ConstantInt>(caseValue);
                    if (!constant) {
                        unknownCased = true;
                    } else if (constant->value.getSExtValue() == someTag) {
                        someCased = true;
                    } else if (constant->value.getSExtValue() == noneTag) {
                        noneCased = true;
                    } else {
                        unknownCased = true;
                    }
                }
                if (!unknownCased) {
                    if (someCased && !noneCased) return Nullability::DefinitelyNull;
                    if (noneCased && !someCased) return Nullability::DefinitelyNotNull;
                }
            }
        }
    }

    return analyzeNullability_recursive(nullableValue, lastInst, gepIndex);
}

// Runs after type-checking (which implicitly unwraps optionals as needed),
// emitting warnings for null-safety violations.
void NullAnalyzer::analyze(IRModule* module) {
    switchPredecessors.clear();
    for (auto function : module->functions) {
        for (auto block : function->body) {
            if (block->body.empty()) continue;
            if (auto switchInst = llvm::dyn_cast<SwitchInst>(block->body.back())) {
                switchPredecessors[switchInst->defaultBlock].push_back(block);
                for (auto& [caseValue, caseBlock] : switchInst->cases) {
                    switchPredecessors[caseBlock].push_back(block);
                }
            }
        }
    }
    for (auto function : module->functions) {
        // The analyzer is reused across modules, so sites must re-warn.
        warnedRanges.clear();
        for (auto block : function->body) {
            for (auto inst : block->body) {
                analyze(inst);
            }
        }
    }
}

void NullAnalyzer::warnNullabilityOnce(Location begin, Location end, const char* message) {
    uint64_t packed = (uint64_t(uint16_t(begin.line)) << 48) | (uint64_t(uint16_t(begin.column)) << 32) | (uint64_t(uint16_t(end.line)) << 16)
                    | uint64_t(uint16_t(end.column));
    if (!warnedRanges.insert({begin.file, packed}).second) return;
    reportWarning(begin, message, {}, end);
}

void NullAnalyzer::warnForNullability(Location begin, Location end, Nullability nullability, const char* nullMessage, const char* nullableMessage) {
    if (nullability == Nullability::DefinitelyNull) {
        warnNullabilityOnce(begin, end, nullMessage);
    } else if (nullability == Nullability::DefinitelyNullable) {
        warnNullabilityOnce(begin, end, nullableMessage);
    }
}

void NullAnalyzer::analyze(Value* value) {
    switch (value->kind) {
    case ValueKind::CallInst: {
        auto call = llvm::cast<CallInst>(value);
        if (auto* callExpr = llvm::dyn_cast_or_null<CallExpr>(call->expr)) {
            if (auto receiverType = callExpr->receiverType) {
                if (receiverType.isOptionalType()) {
                    // TODO: Store the implicit 'this' receiver to the call expr during typechecking to simplify this code.
                    const Expr* target = callExpr->getReceiver() ? callExpr->getReceiver() : callExpr;
                    Value* receiver = stripCasts(call->args[callUserArgsOffset(call)]);
                    // Value-optionals pass the payload address; analyze the optional itself so tag checks apply.
                    if (!receiverType.isImplementedAsPointer()) {
                        if (auto* gep = llvm::dyn_cast<ConstGEPInst>(receiver)) {
                            if (gep->index != IRGenerator::optionalTagFieldIndex) {
                                receiver = stripCasts(gep->pointer);
                            }
                        }
                    }
                    warnForNullability(getExprRangeStart(*target), target->endLocation, analyzeNullability(receiver, call), "receiver is null here",
                                       "receiver may be null; unwrap it with a postfix '!' to silence this warning");
                }
            }
        }
        break;
    }
    case ValueKind::BinaryInst: {
        auto binary = llvm::cast<BinaryInst>(value);
        if (llvm::StringRef(binary->name).starts_with("__implicit_unwrap")) {
            if (binary->getExpr()) {
                // Value-implemented optionals unwrap through a tag comparison, so the
                // operand here is the boolean result; analyze the optional it tests.
                Value* unwrapped = binary->left;
                if (auto compare = llvm::dyn_cast<BinaryInst>(unwrapped)) {
                    auto* extract = llvm::dyn_cast<ExtractInst>(compare->left);
                    auto* tag = llvm::dyn_cast<ConstantInt>(compare->right);
                    if (compare->op == Token::Equal && extract && tag && extract->index == IRGenerator::optionalTagFieldIndex
                        && tag->value.getSExtValue() == IRGenerator::getOptionalSomeTag()) {
                        unwrapped = extract->aggregate;
                    }
                }
                warnForNullability(getExprRangeStart(*binary->getExpr()), binary->getExpr()->endLocation, analyzeNullability(unwrapped, binary),
                                   "value is null here", "value may be null; unwrap it with a postfix '!' to silence this warning");
            }
        } else if ((binary->op == Token::Equal || binary->op == Token::NotEqual) && llvm::isa<ConstantNull>(binary->right)) {
            // Ordering comparisons against null (e.g. `p < null`) carry no
            // null-check meaning, so only equality ops warn here.
            if (auto* binExpr = llvm::dyn_cast_or_null<BinaryExpr>(binary->getExpr())) {
                if (binExpr->redundantNullCheckWarned) break;
            }
            if (binary->getExpr() && analyzeNullability(binary->left, binary) == Nullability::DefinitelyNotNull) {
                warnNullabilityOnce(getExprRangeStart(*binary->getExpr()), binary->getExpr()->endLocation,
                                    "value cannot be null here; null check can be removed");
            }
        }
        break;
    }
    case ValueKind::LoadInst: {
        auto load = llvm::cast<LoadInst>(value);
        if (auto expr = llvm::dyn_cast_or_null<UnaryExpr>(load->expr)) {
            if (expr->getOperand().type.isOptionalType()) {
                warnForNullability(getExprRangeStart(*expr), expr->endLocation, analyzeNullability(load->value, load), "dereferenced pointer is null here",
                                   "dereferenced pointer may be null; unwrap it with a postfix '!' to silence this warning");
            }
        }
        break;
    }
    case ValueKind::GEPInst: {
        auto gep = llvm::cast<GEPInst>(value);
        auto* call = gep->expr ? llvm::dyn_cast<CallExpr>(gep->expr) : nullptr;
        if (call && call->isMethodCall() && call->getFunctionName() == "data" && call->type.isOptionalType()) {
            warnForNullability(getExprRangeStart(*call), call->endLocation, analyzeNullability(gep->pointer, gep), "value is null here",
                               "value may be null; unwrap it with a postfix '!' to silence this warning");
        }
        break;
    }
    case ValueKind::ConstGEPInst: {
        auto gep = llvm::cast<ConstGEPInst>(value);
        if (gep->expr) {
            if (gep->expr->base->type.isOptionalType() && !gep->expr->base->isThis()) {
                warnForNullability(getExprRangeStart(*gep->expr->base), gep->expr->base->endLocation, analyzeNullability(gep->pointer, gep),
                                   "value is null here", "value may be null; unwrap it with a postfix '!' to silence this warning");
            }
        }
        break;
    }
    default:
        break;
    }
}
