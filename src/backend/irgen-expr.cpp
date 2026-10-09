#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/Path.h>
#include <llvm/Support/SaveAndRestore.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../driver/driver.h"
#include "../support/utility.h"

using namespace cx;

Value* IRGenerator::emitVarExpr(const VarExpr& expr) {
    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.decl)) {
        return emitEnumCase(*enumCase, {}, expr.isMovedFrom);
    }
    Value* value = getValue(expr.decl);
    if (emittingGlobalInitializer) {
        // Immutable globals lower to real globals (see emitVarDecl), but a
        // global initializer can't load from another global; splice in the
        // referenced initializer instead, as if it were still inlined.
        if (auto* varDecl = llvm::dyn_cast_or_null<VarDecl>(expr.decl)) {
            if (varDecl->isGlobal() && varDecl->isConst) {
                if (auto* global = llvm::dyn_cast<GlobalVariable>(value)) {
                    if (global->value) value = global->value;
                }
            }
        }
    }
    return value;
}

Value* IRGenerator::emitStringLiteralExpr(const StringLiteralExpr& expr) {
    if ((expr.type.removeOptional().isPointerType() && expr.type.removeOptional().getPointee().isChar())
        || (expr.type.removeOptional().isArrayPointer() && expr.type.removeOptional().getElementType().isChar())) {
        return createGlobalStringPtr(expr.value);
    }

    if (emittingGlobalInitializer && expr.hasType()) {
        // Build the string object as a constant aggregate instead of calling the constructor.
        // Layout: string { characters: Slice<char> { data: char[*], size: int } }.
        auto* stringType = getIRType(expr.type);
        auto stringFields = stringType->getFields();
        auto charactersField = llvm::find_if(stringFields, [](const IRField& field) { return field.name == "characters"; });
        ASSERT(charactersField != stringFields.end());
        auto arrayRefFields = charactersField->type->getFields();
        auto dataField = llvm::find_if(arrayRefFields, [](const IRField& field) { return field.name == "data"; });
        auto sizeField = llvm::find_if(arrayRefFields, [](const IRField& field) { return field.name == "size"; });
        ASSERT(dataField != arrayRefFields.end() && sizeField != arrayRefFields.end());

        auto* arrayRef = createInsertValue(createUndefined(charactersField->type), createGlobalStringPtr(expr.value), dataField - arrayRefFields.begin());
        arrayRef = createInsertValue(arrayRef, createConstantInt(Type::getInt32(), expr.value.size()), sizeField - arrayRefFields.begin());
        return createInsertValue(createUndefined(stringType), arrayRef, charactersField - stringFields.begin());
    }

    auto* stringPtr = createGlobalStringPtr(expr.value);
    auto* size = createConstantInt(Type::getInt32(), expr.value.size());
    auto* alloca = createEntryBlockAlloca(BasicType::get("string", {}), "__str");
    Function* stringConstructor = nullptr;

    for (auto* decl : Module::getStdlibModule()->symbolTable.findInTopLevelScope("string.init")) {
        auto params = llvm::cast<ConstructorDecl>(decl)->getParams();
        if (params.size() == 2 && params[0].type.isPointerType() && params[1].type.isInt32()) {
            checkImplicitCalleeIsChecked(*decl, "string.init");
            stringConstructor = getFunction(*llvm::cast<ConstructorDecl>(decl));
            break;
        }
    }

    ASSERT(stringConstructor);
    createCall(stringConstructor, {alloca, stringPtr, size}, nullptr);
    return alloca;
}

Value* IRGenerator::emitCharacterLiteralExpr(const CharacterLiteralExpr& expr) {
    return createConstantInt(expr.type, expr.value);
}

Value* IRGenerator::emitIntLiteralExpr(const IntLiteralExpr& expr) {
    // Integer literals may be typed as floating-point when used in a context
    // that requires a floating-point value. It might make sense to combine
    // IntLiteralExpr and FloatLiteralExpr into a single class.
    if (expr.type.isFloatingPoint()) {
        return createConstantFP(expr.type, expr.getValue().roundToDouble());
    }

    return createConstantInt(expr.type, expr.getValue());
}

Value* IRGenerator::emitFloatLiteralExpr(const FloatLiteralExpr& expr) {
    return createConstantFP(expr.type, expr.value);
}

Value* IRGenerator::emitBoolLiteralExpr(const BoolLiteralExpr& expr) {
    return createConstantBool(expr.value);
}

Value* IRGenerator::emitNullLiteralExpr(const NullLiteralExpr& expr) {
    if (expr.type.isImplementedAsPointer()) {
        return createConstantNull(expr.type);
    } else {
        return emitOptionalConstruction(expr.type.getWrappedType(), nullptr, expr.isMovedFrom);
    }
}

static EnumDecl& getOptionalEnumDecl() {
    auto* typeTemplate = llvm::cast<TypeTemplate>(Module::getStdlibModule()->symbolTable.findOne("Optional"));
    auto* enumDecl = llvm::cast<EnumDecl>(typeTemplate->typeDecl);
    auto* someCase = enumDecl->getCaseByName("Some");
    ASSERT(someCase && someCase->associatedType && someCase->associatedType.getAnonymousStructElements().size() == 1);
    ASSERT(enumDecl->getCaseByName("None"));
    return *enumDecl;
}

int64_t IRGenerator::getOptionalSomeTag() {
    return getOptionalEnumDecl().getCaseByName("Some")->value->getConstantIntegerValue().getSExtValue();
}

int64_t IRGenerator::getOptionalNoneTag() {
    return getOptionalEnumDecl().getCaseByName("None")->value->getConstantIntegerValue().getSExtValue();
}

Value* IRGenerator::emitOptionalConstruction(Type wrappedType, Expr* arg, bool isMovedFrom) {
    auto* decl = Module::getStdlibModule()->symbolTable.findOne("Optional");
    auto* enumDecl = llvm::cast<EnumDecl>(llvm::cast<TypeTemplate>(decl)->instantiate(GenericArg(wrappedType)));
    auto* enumCase = enumDecl->getCaseByName(arg ? "Some" : "None");
    ASSERT(enumCase);
    if (arg) {
        NamedValue argValue(arg);
        return emitEnumCase(*enumCase, llvm::ArrayRef<NamedValue>(&argValue, 1), isMovedFrom);
    }
    return emitEnumCase(*enumCase, {}, isMovedFrom);
}

Value* IRGenerator::emitOptionalHasValueTest(Value* enumValue) {
    auto* tag = createExtractValue(enumValue, optionalTagFieldIndex);
    return createBinaryOp(Token::Equal, tag, createConstantInt(Type::getInt32(), getOptionalSomeTag()), nullptr);
}

Value* IRGenerator::emitOptionalPayloadPtr(Value* enumPtr, Type wrappedType) {
    // The Some payload is a single-element anonymous struct, so the wrapped value sits at offset zero of the payload union.
    return createCast(createGEP(enumPtr, optionalPayloadFieldIndex), wrappedType.getPointerTo());
}

Value* IRGenerator::emitOptionalUnwrap(const Expr& operand, const Expr& expr, const llvm::Twine& name) {
    auto* value = emitLvalueExpr(operand);
    llvm::StringRef message = "Unwrap failed";
    bool checkNull = !disablesCheck(disabledChecks, DisabledChecks::Null);

    // An lvalue operand addresses the optional storage, so the unwrap addresses the payload:
    // address users (assignment, indexing) write through to it, while value users load through
    // the returned pointer exactly as before.
    if (value->getType()->isPointerType() && value->getType()->getPointee()->equals(getIRType(operand.type))) {
        if (operand.type.isImplementedAsPointer()) {
            if (checkNull) emitAssert(createLoad(value), &expr, expr.location, message, name);
            return value;
        }
        if (checkNull) emitAssert(emitOptionalHasValueTest(createLoad(value)), &expr, expr.location, message, name);
        return emitOptionalPayloadPtr(value, operand.type.getWrappedType());
    }

    if (operand.type.isImplementedAsPointer()) {
        if (checkNull) emitAssert(value, &expr, expr.location, message, name);
        return value;
    } else {
        if (checkNull) emitAssert(emitOptionalHasValueTest(value), &expr, expr.location, message, name);
        if (!value->getType()->isPointerType()) value = createTempAlloca(value);
        return createLoad(emitOptionalPayloadPtr(value, operand.type.getWrappedType()));
    }
}

Value* IRGenerator::emitUndefinedLiteralExpr(const UndefinedLiteralExpr& expr) {
    return createUndefined(expr.type);
}

Value* IRGenerator::emitArrayLiteralExpr(const ArrayLiteralExpr& expr) {
    if (expr.elements.empty() && expr.type.isSlice()) {
        auto* irType = getIRType(expr.type);
        auto fields = irType->getFields();
        ASSERT(fields.size() == 2);
        Value* arrayRef = createInsertValue(createUndefined(irType), createConstantNull(fields[0].type), 0);
        return createInsertValue(arrayRef, createConstantInt(fields[1].type, 0), 1);
    }

    Value* array = createUndefined(expr.type);
    auto index = 0;

    for (auto& element : expr.elements) {
        auto* value = emitExpr(*element);
        array = createInsertValue(array, value, index++);
    }

    return array;
}

Value* IRGenerator::emitAggregateElements(Type type, llvm::ArrayRef<NamedValue> elements) {
    Value* aggregate = createUndefined(type);
    int index = 0;
    for (auto& element : elements) {
        aggregate = createInsertValue(aggregate, emitExpr(*element.value), index++);
    }
    return aggregate;
}

Value* IRGenerator::emitAnonymousStructExpr(const AnonymousStructExpr& expr) {
    return emitAggregateElements(expr.type, expr.elements);
}

Value* IRGenerator::emitImplicitNullComparison(Value* operand, BinaryOperator op) {
    return createBinaryOp(op, operand, createConstantNull(operand->getType()), nullptr);
}

Value* IRGenerator::emitNot(const UnaryExpr& expr) {
    auto* operand = emitExpr(expr.getOperand());
    if (operand->getType()->isPointerType()) {
        return emitImplicitNullComparison(operand, Token::Equal);
    }
    return createNot(operand);
}

Value* IRGenerator::emitUnaryExpr(const UnaryExpr& expr) {
    switch (expr.op) {
    case Token::Plus:
        return emitExpr(expr.getOperand());
    case Token::Minus: {
        Type operandType = expr.getOperand().type;
        // Element-wise negation over fixed-size arrays (e.g. `-float[3]`).
        // Like the binary array ops, element negation is unchecked even for
        // integers; symbolic sizes were rejected during typechecking.
        if (operandType.isArrayType() && operandType.isConcreteArray()) {
            int64_t arraySize = operandType.getArraySize();
            auto* arrayIRType = getIRType(operandType);
            auto* operand = emitExpr(expr.getOperand());
            Value* arrayPtr = operand->getType()->isPointerType() ? operand : createTempAlloca(operand);
            auto* zero = createConstantInt(Type::getInt32(), 0);

            // Large arrays lower to a counted loop; small ones stay unrolled.
            if (arraySize > 4) {
                auto indexType = Type::getInt32();
                auto* indexAlloca = createEntryBlockAlloca(indexType);
                createStore(zero, indexAlloca);

                auto* resultAlloca = createEntryBlockAlloca(arrayIRType);

                auto* function = currentFunction;
                auto* cond = new BasicBlock("arrayop.cond", function);
                auto* body = new BasicBlock("arrayop.body", function);
                auto* end = new BasicBlock("arrayop.end", function);
                createBr(cond);

                setInsertPoint(cond);
                auto* index = createLoad(indexAlloca);
                createCondBr(createBinaryOp(Token::Less, index, createConstantInt(indexType, arraySize), &expr), body, end);

                setInsertPoint(body);
                auto* i = createLoad(indexAlloca);
                auto* elem = createLoad(createGEP(arrayPtr, {zero, i}));
                createStore(createNeg(elem), createGEP(resultAlloca, {zero, i}));
                createStore(createBinaryOp(Token::Plus, i, createConstantInt(indexType, 1), &expr), indexAlloca);
                createBr(cond);

                setInsertPoint(end);
                return resultAlloca;
            }

            Value* result = createUndefined(arrayIRType);
            for (int64_t i = 0; i < arraySize; ++i) {
                auto* idx = createConstantInt(Type::getInt32(), static_cast<int>(i));
                auto* elem = createLoad(createGEP(arrayPtr, {zero, idx}));
                result = createInsertValue(result, createNeg(elem), static_cast<int>(i));
            }
            return result;
        }
        auto* operand = emitExpr(expr.getOperand());
        auto* type = operand->getType();
        // Global initializers can't contain the trap's control flow; they only
        // ever negate constant expressions (such as enum tags) anyway, so emit it plain.
        if (!emittingGlobalInitializer && options.mode != BuildMode::ReleaseFast && !disablesCheck(disabledChecks, DisabledChecks::Overflow)
            && (type->isInteger() || type->isChar())) {
            // Checked negation is 0 - x: it traps on MIN, and on any nonzero unsigned operand.
            return emitCheckedArithmetic(Token::Minus, createConstantInt(type, 0), operand, expr);
        }
        return createNeg(operand);
    }
    case Token::MinusWrap: {
        if (expr.isFoldableIntConstant()) {
            return createConstantInt(expr.type, expr.getConstantIntegerValue());
        }
        auto* operand = emitExpr(expr.getOperand());
        auto* zero = createConstantInt(operand->getType(), 0);
        return emitWrappingArithmetic(Token::Minus, zero, operand, expr);
    }
    case Token::Star: {
        auto operand = emitExpr(expr.getOperand());
        if (auto load = llvm::dyn_cast<LoadInst>(operand)) {
            load->expr = &expr;
        }
        return operand;
    }
    case Token::And: {
        // A borrow already is the address; load it out of its slot instead of taking the slot's address.
        if (expr.getOperand().type.isReferenceType()) {
            return loadThroughStorageAddress(emitExprAsPointer(expr.getOperand()), expr.getOperand().type);
        }
        auto* value = emitExprAsPointer(expr.getOperand());
        // 'this' is an SSA value, not memory, so spill it to a temporary to form a real address.
        // FIXME: This is a point-in-time copy; stores through the address don't update the original.
        if (llvm::isa<Parameter>(value)) {
            value = createTempAlloca(value);
        }
        return value;
    }
    case Token::Not:
        // FIXME: Temporary hack. Lower implicit null checks such as `if (ptr)` and `if (!ptr)` when expression lowering is implemented.
        if (expr.getOperand().type.isOptionalType() && !expr.getOperand().type.getWrappedType().isPointerType()) {
            return createNot(emitOptionalHasValueTest(emitExpr(expr.getOperand())));
        }
        return emitNot(expr);
    case Token::Tilde:
        return createBitwiseNot(emitExpr(expr.getOperand()));
    case Token::Increment:
        return emitConstantIncrement(expr, 1);
    case Token::Decrement:
        return emitConstantIncrement(expr, -1);
    default:
        llvm_unreachable("invalid prefix operator");
    }
}

// TODO: Lower increment and decrement statements to compound assignments so this isn't needed.
Value* IRGenerator::emitConstantIncrement(const UnaryExpr& expr, int increment) {
    auto operandType = expr.getOperand().type;
    auto* ptr = emitLvalueExpr(expr.getOperand());
    if (operandType.isPointerType() && llvm::isa<AllocaInst>(ptr)) {
        ptr = createLoad(ptr);
    }
    auto* value = createLoad(ptr);
    Value* result;

    if (value->getType()->isInteger()) {
        result = createBinaryOp(Token::Plus, value, createConstantInt(value->getType(), increment), &expr);
    } else if (value->getType()->isPointerType()) {
        result = createGEP(value, {createConstantInt(Type::getInt32(), increment)});
    } else if (value->getType()->isFloatingPoint()) {
        result = createBinaryOp(Token::Plus, value, createConstantFP(value->getType(), increment), &expr);
    } else {
        llvm_unreachable("unknown increment/decrement operand type");
    }

    createStore(result, ptr);
    return nullptr;
}

// The right side may not run, and its boolean result cannot borrow its
// temporaries, so they die here instead of at statement end.
static Value* emitShortCircuit(IRGenerator& ir, const Expr& left, const Expr& right, bool isAnd) {
    auto* rhsBlock = new BasicBlock(isAnd ? "and.rhs" : "or.rhs", ir.insertBlock->parent);
    auto* endBlock = new BasicBlock(isAnd ? "and.end" : "or.end");

    Value* lhs = ir.emitBoolConvertibleOperand(left);
    if (isAnd) {
        ir.createCondBr(lhs, rhsBlock, endBlock, lhs);
    } else {
        ir.createCondBr(lhs, endBlock, rhsBlock, lhs);
    }

    ir.setInsertPoint(rhsBlock);
    ir.beginTempScope();
    Value* rhs = ir.emitBoolConvertibleOperand(right);
    ir.endTempScope();
    ir.createBr(endBlock, rhs);

    ir.setInsertPoint(endBlock);
    endBlock->parameter = new Parameter{ValueKind::Parameter, lhs->getType(), isAnd ? "and" : "or"};
    return endBlock->parameter;
}

Value* IRGenerator::emitLogicalAnd(const Expr& left, const Expr& right) {
    return emitShortCircuit(*this, left, right, true);
}

Value* IRGenerator::emitLogicalOr(const Expr& left, const Expr& right) {
    return emitShortCircuit(*this, left, right, false);
}

// Pointers and non-pointer optionals are not boolean values; conditions test them for null / some.
Value* IRGenerator::lowerImplicitBool(Value* value, const Expr& expr) {
    if (value->getType()->isPointerType()) return emitImplicitNullComparison(value);
    if (expr.type.isOptionalType() && !expr.type.getWrappedType().isPointerType()) return emitOptionalHasValueTest(value);
    return value;
}

Value* IRGenerator::emitBoolConvertibleOperand(const Expr& expr) {
    return lowerImplicitBool(emitExpr(expr), expr);
}

Value* IRGenerator::emitNullCoalescingExpr(const BinaryExpr& expr) {
    // The left side is evaluated once up front; both the null test and the value branch reuse it.
    auto* lhsValue = emitExpr(expr.getLHS());
    Value* hasValue;
    if (expr.getLHS().type.isImplementedAsPointer()) {
        hasValue = emitImplicitNullComparison(lhsValue);
    } else {
        hasValue = emitOptionalHasValueTest(lhsValue);
    }

    auto* function = insertBlock->parent;
    auto* valueBlock = new BasicBlock("coalesce.value", function);
    auto* defaultBlock = new BasicBlock("coalesce.default");
    auto* endBlock = new BasicBlock("coalesce.end");
    // The default runs only on null; see emitIfExpr.
    auto* defaultGuard = createTempGuard();
    auto* outerGuard = tempGuard;
    createCondBr(hasValue, valueBlock, defaultBlock);

    setInsertPoint(valueBlock);
    Value* thenValue;
    if (expr.type == expr.getLHS().type) {
        // The result is the optional itself (e.g. `int? ?? int?`); the tested value is already it.
        thenValue = lhsValue;
    } else {
        // Unwrap without asserting; the branch proves the value is non-null.
        Value* unwrapped = lhsValue;
        if (!expr.getLHS().type.isImplementedAsPointer()) {
            if (!unwrapped->getType()->isPointerType()) unwrapped = createTempAlloca(unwrapped);
            unwrapped = createLoad(emitOptionalPayloadPtr(unwrapped, expr.getLHS().type.getWrappedType()));
        }
        thenValue = createCastIfNeeded(unwrapped, expr.type);
    }
    createBr(endBlock, thenValue);

    setInsertPoint(defaultBlock);
    createStore(createConstantBool(true), defaultGuard);
    tempGuard = defaultGuard;
    createBr(endBlock, emitExpr(expr.getRHS()));
    tempGuard = outerGuard;

    setInsertPoint(endBlock);
    endBlock->parameter = new Parameter{ValueKind::Parameter, thenValue->getType(), "coalesce"};
    return endBlock->parameter;
}

Value* IRGenerator::createSelect(Value* condition, Value* trueValue, Value* falseValue) {
    ASSERT(trueValue->getType()->equals(falseValue->getType()));
    auto* function = insertBlock->parent;
    auto* trueBlock = new BasicBlock("select.true", function);
    auto* falseBlock = new BasicBlock("select.false", function);
    auto* endBlock = new BasicBlock("select.end");
    createCondBr(condition, trueBlock, falseBlock);
    setInsertPoint(trueBlock);
    createBr(endBlock, trueValue);
    setInsertPoint(falseBlock);
    createBr(endBlock, falseValue);
    setInsertPoint(endBlock);
    endBlock->parameter = new Parameter{ValueKind::Parameter, trueValue->getType(), "select"};
    return endBlock->parameter;
}

// Chars compare as unsigned, so arithmetic on them runs in 8-bit unsigned integers.
static IRType* lowerCharOperands(IRGenerator& ir, Value*& left, Value*& right) {
    auto* type = left->getType();
    if (!type->isChar()) return type;
    auto* uint8Type = getIRType(Type::getUInt8());
    left = ir.createCast(left, uint8Type);
    right = ir.createCast(right, uint8Type);
    return uint8Type;
}

Value* IRGenerator::emitWrappingArithmetic(Token::Kind op, Value* left, Value* right, const Expr& expr, Value** overflowedOut) {
    bool resultIsChar = left->getType()->isChar();
    auto* type = lowerCharOperands(*this, left, right);

    int width = getIntegerBitWidth(type);
    ASSERT(width != 0);
    bool isSigned = type->isSignedInteger();

    Value* result;
    Value* overflowed = nullptr;

    if (overflowedOut) {
        // The backends detect the overflow from this node: with.overflow
        // intrinsics in LLVM, manual checks in C (see CheckedArithInst).
        result = createCheckedArith(op, left, right, &expr);
        overflowed = createArithOverflow(result);
    } else if (width < 64) {
        // The operation is exact in 64 bits, so truncating back down wraps.
        // Widen only to 64 bits: MSVC and xcc (the playground's C compiler) don't support
        // __int128, so 64-bit operands use the in-width wrapping below instead of widening.
        auto* wideType = getIRType(isSigned ? Type::getInt64() : Type::getUInt64());
        result = createCast(createBinaryOp(op, createCast(left, wideType), createCast(right, wideType), &expr), type);
    } else {
        // Compute in the unsigned domain so the wrapping step isn't signed overflow in the C backend.
        auto* unsignedType = getIRType(getUnsignedIntegerType(width));
        auto* a = createCastIfNeeded(left, unsignedType);
        auto* b = createCastIfNeeded(right, unsignedType);
        result = createCastIfNeeded(createBinaryOp(op, a, b, &expr), type);
    }

    if (resultIsChar) result = createCast(result, getIRType(Type::getChar()));
    if (overflowedOut) *overflowedOut = overflowed;
    return result;
}

Value* IRGenerator::emitCheckedArithmetic(BinaryOperator op, Value* left, Value* right, const Expr& expr) {
    Value* overflowed = nullptr;
    auto* result = emitWrappingArithmetic(op, left, right, expr, &overflowed);
    emitAssert(createNot(overflowed), &expr, expr.location, "integer overflow", "overflow");
    return result;
}

Value* IRGenerator::emitSaturatingArithmetic(Token::Kind op, Value* left, Value* right, const Expr& expr) {
    if (op != Token::Star) {
        // The backends clamp from this node: sadd.sat/ssub.sat intrinsics in
        // LLVM, a manual clamp in C (see SaturatingArithInst).
        bool resultIsChar = left->getType()->isChar();
        lowerCharOperands(*this, left, right);
        Value* result = createSaturatingArith(op, left, right, &expr);
        if (resultIsChar) result = createCast(result, getIRType(Type::getChar()));
        return result;
    }

    // LLVM offers no saturating multiply, so clamp the checked product manually.
    Value* overflowed = nullptr;
    auto* wrapped = emitWrappingArithmetic(op, left, right, expr, &overflowed);

    auto* type = wrapped->getType();
    bool resultIsChar = type->isChar();
    IRType* limitType = type;
    if (resultIsChar) limitType = getIRType(Type::getUInt8());
    int width = getIntegerBitWidth(limitType);
    bool isSigned = limitType->isSignedInteger();
    auto min = llvm::APSInt::getMinValue(width, !isSigned);
    auto max = llvm::APSInt::getMaxValue(width, !isSigned);
    auto* minVal = createConstantInt(limitType, min);
    auto* maxVal = createConstantInt(limitType, max);
    if (resultIsChar) {
        minVal = createCast(minVal, type);
        maxVal = createCast(maxVal, type);
    }

    Value* sat;
    if (!isSigned) {
        sat = maxVal;
    } else {
        auto* zero = createConstantInt(left->getType(), 0);
        auto* leftNeg = createBinaryOp(Token::Less, left, zero, &expr);
        auto* rightNeg = createBinaryOp(Token::Less, right, zero, &expr);
        auto* towardMin = createBinaryOp(Token::NotEqual, leftNeg, rightNeg, &expr);
        sat = createSelect(towardMin, minVal, maxVal);
    }
    return createSelect(overflowed, sat, wrapped);
}

Value* IRGenerator::emitSaturatingLeftShift(Value* left, Value* right, const Expr& expr) {
    bool resultIsChar = left->getType()->isChar();
    auto* type = lowerCharOperands(*this, left, right);

    int width = getIntegerBitWidth(type);
    ASSERT(width != 0);
    bool isSigned = type->isSignedInteger();
    auto* unsignedType = getIRType(getUnsignedIntegerType(width));
    auto* zero = createConstantInt(type, 0);
    auto* isZero = createBinaryOp(Token::Equal, left, zero, &expr);
    auto* minVal = createConstantInt(type, llvm::APSInt::getMinValue(width, !isSigned));
    auto* maxVal = createConstantInt(type, llvm::APSInt::getMaxValue(width, !isSigned));
    Value* sat = isSigned ? createSelect(createBinaryOp(Token::Less, left, zero, &expr), minVal, maxVal) : maxVal;

    auto* b = createCastIfNeeded(right, unsignedType);
    auto* shiftTooBig = createBinaryOp(Token::GreaterOrEqual, b, createConstantInt(unsignedType, width), &expr);

    auto* function = insertBlock->parent;
    auto* inRange = new BasicBlock("shl_sat.inrange", function);
    auto* tooBig = new BasicBlock("shl_sat.toobig", function);
    auto* end = new BasicBlock("shl_sat.end");
    createCondBr(shiftTooBig, tooBig, inRange);

    setInsertPoint(tooBig);
    createBr(end, createSelect(isZero, zero, sat));

    setInsertPoint(inRange);
    auto* a = createCastIfNeeded(left, unsignedType);
    auto* shiftedU = createBinaryOp(Token::LeftShift, a, b, &expr);
    auto* shifted = createCastIfNeeded(shiftedU, type);
    auto* shiftAmount = createCastIfNeeded(b, type);
    auto* recovered = createBinaryOp(Token::RightShift, shifted, shiftAmount, &expr);
    auto* overflowed = createBinaryOp(Token::NotEqual, recovered, left, &expr);
    createBr(end, createSelect(overflowed, sat, shifted));

    setInsertPoint(end);
    end->parameter = new Parameter{ValueKind::Parameter, type, "shl_sat"};
    Value* result = end->parameter;
    if (resultIsChar) result = createCast(result, getIRType(Type::getChar()));
    return result;
}

// Element types the LLVM backend vectorizes directly (see codegenArrayOp).
// bool packs a byte per element in memory but lowers to i1, and float80
// and pointers have no SIMD lowering, so those keep the scalar expansion
// instead.
static bool isVectorFriendlyElement(IRType* type) {
    if (type->isBool()) return false;
    if (type->isChar() || type->isInteger()) return true;
    if (type->isFloatingPoint()) {
        return type->getName() != "float80";
    }
    return false;
}

Value* IRGenerator::emitPositiveModulo(Value* lhs, Value* rhs, const Expr* expr) {
    if (lhs->getType()->isUnsignedInteger()) return createBinaryOp(Token::Modulo, lhs, rhs, expr);
    // Positive remainder ((a % b) + b) % b. The operands are already evaluated values, so this doesn't re-evaluate them.
    auto* rem = createBinaryOp(Token::Modulo, lhs, rhs, expr);
    auto* shifted = createBinaryOp(Token::Plus, rem, rhs, expr);
    return createBinaryOp(Token::Modulo, shifted, rhs, expr);
}

Value* IRGenerator::emitBinaryExpr(const BinaryExpr& expr) {
    if (expr.isAssignment()) {
        return emitAssignment(expr);
    }

    if (expr.comparisonLowering) {
        // The comparison was lowered to a replacement AST over compiler-generated
        // temporaries during typechecking. Evaluate each side once and bind the
        // temporaries to the values, so operands with side effects run only once.
        // The bindings alias the values (no copies), so there is nothing to
        // destroy; they are removed right after the lowering is emitted.
        auto* lhsValue = emitExpr(expr.getLHS());
        auto* rhsValue = emitExpr(expr.getRHS());
        auto& bindings = scopes.back().valuesByDecl;
        bindings[expr.comparisonTempLHS] = lhsValue;
        bindings[expr.comparisonTempRHS] = rhsValue;
        auto* result = emitExpr(*expr.comparisonLowering);
        bindings.erase(expr.comparisonTempLHS);
        bindings.erase(expr.comparisonTempRHS);
        return result;
    }

    // Array programming (`float[3] + float[3]`, `float[3] * 2.0`, etc.):
    // element-wise ops over fixed-size arrays. Typechecking validated sizes
    // and returns array (arithmetic) or bool (==/!=); emit directly here.
    // Skip when an overload was selected (calleeDecl set, e.g. `char[N] == string`
    // via string overloads); only builtin element-wise ops reach here.
    if (expr.calleeDecl == nullptr) {
        // Match the typechecker's test: raw operand types, so optional arrays
        // compared against null reach the tag test below and pointers compare
        // as pointers instead of element-wise.
        Type leftT = expr.getLHS().type;
        Type rightT = expr.getRHS().type;
        bool leftIsArray = leftT.isArrayType() && leftT.isConcreteArray();
        bool rightIsArray = rightT.isArrayType() && rightT.isConcreteArray();
        bool isArrayOp =
            (leftIsArray || rightIsArray)
            && (expr.op == Token::Plus || expr.op == Token::Minus || expr.op == Token::Star || expr.op == Token::Slash || expr.op == Token::Modulo
                || expr.op == Token::PositiveModulo || expr.op == Token::Equal || expr.op == Token::NotEqual || expr.op == Token::And || expr.op == Token::Or
                || expr.op == Token::Xor || expr.op == Token::LeftShift || expr.op == Token::RightShift || isWrappingOrSaturatingOperator(expr.op));
        if (isArrayOp && (leftIsArray || rightIsArray)) {
            Type arrayT = leftIsArray ? leftT : rightT;
            int64_t arraySize = arrayT.getArraySize();
            auto* lhsValue = emitExpr(expr.getLHS());
            auto* rhsValue = emitExpr(expr.getRHS());
            Value* lhsPtr = nullptr;
            Value* rhsPtr = nullptr;
            Value* lhsScalar = nullptr;
            Value* rhsScalar = nullptr;
            if (leftIsArray) {
                lhsPtr = lhsValue->getType()->isPointerType() ? lhsValue : createTempAlloca(lhsValue);
            } else {
                lhsScalar = lhsValue;
            }
            if (rightIsArray) {
                rhsPtr = rhsValue->getType()->isPointerType() ? rhsValue : createTempAlloca(rhsValue);
            } else {
                rhsScalar = rhsValue;
            }
            bool isComparison = (expr.op == Token::Equal || expr.op == Token::NotEqual);
            Token::Kind combiner = expr.op == Token::Equal ? Token::And : Token::Or;

            // Vector-friendly elements stay whole in one node; the LLVM
            // backend emits SIMD for it directly, in every build mode.
            auto* arrayIRType = getIRType(arrayT);
            // Wrapping ops lower to the same SIMD add/sub/mul as wrapping array `+`/`-`/`*`.
            // Saturating ops need overflow clamps, so they stay on the scalar path.
            if (isVectorFriendlyElement(arrayIRType->getElementType()) && !isSaturatingOperator(expr.op)) {
                Value* left = leftIsArray ? lhsPtr : lhsScalar;
                Value* right = rightIsArray ? rhsPtr : rhsScalar;
                Token::Kind vectorOp = isWrappingOperator(expr.op) ? getWrappingOrSaturatingBaseOp(expr.op) : static_cast<Token::Kind>(expr.op);
                return createArrayOp(vectorOp, left, right, arrayIRType, &expr);
            }

            // Scalar fallback for the rest. PositiveModulo has no IR
            // instruction; expand ((a % b) + b) % b per element like scalars.
            // Element ops are unchecked (matching existing array semantics).
            auto emitElementOp = [&](Value* l, Value* r) -> Value* {
                if (isWrappingOperator(expr.op)) return emitWrappingArithmetic(getWrappingOrSaturatingBaseOp(expr.op), l, r, expr);
                if (expr.op == Token::LeftShiftSat) return emitSaturatingLeftShift(l, r, expr);
                if (isSaturatingOperator(expr.op)) return emitSaturatingArithmetic(getWrappingOrSaturatingBaseOp(expr.op), l, r, expr);
                if (expr.op != Token::PositiveModulo) return createBinaryOp(expr.op, l, r, &expr);
                if (l->getType()->isUnsignedInteger()) return createBinaryOp(Token::Modulo, l, r, &expr);
                auto* rem = createBinaryOp(Token::Modulo, l, r, &expr);
                auto* shifted = createBinaryOp(Token::Plus, rem, r, &expr);
                return createBinaryOp(Token::Modulo, shifted, r, &expr);
            };

            // Large arrays lower to a counted loop; small ones stay unrolled
            // for minimal overhead.
            if (arraySize > 4) {
                auto indexType = Type::getInt32();
                auto* indexAlloca = createEntryBlockAlloca(indexType);
                auto* zero = createConstantInt(indexType, 0);
                createStore(zero, indexAlloca);

                AllocaInst* resultAlloca = nullptr;
                AllocaInst* accAlloca = nullptr;
                if (isComparison) {
                    accAlloca = createEntryBlockAlloca(Type::getBool());
                    createStore(createConstantBool(expr.op == Token::Equal), accAlloca);
                } else {
                    resultAlloca = createEntryBlockAlloca(getIRType(arrayT));
                }

                auto* function = currentFunction;
                auto* cond = new BasicBlock("arrayop.cond", function);
                auto* body = new BasicBlock("arrayop.body", function);
                auto* end = new BasicBlock("arrayop.end", function);
                createBr(cond);

                setInsertPoint(cond);
                auto* index = createLoad(indexAlloca);
                createCondBr(createBinaryOp(Token::Less, index, createConstantInt(indexType, arraySize), &expr), body, end);

                setInsertPoint(body);
                auto* i = createLoad(indexAlloca);
                auto emitLoopElement = [&](Value* arrayPtr) -> Value* { return createLoad(createGEP(arrayPtr, {zero, i})); };
                Value* lhsElem = leftIsArray ? emitLoopElement(lhsPtr) : lhsScalar;
                Value* rhsElem = rightIsArray ? emitLoopElement(rhsPtr) : rhsScalar;
                if (isComparison) {
                    Value* cmp = emitElementOp(lhsElem, rhsElem);
                    createStore(createBinaryOp(combiner, createLoad(accAlloca), cmp, &expr), accAlloca);
                } else {
                    createStore(emitElementOp(lhsElem, rhsElem), createGEP(resultAlloca, {zero, i}));
                }
                createStore(createBinaryOp(Token::Plus, i, createConstantInt(indexType, 1), &expr), indexAlloca);
                createBr(cond);

                setInsertPoint(end);
                if (isComparison) {
                    return createLoad(accAlloca);
                }
                return resultAlloca;
            }

            auto emitArrayElement = [&](Value* arrayPtr, int64_t index) -> Value* {
                auto* zero = createConstantInt(Type::getInt32(), 0);
                auto* idx = createConstantInt(Type::getInt32(), static_cast<int>(index));
                auto* gep = createGEP(arrayPtr, {zero, idx});
                return createLoad(gep);
            };

            if (isComparison) {
                if (arraySize == 0) return createConstantBool(expr.op == Token::Equal);
                // `a == b` lowers to `(a[0]==b[0]) & (a[1]==b[1]) & ...` (bitwise
                // AND on bools, eager; equivalent to && for pure comparisons).
                // `!=` uses `|` (OR). Broadcast compares each element to the scalar.
                Value* result = nullptr;
                for (int64_t i = 0; i < arraySize; ++i) {
                    Value* lhsElem = leftIsArray ? emitArrayElement(lhsPtr, i) : lhsScalar;
                    Value* rhsElem = rightIsArray ? emitArrayElement(rhsPtr, i) : rhsScalar;
                    Value* cmp = emitElementOp(lhsElem, rhsElem);
                    result = result ? createBinaryOp(combiner, result, cmp, &expr) : cmp;
                }
                return result;
            } else {
                Value* result = createUndefined(arrayIRType);
                for (int64_t i = 0; i < arraySize; ++i) {
                    Value* lhsElem = leftIsArray ? emitArrayElement(lhsPtr, i) : lhsScalar;
                    Value* rhsElem = rightIsArray ? emitArrayElement(rhsPtr, i) : rhsScalar;
                    Value* elem = emitElementOp(lhsElem, rhsElem);
                    result = createInsertValue(result, elem, static_cast<int>(i));
                }
                return result;
            }
        }
    }

    if (expr.calleeDecl != nullptr) {
        auto* value = emitCallExpr(expr);
        if (expr.negateResult) value = createNot(value);
        return value;
    }

    if (expr.op == Token::Equal || expr.op == Token::NotEqual) {
        // Lower null checks on value-implemented optionals to a tag test, matching `if (opt)` and `if (!opt)`.
        // Pointer-implemented optionals take the generic path below (pointer compared against null).
        const Expr* optOperand = nullptr;
        if (expr.getLHS().isNullLiteralExpr() && expr.getRHS().type.isOptionalType()) {
            optOperand = &expr.getRHS();
        } else if (expr.getRHS().isNullLiteralExpr() && expr.getLHS().type.isOptionalType()) {
            optOperand = &expr.getLHS();
        }
        if (optOperand && !optOperand->type.isImplementedAsPointer()) {
            auto* hasValue = emitOptionalHasValueTest(emitExpr(*optOperand));
            return expr.op == Token::NotEqual ? hasValue : createNot(hasValue);
        }
    }

    switch (expr.op) {
    case Token::AndAnd:
        if (expr.isFoldableBoolConstant()) return createConstantBool(expr.getConstantBoolValue());
        return emitLogicalAnd(expr.getLHS(), expr.getRHS());

    case Token::OrOr:
        if (expr.isFoldableBoolConstant()) return createConstantBool(expr.getConstantBoolValue());
        return emitLogicalOr(expr.getLHS(), expr.getRHS());

    case Token::QuestionQuestion:
        return emitNullCoalescingExpr(expr);

    case Token::Is: {
        auto left = emitExprOrEnumTag(expr.getLHS(), nullptr);
        auto right = emitExprOrEnumTag(expr.getRHS(), nullptr);
        return createBinaryOp(Token::Equal, left, right, &expr);
    }

    case Token::PositiveModulo: {
        auto left = emitExprOrEnumTag(expr.getLHS(), nullptr);
        auto right = emitExprOrEnumTag(expr.getRHS(), nullptr);
        lowerCharOperands(*this, left, right);
        auto* result = emitPositiveModulo(left, right, &expr);
        return createCastIfNeeded(result, getIRType(expr.type));
    }

    default:
        if (isWrappingOrSaturatingOperator(expr.op) && expr.isFoldableIntConstant()) {
            return createConstantInt(expr.type, expr.getConstantIntegerValue());
        }

        auto left = emitExprOrEnumTag(expr.getLHS(), nullptr);
        auto right = emitExprOrEnumTag(expr.getRHS(), nullptr);

        if (left->getType()->isPointerType() && left->getType()->getPointee()->equals(right->getType())) {
            left = createLoad(left);
        } else if (right->getType()->isPointerType() && right->getType()->getPointee()->equals(left->getType())) {
            right = createLoad(right);
        }

        if (isWrappingOperator(expr.op) && (left->getType()->isInteger() || left->getType()->isChar())) {
            return emitWrappingArithmetic(getWrappingOrSaturatingBaseOp(expr.op), left, right, expr);
        }
        if (expr.op == Token::LeftShiftSat && (left->getType()->isInteger() || left->getType()->isChar())) {
            return emitSaturatingLeftShift(left, right, expr);
        }
        if (isSaturatingOperator(expr.op) && (left->getType()->isInteger() || left->getType()->isChar())) {
            return emitSaturatingArithmetic(getWrappingOrSaturatingBaseOp(expr.op), left, right, expr);
        }
        // Global initializers can't contain the trap's control flow; like unary
        // minus above, emit the operation plain there.
        if (!emittingGlobalInitializer && (expr.op == Token::Plus || expr.op == Token::Minus || expr.op == Token::Star)
            && options.mode != BuildMode::ReleaseFast && !disablesCheck(disabledChecks, DisabledChecks::Overflow)
            && (left->getType()->isInteger() || left->getType()->isChar())) {
            return emitCheckedArithmetic(expr.op, left, right, expr);
        }
        // Chars order, divide, and shift as unsigned: lower those to uint8
        // like arithmetic does, so backends with signed C chars agree.
        // Equality and bitwise operators are unaffected either way.
        bool needsUnsigned = expr.op == Token::Less || expr.op == Token::LessOrEqual || expr.op == Token::Greater || expr.op == Token::GreaterOrEqual
                          || expr.op == Token::Slash || expr.op == Token::Modulo || expr.op == Token::LeftShift || expr.op == Token::RightShift;
        if (left->getType()->isChar() && needsUnsigned) {
            lowerCharOperands(*this, left, right);
            Value* result = createBinaryOp(expr.op, left, right, &expr);
            return createCastIfNeeded(result, getIRType(expr.type));
        }
        return createBinaryOp(expr.op, left, right, &expr);
    }
}

Value* IRGenerator::emitAssignment(const BinaryExpr& expr) {
    if (expr.getRHS().isUndefinedLiteralExpr()) return nullptr;

    if (auto* member = llvm::dyn_cast<MemberExpr>(&expr.getLHS()); member && member->swizzleIndices.size() > 1) {
        return emitSwizzleAssignment(*member, expr.getRHS());
    }

    // Evaluate LHS address, then RHS (which may borrow from LHS), then destroy
    // the old LHS value before storing. Destroying before RHS would
    // use-after-free when RHS borrows LHS, e.g. `s = join(string(&s), ...)`.
    auto lvalue = emitLvalueExpr(expr.getLHS());
    auto rvalue = emitExprForPassing(expr.getRHS(), lvalue->getType()->getPointee());
    destroyAssignmentLHS(expr.getLHS(), lvalue, expr.lhsIsMoved, expr.lhsIsLive);
    createStore(rvalue, lvalue);
    return nullptr;
}

static bool isBuiltinArrayToSliceConversion(Type sourceType, IRType* targetType) {
    return sourceType.removePointer().isConcreteArray() && targetType->isStruct() && targetType->getName().starts_with("Slice<");
}

static bool isEmptyArrayLiteral(const Expr& expr) {
    auto* array = llvm::dyn_cast<ArrayLiteralExpr>(&expr);
    return array && array->elements.empty();
}

Value* IRGenerator::emitExprForPassing(const Expr& expr, IRType* targetType) {
    if (isEmptyArrayLiteral(expr) && targetType && targetType->isPointerType()) {
        return createConstantNull(targetType);
    }

    if (!targetType) {
        // In variadic calls, arrays decay to pointers to their first element (as in C).
        if (expr.type.isConcreteArray()) {
            auto* value = emitExprAsPointer(expr);
            ASSERT(value->getType()->getPointee()->isArrayType());
            if (expr.type.getArraySize() == 0) {
                return createConstantNull(value->getType()->getPointee()->getElementType()->getPointerTo());
            }
            return createGEP(value, 0);
        }
        return emitExpr(expr);
    }

    // TODO: Handle implicit conversions in a separate function.

    if (isBuiltinArrayToSliceConversion(expr.type, targetType)) {
        ASSERT(expr.type.removePointer().isConcreteArray());
        // Pointer-typed lvalues (e.g. spilled parameters) point at the pointer variable; load the pointer itself.
        auto* value = expr.type.isPointerType() ? emitExpr(expr) : emitExprAsPointer(expr);
        Value* elementPtr;
        if (expr.type.removePointer().getArraySize() == 0) {
            elementPtr = createConstantNull(targetType->getFields()[0].type);
        } else {
            elementPtr = createGEP(value, 0);
        }
        auto* arrayRef = createInsertValue(createUndefined(targetType), elementPtr, 0);
        auto size = createConstantInt(Type::getInt32(), expr.type.removePointer().getArraySize());
        return createInsertValue(arrayRef, size, 1);
    }

    // Handle implicit conversions to type 'T[*]'.
    if (expr.type.removePointer().isConcreteArray() && targetType->isPointerType() && !targetType->getPointee()->isArrayType()) {
        if (expr.type.removePointer().getArraySize() == 0) return createConstantNull(targetType);
        auto* value = expr.type.isPointerType() ? loadThroughStorageAddress(emitLvalueExpr(expr), expr.type) : emitExprAsPointer(expr);
        return createCast(value, targetType);
    }

    // Handle implicit conversions to void pointer, and to base type pointer.
    // Skip this when the target is a pointer to the source type. AutoReference keeps the source type (see Typechecker::convert),
    // so that case is taking the address (handled by the temp alloca below), not a bitcast.
    if (expr.type.isImplementedAsPointer() && targetType->isPointerType() && !getIRType(expr.type)->equals(targetType->getPointee())) {
        return createCastIfNeeded(emitExpr(expr), targetType);
    }

    // TODO: Refactor the following.
    auto* value = emitLvalueExpr(expr);
    if (!value) return nullptr;

    if (targetType->isPointerType() && value->getType()->equals(targetType->getPointee())) {
        return createTempAlloca(value);
    } else if (value->getType()->isPointerType() && !targetType->equals(value->getType())) {
        value = createLoad(value);
        if (value->getType()->isPointerType() && !targetType->equals(value->getType())) {
            value = createLoad(value);
        }
        return value;
    } else {
        return value;
    }
}

void IRGenerator::emitAssert(Value* condition, const Expr* expr, Location location, llvm::StringRef message, const llvm::Twine& name) {
    condition = createIsNull(condition, expr, name + ".condition");
    auto* function = insertBlock->parent;
    auto* failBlock = new BasicBlock((name + ".fail").str(), function);
    auto* successBlock = new BasicBlock((name + ".success").str(), function);
    createCondBr(condition, failBlock, successBlock);
    setInsertPoint(failBlock);
    emitAbortWithMessage(message, location);
    setInsertPoint(successBlock);
}

void IRGenerator::emitAbortWithMessage(llvm::StringRef message, Location location) {
    auto* assertFailDecl = llvm::cast<FunctionDecl>(Module::getStdlibModule()->symbolTable.findOne("assertFail"));
    checkImplicitCalleeIsChecked(*assertFailDecl, "assertFail");
    auto* assertFail = getFunction(*assertFailDecl);
    auto messageAndLocation = llvm::join_items("", message, " at ", llvm::sys::path::filename(location.file), ":", std::to_string(location.line), ":",
                                               std::to_string(location.column), "\n");
    createCall(assertFail, createGlobalStringPtr(messageAndLocation), nullptr);
    createUnreachable();
}

// `emitPayload` runs after the tag store, matching the previous evaluation order
// of case arguments relative to that store. A null payload with `zeroPayload`
// clears an unused associated value.
template<typename EmitPayload>
static Value* materializeEnumCase(IRGenerator& ir, Value* tag, Type enumType, bool zeroPayload, bool registerDestructor, EmitPayload&& emitPayload) {
    auto* enumValue = ir.createEntryBlockAlloca(enumType, "enum");
    ir.createStore(tag, ir.createGEP(enumValue, 0, nullptr, "tag"));
    if (Value* payload = emitPayload()) {
        auto* associatedValuePtr = ir.createCast(ir.createGEP(enumValue, 1, nullptr, "associatedValue"), payload->getType()->getPointerTo());
        ir.createStore(payload, associatedValuePtr);
    } else if (zeroPayload) {
        ir.zeroEnumPayload(enumValue, enumType);
    }
    // A case constructed without payload arguments owns nothing, so there is nothing to destroy.
    if (registerDestructor) ir.registerTempDestructor(enumValue, enumType);
    return enumValue;
}

Value* IRGenerator::emitEnumCase(const EnumCase& enumCase, llvm::ArrayRef<NamedValue> associatedValueElements, bool isMovedFrom) {
    auto enumDecl = enumCase.getEnumDecl();
    auto tag = emitExpr(*enumCase.value);
    if (!enumDecl->hasAssociatedValues()) return tag;

    if (emittingGlobalInitializer) {
        // Only single-payload enums (i.e. Optional) reach here; anything else is rejected in sema.
        // The payload union has one member, so the payload value initializes it directly.
        auto* enumType = getIRType(enumDecl->getType());
        ASSERT(enumType->getFields()[optionalPayloadFieldIndex].type->getFields().size() == 1);
        Value* enumValue = createInsertValue(createUndefined(enumType), tag, optionalTagFieldIndex);
        if (!associatedValueElements.empty()) {
            Value* payload = emitAggregateElements(enumCase.associatedType, associatedValueElements);
            auto* unionType = enumType->getFields()[optionalPayloadFieldIndex].type;
            enumValue = createInsertValue(enumValue, createInsertValue(createUndefined(unionType), payload, 0), optionalPayloadFieldIndex);
        }
        return enumValue;
    }

    // TODO: Could reuse variable alloca instead of always creating a new one here.
    return materializeEnumCase(*this, tag, enumDecl->getType(), associatedValueElements.empty() && enumCase.associatedType,
                               !isMovedFrom && !associatedValueElements.empty(), [&]() -> Value* {
                                   if (associatedValueElements.empty()) return nullptr;
                                   return emitAggregateElements(enumCase.associatedType, associatedValueElements);
                               });
}

Value* IRGenerator::emitEnumCaseCall(const EnumCase& enumCase, const CallExpr& expr) {
    if (expr.argParamIndices.size() != expr.args.size()) return emitEnumCase(enumCase, expr.args, expr.isMovedFrom);
    auto enumDecl = enumCase.getEnumDecl();
    auto tag = emitExpr(*enumCase.value);
    if (!enumDecl->hasAssociatedValues()) return tag;

    return materializeEnumCase(*this, tag, enumDecl->getType(), expr.args.empty() && enumCase.associatedType, !expr.isMovedFrom && !expr.args.empty(),
                               [&]() -> Value* {
                                   if (expr.args.empty()) return nullptr;
                                   llvm::SmallVector<Value*, 8> writtenValues;
                                   for (auto& arg : expr.args)
                                       writtenValues.push_back(emitExpr(*arg.value));
                                   Value* payload = createUndefined(enumCase.associatedType);
                                   for (size_t i = 0; i < expr.args.size(); ++i)
                                       payload = createInsertValue(payload, writtenValues[i], expr.argParamIndices[i]);
                                   return payload;
                               });
}

// A case constructed without payload arguments (e.g. `Ok` in `r == Ok`) leaves the payload
// bytes uninitialized. The value can still be destroyed or copied into owned storage, so zero
// the payload to keep it well-formed.
void IRGenerator::zeroEnumPayload(Value* enumValue, Type enumType) {
    auto* memsetDecl = llvm::cast<FunctionDecl>(Module::getStdlibModule()->symbolTable.findOne("memset"));
    checkImplicitCalleeIsChecked(*memsetDecl, "memset");
    auto* memsetFunction = getFunction(*memsetDecl);
    auto params = memsetDecl->getParams();
    auto* payloadPtr = createCastIfNeeded(createGEP(enumValue, 1, nullptr, "associatedValue"), params[0].type);
    auto* zero = createCastIfNeeded(createConstantInt(Type::getInt32(), 0), params[1].type);
    IRType* payloadType = getIRType(enumType)->getFields()[optionalPayloadFieldIndex].type;
    auto* size = createCastIfNeeded(new SizeofInst{ValueKind::SizeofInst, payloadType, getIRType(Type::getUInt64()), ""}, params[2].type);
    createCall(memsetFunction, {payloadPtr, zero, size}, nullptr);
}

void IRGenerator::emitEnumPayloadDestruction(EnumDecl& enumDecl, Value* self) {
    auto* function = insertBlock->parent;
    auto* end = new BasicBlock("enum.dtor.end", function);
    auto* tag = createLoad(createGEP(self, 0, nullptr, "tag"));
    auto* switchInst = createSwitch(tag, end);

    int index = 0;
    for (auto& enumCase : enumDecl.cases) {
        if (!enumCase.associatedType || !enumCase.associatedType.needsDestruction()) continue;
        auto* block = new BasicBlock("enum.dtor.case." + std::to_string(index++), function);
        setInsertPoint(block);
        auto* payloadPtr = createCast(createGEP(self, 1, nullptr, "associatedValue"), enumCase.associatedType.getPointerTo());
        destroyElementsForAssignment(payloadPtr, enumCase.associatedType);
        createBr(end);
        switchInst->cases.emplace_back(emitExpr(*enumCase.value), block);
    }
    setInsertPoint(end);
}

Value* IRGenerator::emitClosureCallExpr(const CallExpr& expr) {
    auto* closureDecl = llvm::cast<TypeDecl>(llvm::cast<VariableDecl>(expr.calleeDecl)->type.getDecl());
    size_t captureCount = closureDecl->fields.size() - 1;

    Value* closure = emitExpr(*expr.callee);
    Value* function = createExtractValue(closure, 0);
    auto paramTypes = llvm::cast<IRFunctionType>(function->getType()->getPointee())->getParamTypes();
    ASSERT(paramTypes.size() == captureCount + expr.args.size());

    llvm::SmallVector<Value*, 16> args;
    for (size_t i = 0; i < captureCount; ++i) {
        args.push_back(createExtractValue(closure, i + 1));
    }
    for (size_t i = 0; i < expr.args.size(); ++i) {
        args.push_back(emitExprForPassing(*expr.args[i].value, paramTypes[captureCount + i]));
    }
    return maybeRegisterResultTemp(createCall(function, args, &expr), expr);
}

// Whether a value of this type retains no borrow of whatever it was computed from: plain
// scalars and compositions of them, holding neither pointers nor nominal types that could
// hide them. Conservative: anything else is assumed to possibly alias its source, e.g. an
// iterator holding raw pointers into its container.
static bool cannotBorrowReceiver(Type type) {
    if (type.isBuiltinType() && !type.isPointerType()) return true;
    if (type.isOptionalType()) return cannotBorrowReceiver(type.getWrappedType());
    if (type.isFixedArray()) return cannotBorrowReceiver(type.getElementType());
    if (type.isAnonymousStructType()) {
        return llvm::all_of(type.getAnonymousStructElements(), [](auto& element) { return cannotBorrowReceiver(element.type); });
    }
    return false;
}

Value* IRGenerator::emitConstStructCall(const CallExpr& expr, const ConstructorDecl& constructor) {
    // Autogenerated parameters are field-named but reordered (defaulted ones move last),
    // and named arguments reorder further, so map each parameter to its field index once.
    auto* typeDecl = constructor.getTypeDecl();
    auto params = constructor.getParams();
    llvm::SmallVector<int, 16> fieldForParam;
    for (auto& param : params) {
        auto field = llvm::find_if(typeDecl->fields, [&](const FieldDecl& field) { return field.getName() == param.getName(); });
        ASSERT(field != typeDecl->fields.end());
        fieldForParam.push_back(int(field - typeDecl->fields.begin()));
    }
    Value* aggregate = createUndefined(expr.type);
    for (size_t i = 0; i < expr.args.size(); ++i) {
        int paramIndex = expr.paramIndexForArg(i);
        ASSERT(paramIndex >= 0 && size_t(paramIndex) < fieldForParam.size());
        aggregate = createInsertValue(aggregate, emitExpr(*expr.args[i].value), fieldForParam[size_t(paramIndex)]);
    }
    return aggregate;
}

// Whether a non-foreign `init` in a constructor delegates to `this`: bare
// and `this` calls always do; a qualified type only when it names the
// current type or an interface, whose initializers run on `this`.
static bool qualifiedInitDelegates(const CallExpr& expr, const Decl* currentDecl) {
    auto* receiver = llvm::dyn_cast_or_null<VarExpr>(expr.getReceiver());
    auto* target = receiver ? llvm::dyn_cast_or_null<TypeDecl>(receiver->decl) : nullptr;
    if (!target) return true;
    if (target->isInterface()) return true;
    return target == llvm::cast<ConstructorDecl>(currentDecl)->getTypeDecl();
}

Value* IRGenerator::emitCallExpr(const CallExpr& expr, AllocaInst* thisAllocaForInit) {
    if (emittingGlobalInitializer) {
        // An autogenerated constructor just copies arguments into fields, so with constant
        // arguments the call folds to a constant aggregate. Sema only lets these calls into
        // global initializers for copyable structs; anything else falls through and fails
        // loudly below, as before.
        if (auto* constructor = llvm::dyn_cast_or_null<ConstructorDecl>(expr.calleeDecl)) {
            if (constructor->isAutogenerated && expr.type.isImplicitlyCopyable()) return emitConstStructCall(expr, *constructor);
        }
    }
    if (auto* variableDecl = llvm::dyn_cast_or_null<VariableDecl>(expr.calleeDecl)) {
        if (variableDecl->type.isClosureType()) {
            return emitClosureCallExpr(expr);
        }
    }

    if (expr.isBuiltinConversion()) {
        return createCastIfNeeded(emitExpr(*expr.args.front().value), expr.type);
    }

    if (expr.isBuiltinCast()) {
        return emitBuiltinCast(expr);
    }

    if (expr.getFunctionName() == "assert") {
        // --release strips asserts, except in test functions, which keep them in all modes. Stripped asserts don't evaluate the condition.
        auto* enclosingFunction = llvm::dyn_cast_or_null<FunctionDecl>(currentDecl);
        if (!assertsEnabled(options.mode, enclosingFunction && enclosingFunction->isTest)) return nullptr;
        const Expr* condition = expr.args.front().value;
        const auto* messageExpr = llvm::cast<StringLiteralExpr>(expr.args[1].value);
        if (expr.argParamIndices.size() == expr.args.size()) {
            for (size_t i = 0; i < expr.args.size(); ++i) {
                if (expr.argParamIndices[i] == 0) condition = expr.args[i].value;
                if (expr.argParamIndices[i] == 1) messageExpr = llvm::cast<StringLiteralExpr>(expr.args[i].value);
            }
        }
        emitAssert(emitExpr(*condition), &expr, expr.callee->location, messageExpr->value);
        return nullptr;
    }

    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.calleeDecl)) {
        return emitEnumCaseCall(*enumCase, expr);
    }

    if (expr.getReceiver() && expr.receiverType.removeOptional().isArrayType() && !expr.receiverType.removeOptional().isFixedArray()
        && expr.getFunctionName() == "data") {
        return emitExpr(*expr.getReceiver());
    }

    // Array.size() is known from the type; emit the constant instead of a call
    // that only folds away after inlining. Sizeof-computed sizes materialize
    // through sizeof (sema never instantiates the method for those, so there
    // is no callee); other symbolic sizes still call the method.
    if (expr.getFunctionName() == "size" && expr.getReceiver()) {
        Type receiverType = expr.receiverType.removeOptional().removePointer();
        if (receiverType.hasSizeofArraySize()) {
            return createCastIfNeeded(createSizeof(receiverType.getSizeofArrayOperand()), getIRType(Type::getInt32()));
        }
        if (auto* functionDecl = llvm::dyn_cast_or_null<FunctionDecl>(expr.calleeDecl)) {
            if (functionDecl->getTypeDecl() && functionDecl->getTypeDecl()->getName() == "Array" && receiverType.isConcreteArray()) {
                return createConstantInt(Type::getInt32(), receiverType.getArraySize());
            }
        }
    }

    if (expr.isMoveInit()) {
        auto* receiverPtr = emitExprAsPointer(*expr.getReceiver());
        // Through-pointer init (e.g. ptrVar.init(x)) loads the destination out
        // of its slot; exact-type init already holds the slot address, and
        // loading would dereference uninitialized storage for pointer-typed slots.
        auto* receiverValue =
            expr.args[0].value->type == expr.getReceiver()->type ? receiverPtr : loadThroughStorageAddress(receiverPtr, expr.getReceiver()->type);
        auto* argumentValue = emitExpr(*expr.args[0].value);
        createStore(argumentValue, receiverValue);
        return nullptr;
    }

    // Explicit deinit on a fixed array or anonymous struct destroys owning
    // elements structurally, like implicit destruction; leniency left no
    // callee since they declare no destructor. Through a borrow (e.g. a
    // for-loop element) the value is already the element address.
    if (!expr.calleeDecl && expr.getFunctionName() == "deinit" && expr.getReceiver()) {
        const Expr* receiver = expr.getReceiver();
        Type aggregateType = receiver->type.removeReference();
        if ((aggregateType.isFixedArray() || aggregateType.isAnonymousStructType()) && aggregateType.needsDestruction()) {
            Value* base = receiver->type.isReferenceType() ? emitExpr(*receiver) : emitLvalueExpr(*receiver);
            destroyElementsForAssignment(base, aggregateType);
            return nullptr;
        }
    }

    Value* calleeValue = getFunctionForCall(expr);

    if (!calleeValue) {
        return nullptr;
    }

    std::vector<IRType*> params;

    if (auto* function = llvm::dyn_cast<Function>(calleeValue)) {
        params = map(function->params, [](const Parameter& p) { return p.type; });
    } else {
        if (!calleeValue->getType()->getPointee()->isFunctionType()) {
            calleeValue = createLoad(calleeValue);
        }
        params = calleeValue->getType()->getPointee()->getParamTypes();
    }

    auto param = params.begin();
    llvm::SmallVector<Value*, 16> args;
    auto* calleeDecl = expr.calleeDecl;

    if (calleeDecl->isMethodDecl()) {
        if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(calleeDecl)) {
            if (thisAllocaForInit) {
                args.emplace_back(thisAllocaForInit);
            } else if (expr.isForeignInit()) {
                // Reinitializing another instance runs its constructor on its
                // own storage; only bare, this, or qualified init delegates.
                llvm::SaveAndRestore saveEmittingReceiver(emittingReceiver, true);
                args.emplace_back(emitExprForPassing(*expr.getReceiver(), *param));
            } else if (currentDecl->isConstructorDecl() && expr.getFunctionName() == "init" && qualifiedInitDelegates(expr, currentDecl)) {
                args.emplace_back(getThis(*param));
            } else {
                auto* tempAlloca = createEntryBlockAlloca(constructorDecl->getTypeDecl()->getType());
                // A moved result is owned by its consumer; anything else dies at the
                // end of the enclosing statement.
                if (!expr.isMovedFrom) {
                    registerTempDestructor(tempAlloca, constructorDecl->getTypeDecl()->getType());
                }
                args.emplace_back(tempAlloca);
            }
        } else if (expr.getReceiver()) {
            // The declared return type, not expr.type: the latter erases operator[]
            // borrows, which would strand subscript results of temporaries.
            auto* callee = llvm::cast<FunctionDecl>(calleeDecl);
            llvm::SaveAndRestore saveEmittingReceiver(emittingReceiver, !cannotBorrowReceiver(callee->getReturnType()));
            args.emplace_back(emitExprForPassing(*expr.getReceiver(), *param));
        } else {
            args.emplace_back(getThis());
        }
        ++param;
    }

    std::vector<IRType*> argParamTypes(param, params.end());
    // Same-named externs share one object when merely ABI-compatible, so their
    // calls may carry a differently spelled but ABI-equal argument type.
    bool abiOnlyArgs = llvm::dyn_cast_or_null<FunctionDecl>(calleeDecl) && llvm::cast<FunctionDecl>(calleeDecl)->isExtern();
    if (expr.argParamIndices.size() == expr.args.size()) {
        llvm::SmallVector<Value*, 16> writtenValues;
        for (size_t i = 0; i < expr.args.size(); ++i) {
            int paramIndex = expr.argParamIndices[i];
            IRType* paramType = (paramIndex != -1 && size_t(paramIndex) < argParamTypes.size()) ? argParamTypes[size_t(paramIndex)] : nullptr;
            auto* argValue = emitExprForPassing(*expr.args[i].value, paramType);
            ASSERT(!paramType || (abiOnlyArgs ? argValue->getType()->abiEquals(paramType) : argValue->getType()->equals(paramType)));
            writtenValues.push_back(argValue);
        }
        llvm::SmallVector<Value*, 16> orderedValues(argParamTypes.size(), nullptr);
        llvm::SmallVector<Value*, 4> extras;
        for (size_t i = 0; i < expr.args.size(); ++i) {
            int paramIndex = expr.argParamIndices[i];
            if (paramIndex == -1) {
                extras.push_back(writtenValues[i]);
            } else {
                orderedValues[size_t(paramIndex)] = writtenValues[i];
            }
        }
        for (auto* value : orderedValues)
            if (value) args.push_back(value);
        for (auto* value : extras)
            args.push_back(value);
    } else {
        for (const auto& arg : expr.args) {
            auto paramType = param != params.end() ? *param++ : nullptr;
            auto* argValue = emitExprForPassing(*arg.value, paramType);
            ASSERT(!paramType || argValue->getType()->equals(paramType));
            args.push_back(argValue);
        }
    }

    if (calleeDecl->isConstructorDecl()) {
        createCall(calleeValue, args, &expr);
        return args[0];
    } else {
        return maybeRegisterResultTemp(createCall(calleeValue, args, &expr), expr);
    }
}

Value* IRGenerator::emitBuiltinCast(const CallExpr& expr) {
    auto* value = emitExpr(*expr.args.front().value);
    auto* targetType = getIRType(expr.genericArgs.front().getType());
    if (value->getType()->equals(targetType)) return value;
    return createCast(value, targetType);
}

Value* IRGenerator::emitSizeofExpr(const SizeofExpr& expr) {
    // The cx type is int by default and adopts a peer numeric type, so the
    // size value is not fixed as uint64.
    return new SizeofInst{ValueKind::SizeofInst, getIRType(expr.operandType), getIRType(expr.type), ""};
}

Value* IRGenerator::emitMemberAccess(Value* baseValue, const FieldDecl* field, const MemberExpr* expr) {
    auto baseTypeDecl = field->getParentDecl();

    if (baseValue->getType()->isPointerType()) {
        if (baseValue->getType()->getPointee()->isPointerType()) {
            baseValue = createLoad(baseValue);
        }

        if (baseTypeDecl->isUnion()) {
            return createCast(baseValue, field->type.getPointerTo(), field->getName());
        } else {
            return createGEP(baseValue, baseTypeDecl->getFieldIndex(field), expr, field->getName());
        }
    } else {
        if (baseTypeDecl->isUnion()) {
            // Union members overlap, so a member of a union value cannot be
            // projected; spill to memory and reload as the member type instead.
            return createLoad(createCast(createTempAlloca(baseValue), field->type.getPointerTo(), field->getName()));
        }
        return createExtractValue(baseValue, baseTypeDecl->getFieldIndex(field), field->getName());
    }
}

Value* IRGenerator::emitMemberExpr(const MemberExpr& expr) {
    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.decl)) {
        return emitEnumCase(*enumCase, {}, expr.isMovedFrom);
    }

    if (auto* varDecl = llvm::dyn_cast_or_null<VarDecl>(expr.decl)) {
        Value* value = getValue(varDecl);
        if (emittingGlobalInitializer) {
            // Like emitVarExpr: a global initializer can't load from another
            // global; splice in the referenced initializer instead.
            if (varDecl->isGlobal() && varDecl->isConst) {
                if (auto* global = llvm::dyn_cast<GlobalVariable>(value)) {
                    if (global->value) value = global->value;
                }
            }
        }
        return value;
    }

    if (expr.base->type.removePointer().isAnonymousStructType()) {
        return emitAnonymousStructElementAccess(expr);
    }

    // Array swizzles (`vec.x`, `vec.xy`, `vec.rgba`, etc.). Single-char
    // returns the element address so it works as an lvalue; multi-char
    // builds a new array (direct assignment stores it back element-wise).
    if (!expr.swizzleIndices.empty()) {
        if (expr.swizzleIndices.size() == 1) {
            auto* zero = createConstantInt(Type::getInt32(), 0);
            auto* idx = createConstantInt(Type::getInt32(), expr.swizzleIndices[0]);
            return createGEP(emitArrayBasePtr(*expr.base), {zero, idx});
        }
        auto* baseValue = emitExpr(*expr.base);
        Value* basePtr = baseValue->getType()->isPointerType() ? baseValue : createTempAlloca(baseValue);
        auto emitSwizzleElement = [&](int index) -> Value* {
            auto* zero = createConstantInt(Type::getInt32(), 0);
            auto* idx = createConstantInt(Type::getInt32(), index);
            auto* gep = createGEP(basePtr, {zero, idx});
            return createLoad(gep);
        };
        auto* arrayIRType = getIRType(expr.type);
        Value* result = createUndefined(arrayIRType);
        for (size_t i = 0; i < expr.swizzleIndices.size(); ++i) {
            Value* elem = emitSwizzleElement(expr.swizzleIndices[i]);
            result = createInsertValue(result, elem, static_cast<int>(i));
        }
        return result;
    }

    return emitMemberAccess(emitLvalueExpr(*expr.base), llvm::cast<FieldDecl>(expr.decl), &expr);
}

Value* IRGenerator::emitAnonymousStructElementAccess(const MemberExpr& expr) {
    unsigned index = 0;
    for (auto& element : expr.base->type.removePointer().getAnonymousStructElements()) {
        if (element.name == expr.member) break;
        ++index;
    }

    auto* baseValue = emitLvalueExpr(*expr.base);
    if (baseValue->getType()->isPointerType() && baseValue->getType()->getPointee()->isPointerType()) {
        baseValue = createLoad(baseValue);
    }

    if (baseValue->getType()->isPointerType()) {
        return createGEP(baseValue, index, nullptr, expr.member);
    } else {
        return createExtractValue(baseValue, index, expr.member);
    }
}

Value* IRGenerator::emitArrayBasePtr(const Expr& base) {
    auto* value = emitLvalueExpr(base);

    if (value->getType()->isPointerType() && value->getType()->getPointee()->isPointerType() && value->getType()->getPointee()->equals(getIRType(base.type))) {
        value = createLoad(value);
    }
    if (!value->getType()->isPointerType()) value = createTempAlloca(value);
    return value;
}

Value* IRGenerator::emitSwizzleAssignment(const MemberExpr& lhs, const Expr& rhs) {
    auto* basePtr = emitArrayBasePtr(*lhs.base);
    auto* rhsValue = emitExpr(rhs);
    Value* rhsPtr = rhsValue->getType()->isPointerType() ? rhsValue : createTempAlloca(rhsValue);
    auto* zero = createConstantInt(Type::getInt32(), 0);
    for (size_t i = 0; i < lhs.swizzleIndices.size(); ++i) {
        auto* src = createGEP(rhsPtr, {zero, createConstantInt(Type::getInt32(), static_cast<int>(i))});
        auto* dst = createGEP(basePtr, {zero, createConstantInt(Type::getInt32(), lhs.swizzleIndices[i])});
        createStore(createLoad(src), dst);
    }
    return nullptr;
}

Value* IRGenerator::emitIndexedAccess(const Expr& base, const Expr& index) {
    auto* value = emitArrayBasePtr(base);
    auto* indexValue = emitExpr(index);

    Value* gep;
    if (base.type.removeOptional().isArrayPointer()) {
        gep = createGEP(value, {indexValue});
    } else {
        Type arrayType = base.type.removeOptional().removePointer();
        if (arrayType.isConcreteArray()) {
            auto* enclosingFunction = llvm::dyn_cast_or_null<FunctionDecl>(currentDecl);
            if (assertsEnabled(options.mode, enclosingFunction && enclosingFunction->isTest) && !disablesCheck(disabledChecks, DisabledChecks::Bounds)) {
                // A single unsigned comparison catches negative indices too: they wrap to huge values.
                auto* wideIndex = createCastIfNeeded(indexValue, Type::getUInt64());
                auto* size = createConstantInt(Type::getUInt64(), arrayType.getArraySize());
                emitAssert(createBinaryOp(Token::Less, wideIndex, size, &index), &index, index.location, "index out of bounds", "bounds");
            }
        }
        gep = createGEP(value, {createConstantInt(Type::getInt32(), 0), indexValue});
    }
    if (auto* call = llvm::dyn_cast<CallExpr>(&base); call && call->isMethodCall() && call->getFunctionName() == "data") {
        llvm::cast<GEPInst>(gep)->expr = &base;
    }
    return gep;
}

Value* IRGenerator::emitIndexExpr(const IndexExpr& expr) {
    if (!expr.getBase()->type.removeOptional().removePointer().isArrayType()) {
        return emitCallExpr(expr);
    }

    return emitIndexedAccess(*expr.getBase(), *expr.getIndex());
}

Value* IRGenerator::emitIndexAssignmentExpr(const IndexAssignmentExpr& expr) {
    if (!expr.getBase()->type.removeOptional().removePointer().isArrayType()) {
        emitCallExpr(expr);
        return nullptr;
    }

    auto gep = emitIndexedAccess(*expr.getBase(), *expr.getIndex());
    auto* value = emitExpr(*expr.getValue());
    createStore(value, gep);
    return nullptr;
}

Value* IRGenerator::emitUnwrapExpr(const UnwrapExpr& expr) {
    if (expr.calleeDecl) {
        return emitCallExpr(expr);
    }
    // Redundant unwraps of narrowed values warn in sema; elide them here.
    if (!expr.getReceiver()->type.isOptionalType()) {
        return emitExpr(*expr.getReceiver());
    }
    // The result borrows the operand, so operand temporaries die at scope exit like receivers.
    llvm::SaveAndRestore saveEmittingReceiver(emittingReceiver, true);
    return emitOptionalUnwrap(*expr.getReceiver(), expr, "assert");
}

Value* IRGenerator::emitLambdaExpr(const LambdaExpr& expr) {
    auto functionDecl = expr.functionDecl;

    auto currentFunctionBackup = currentFunction;
    auto insertBlockBackup = insertBlock;
    auto scopesBackup = std::move(scopes);
    auto tempScopesBackup = std::move(tempScopes);
    auto tempGuardBackup = tempGuard;
    tempGuard = nullptr;
    auto emittingReceiverBackup = emittingReceiver;
    emittingReceiver = false;

    emitDecl(*functionDecl);

    currentFunction = currentFunctionBackup;
    scopes = std::move(scopesBackup);
    tempScopes = std::move(tempScopesBackup);
    tempGuard = tempGuardBackup;
    emittingReceiver = emittingReceiverBackup;
    if (insertBlockBackup) setInsertPoint(insertBlockBackup);

    if (functionDecl->captures.empty()) {
        VarExpr varExpr(functionDecl->getName(), functionDecl->getLocation());
        varExpr.decl = functionDecl;
        varExpr.type = expr.type;
        return emitVarExpr(varExpr);
    }

    Value* closure = createUndefined(getIRType(expr.type));
    closure = createInsertValue(closure, getFunction(*functionDecl), 0);
    int index = 1;
    for (auto* captured : functionDecl->captures) {
        VarExpr captureExpr(std::string(captured->getName()), captured->getLocation());
        captureExpr.decl = captured;
        // emitExpr loads iff the pointee matches the expression type, so typing `this`
        // captures as pointers stores the pointer instead of a copy of the object.
        captureExpr.type = captured->getCaptureType();
        captureExpr.assignableType = captured->getCaptureType();
        closure = createInsertValue(closure, emitExpr(captureExpr), index++);
    }
    return maybeRegisterResultTemp(closure, expr);
}

Value* IRGenerator::emitIfExpr(const IfExpr& expr) {
    // Integer ternaries fold in emitPlainExpr; fold boolean ones here so global initializers stay branch-free.
    if (expr.hasType() && expr.type.isBool() && expr.isFoldableBoolConstant()) {
        return createConstantBool(expr.getConstantBoolValue());
    }

    auto* condition = emitBoolConvertibleOperand(*expr.condition);
    auto* function = currentFunction;
    auto* thenBlock = new BasicBlock("if.then", function);
    auto* elseBlock = new BasicBlock("if.else");
    auto* endIfBlock = new BasicBlock("if.end");
    // Only the taken arm runs, so each arm gets a guard flag: its temporaries
    // are destroyed at statement end only if the arm executed.
    auto* thenGuard = createTempGuard();
    auto* elseGuard = createTempGuard();
    auto* outerGuard = tempGuard;
    createCondBr(condition, thenBlock, elseBlock);

    // A diverging arm contributes no value to the join; like switch arms, it
    // terminates its block instead of branching out.
    bool thenDiverges = expr.thenExpr->type.isNeverType();
    bool elseDiverges = expr.elseExpr->type.isNeverType();

    // A reference-typed join binds the selected arm's address (like C++ 'c ? x : y'),
    // so arms emit as pointers for passing instead of loaded values.
    IRType* joinType = expr.type.isReferenceType() ? getIRType(expr.type) : nullptr;
    auto emitArm = [&](const Expr& arm) { return joinType ? emitExprForPassing(arm, joinType) : emitExpr(arm); };

    setInsertPoint(thenBlock);
    createStore(createConstantBool(true), thenGuard);
    tempGuard = thenGuard;
    auto* thenValue = emitArm(*expr.thenExpr);
    // Void branches produce no value to join; like void calls, the result is only usable in discard positions.
    bool isVoid = !thenValue || thenValue->getType()->isVoid();
    if (thenDiverges) {
        createUnreachable();
    } else {
        createBr(endIfBlock, isVoid ? nullptr : thenValue);
    }

    setInsertPoint(elseBlock);
    createStore(createConstantBool(true), elseGuard);
    tempGuard = elseGuard;
    auto* elseValue = emitArm(*expr.elseExpr);
    bool elseIsVoid = !elseValue || elseValue->getType()->isVoid();
    if (elseDiverges) {
        createUnreachable();
    } else {
        createBr(endIfBlock, elseIsVoid ? nullptr : elseValue);
    }
    tempGuard = outerGuard;

    setInsertPoint(endIfBlock);
    Value* joinValue = thenDiverges ? elseValue : thenValue;
    if (!joinValue || joinValue->getType()->isVoid()) return joinValue;
    endIfBlock->parameter = new Parameter{ValueKind::Parameter, joinValue->getType(), "if.result"};
    return endIfBlock->parameter;
}

Value* IRGenerator::emitSwitchExpr(const SwitchExpr& expr) {
    Value* enumValue = nullptr;
    Value* condition = emitExprOrEnumTag(*expr.condition, &enumValue);

    // Like emitIfExpr, leave blocks unparented so setInsertPoint adopts them in emission
    // order; nested switch expressions then lay out before the outer end block, which the
    // block-parameter lowering requires.
    auto* insertBlockBackup = insertBlock;
    auto caseIndex = 0;

    auto cases = map(expr.arms, [&](const SwitchExprArm& arm) {
        auto* value = emitExprOrEnumTag(*arm.value, nullptr);
        auto* block = new BasicBlock("switch.case." + std::to_string(caseIndex++));
        return std::make_pair(value, block);
    });

    setInsertPoint(insertBlockBackup);
    auto* defaultBlock = new BasicBlock("switch.default");
    auto* end = new BasicBlock("switch.end");
    // Only the taken arm runs; see emitIfExpr.
    llvm::SmallVector<Value*, 8> armGuards;
    for (size_t i = 0; i < expr.arms.size(); ++i) {
        armGuards.push_back(createTempGuard());
    }
    auto* defaultGuard = createTempGuard();
    auto* outerGuard = tempGuard;
    auto* switchInst = createSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (size_t armIndex = 0; armIndex < expr.arms.size(); ++armIndex) {
        auto& arm = expr.arms[armIndex];
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        setInsertPoint(block);
        createStore(createConstantBool(true), armGuards[armIndex]);
        tempGuard = armGuards[armIndex];

        if (auto* associatedValue = arm.associatedValue) {
            bindBorrowedEnumPayload(enumValue, associatedValue);
        }

        // Never arms diverge, so they terminate the block instead of branching out with a value.
        if (arm.expr->type.isNeverType()) {
            emitExpr(*arm.expr);
            createUnreachable();
        } else {
            createBr(end, emitExpr(*arm.expr));
        }
        switchInst->cases.emplace_back(value, block);
        ++casesIterator;
    }

    setInsertPoint(defaultBlock);
    if (expr.defaultExpr) {
        createStore(createConstantBool(true), defaultGuard);
        tempGuard = defaultGuard;
        if (expr.defaultExpr->type.isNeverType()) {
            emitExpr(*expr.defaultExpr);
            createUnreachable();
        } else {
            createBr(end, emitExpr(*expr.defaultExpr));
        }
    } else {
        llvm::SmallVector<Expr*, 8> caseValues;
        for (auto& arm : expr.arms) {
            caseValues.push_back(arm.value);
        }
        // The typechecker guarantees an exhaustive enum switch here, so the check always applies.
        bool checkEmitted = emitEnumSwitchCheck(*expr.condition, caseValues, *switchInst, end);
        ASSERT(checkEmitted);
    }
    tempGuard = outerGuard;

    setInsertPoint(end);
    end->parameter = new Parameter{ValueKind::Parameter, getIRType(expr.type), "switch.result"};
    return end->parameter;
}

Value* IRGenerator::emitImplicitCastExpr(const ImplicitCastExpr& expr) {
    switch (expr.castKind) {
    case ImplicitCastExpr::OptionalWrap:
        if (expr.type.getWrappedType().isImplementedAsPointer()) {
            if (expr.type.getWrappedType().isArrayPointer() && (expr.operand->type.removePointer().isConcreteArray() || isEmptyArrayLiteral(*expr.operand))) {
                return emitExprForPassing(*expr.operand, getIRType(expr.type.getWrappedType()));
            }
            if (expr.type.getWrappedType().isReferenceType() && !expr.operand->type.isReferenceType()) {
                // Borrowing through the wrap: the operand is a value, so take the address-preserving
                // path and materialize temporaries the way argument passing does. (An operand that is
                // already a borrow takes the normal path below.)
                Value* value = emitLvalueExpr(*expr.operand);
                if (!value->getType()->isPointerType()) value = createTempAlloca(value);
                return value;
            }
            return emitExpr(*expr.operand);
        } else {
            return emitOptionalConstruction(expr.operand->type, expr.operand, expr.isMovedFrom);
        }
    case ImplicitCastExpr::OptionalUnwrap:
        return emitOptionalUnwrap(*expr.operand, expr, "__implicit_unwrap");
    case ImplicitCastExpr::OptionalUnwrapPointer: {
        // Narrowing proved the target non-null, so project the payload without asserting.
        // Pointer-implemented payloads share the optional's address; others live at the payload field.
        Value* pointer = emitExpr(*expr.operand);
        Type wrapped = expr.operand->type.getPointee().getWrappedType();
        if (wrapped.isImplementedAsPointer()) return createCastIfNeeded(pointer, getIRType(expr.type));
        return emitOptionalPayloadPtr(pointer, wrapped);
    }
    case ImplicitCastExpr::AutoReference:
        return emitPlainExpr(*expr.operand);
    case ImplicitCastExpr::AutoDereference:
        return createLoad(emitPlainExpr(*expr.operand));
    case ImplicitCastExpr::NumericWiden:
        return createCastIfNeeded(emitExpr(*expr.operand), expr.type);
    case ImplicitCastExpr::UserConversion:
        return emitUserConversion(expr);
    }

    llvm_unreachable("all implicit cast kinds handled");
}

Value* IRGenerator::emitUserConversion(const ImplicitCastExpr& expr, AllocaInst* thisAllocaForInit) {
    auto* conversion = llvm::cast<FunctionDecl>(expr.conversionDecl);
    Function* callee = getFunction(*conversion);
    if (auto* ctor = llvm::dyn_cast<ConstructorDecl>(conversion)) {
        // Mirror constructor calls: the callee initializes the local or a fresh temporary.
        auto* thisAlloca = thisAllocaForInit ? thisAllocaForInit : createEntryBlockAlloca(ctor->getTypeDecl()->getType());
        if (!thisAllocaForInit && !expr.isMovedFrom) {
            registerTempDestructor(thisAlloca, ctor->getTypeDecl()->getType());
        }
        llvm::SmallVector<Value*, 2> args{thisAlloca, emitExprForPassing(*expr.operand, callee->params[1].type)};
        createCall(callee, args, &expr);
        return thisAlloca;
    }
    // Mirror method calls: the operand becomes the receiver.
    llvm::SmallVector<Value*, 1> args;
    {
        llvm::SaveAndRestore saveEmittingReceiver(emittingReceiver, true);
        args.push_back(emitExprForPassing(*expr.operand, callee->params[0].type));
    }
    return maybeRegisterResultTemp(createCall(callee, args, &expr), expr);
}

Value* IRGenerator::emitPlainExpr(const Expr& expr) {
    llvm::SaveAndRestore saveChecks(disabledChecks, disabledChecks | expr.disabledChecks);
    // Cyclic constants can leave expressions untyped; don't fold those.
    if (expr.hasType() && expr.type.isInteger() && expr.isFoldableIntConstant()) {
        return createConstantInt(expr.type, expr.getConstantIntegerValue());
    }
    if (expr.hasType() && expr.type.isBool() && expr.isFoldableBoolConstant()) {
        return createConstantBool(expr.getConstantBoolValue());
    }

    switch (expr.kind) {
    case ExprKind::VarExpr:
        return emitVarExpr(llvm::cast<VarExpr>(expr));
    case ExprKind::StringLiteralExpr:
        return emitStringLiteralExpr(llvm::cast<StringLiteralExpr>(expr));
    case ExprKind::CharacterLiteralExpr:
        return emitCharacterLiteralExpr(llvm::cast<CharacterLiteralExpr>(expr));
    case ExprKind::IntLiteralExpr:
        return emitIntLiteralExpr(llvm::cast<IntLiteralExpr>(expr));
    case ExprKind::FloatLiteralExpr:
        return emitFloatLiteralExpr(llvm::cast<FloatLiteralExpr>(expr));
    case ExprKind::BoolLiteralExpr:
        return emitBoolLiteralExpr(llvm::cast<BoolLiteralExpr>(expr));
    case ExprKind::NullLiteralExpr:
        return emitNullLiteralExpr(llvm::cast<NullLiteralExpr>(expr));
    case ExprKind::UndefinedLiteralExpr:
        return emitUndefinedLiteralExpr(llvm::cast<UndefinedLiteralExpr>(expr));
    case ExprKind::ArrayLiteralExpr:
        return emitArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr));
    case ExprKind::AnonymousStructExpr:
        return emitAnonymousStructExpr(llvm::cast<AnonymousStructExpr>(expr));
    case ExprKind::UnaryExpr:
        return emitUnaryExpr(llvm::cast<UnaryExpr>(expr));
    case ExprKind::BinaryExpr:
        return emitBinaryExpr(llvm::cast<BinaryExpr>(expr));
    case ExprKind::CallExpr:
        return emitCallExpr(llvm::cast<CallExpr>(expr));
    case ExprKind::SizeofExpr:
        return emitSizeofExpr(llvm::cast<SizeofExpr>(expr));
    case ExprKind::MemberExpr:
        return emitMemberExpr(llvm::cast<MemberExpr>(expr));
    case ExprKind::IndexExpr:
        return emitIndexExpr(llvm::cast<IndexExpr>(expr));
    case ExprKind::IndexAssignmentExpr:
        return emitIndexAssignmentExpr(llvm::cast<IndexAssignmentExpr>(expr));
    case ExprKind::UnwrapExpr:
        return emitUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
    case ExprKind::LambdaExpr:
        return emitLambdaExpr(llvm::cast<LambdaExpr>(expr));
    case ExprKind::IfExpr:
        return emitIfExpr(llvm::cast<IfExpr>(expr));
    case ExprKind::SwitchExpr:
        return emitSwitchExpr(llvm::cast<SwitchExpr>(expr));
    case ExprKind::ImplicitCastExpr:
        return emitImplicitCastExpr(llvm::cast<ImplicitCastExpr>(expr));
    case ExprKind::VarDeclExpr:
        return emitVarDecl(*llvm::cast<VarDeclExpr>(expr).varDecl);
    }
    llvm_unreachable("all cases handled");
}

Value* IRGenerator::emitExpr(const Expr& expr) {
    // No mask push: emitLvalueExpr below applies this expression's mask, and
    // the trailing load emits no checks.
    auto* value = emitLvalueExpr(expr);

    if (value && value->getType()->isPointerType() && value->getType()->getPointee()->equals(getIRType(expr.type))) {
        return createLoad(value);
    }

    return value;
}

Value* IRGenerator::emitExprAsPointer(const Expr& expr) {
    auto* value = emitLvalueExpr(expr);
    if (!value->getType()->isPointerType()) {
        value = createTempAlloca(value);
    }
    return value;
}

Value* IRGenerator::loadThroughStorageAddress(Value* value, Type exprType) {
    // Spilled params and locals add an indirection: when value points to the expr's own
    // pointer type instead of into the data, load once to get the data pointer.
    if (value->getType()->isPointerType() && value->getType()->getPointee()->isPointerType() && value->getType()->getPointee()->equals(getIRType(exprType))) {
        return createLoad(value);
    }
    return value;
}

Value* IRGenerator::emitExprOrEnumTag(const Expr& expr, Value** enumValue) {
    if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr)) {
        if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(memberExpr->decl)) {
            return emitExpr(*enumCase->value);
        }
    }

    if (auto* enumDecl = llvm::dyn_cast_or_null<EnumDecl>(expr.type.getDecl())) {
        // Pointer-implemented optionals are bare pointers with no tag; compare them directly.
        if (enumDecl->hasAssociatedValues() && !expr.type.isImplementedAsPointer()) {
            auto* value = emitLvalueExpr(expr);
            if (!value->getType()->isPointerType()) {
                // Temporaries have no address; spill to a temp so the tag load and associated-value access work.
                value = createTempAlloca(value);
            }
            if (enumValue) *enumValue = value;
            return createLoad(createGEP(value, 0, nullptr, value->getName() + ".tag"));
        }
    }

    return emitExpr(expr);
}

Value* IRGenerator::emitLvalueExpr(const Expr& expr) {
    llvm::SaveAndRestore saveChecks(disabledChecks, disabledChecks | expr.disabledChecks);
    auto value = emitPlainExpr(expr);

    // Handle optionals that have been implicitly unwrapped due to data-flow analysis.
    // Pointer-implemented optionals need no access adjustment: the narrowed type is a compile-time view of the same value.
    if (expr.hasAssignableType() && expr.assignableType.isOptionalType() && !expr.assignableType.getWrappedType().isImplementedAsPointer()
        && expr.type == expr.assignableType.getWrappedType()) {
        // Temporaries are SSA values, not memory; spill to a temp so the payload access works.
        if (!value->getType()->isPointerType()) value = createTempAlloca(value);
        return emitOptionalPayloadPtr(value, expr.assignableType.getWrappedType());
    }

    // Handle enums narrowed to a case payload due to data-flow analysis: project the payload slot.
    if (expr.hasAssignableType() && EnumDecl::isPayloadView(expr.assignableType, expr.type)) {
        // Temporaries are SSA values, not memory; spill to a temp so the payload access works.
        if (!value->getType()->isPointerType()) value = createTempAlloca(value);
        return createCast(createGEP(value, 1, nullptr, "associatedValue"), expr.type.getPointerTo());
    }

    if (value && expr.hasType()) {
        auto type = getIRType(expr.type);

        // TODO: Why only FP and integers are cast here?
        if (!type->equals(value->getType()) && (value->getType()->isFloatingPoint() || value->getType()->isInteger())) {
            return createCast(value, type);
        }
    }

    return value;
}

void IRGenerator::setInsertPoint(BasicBlock* block) {
    if (!block->parent) {
        currentFunction->body.push_back(block);
        block->parent = currentFunction;
    }

    insertBlock = block;
}
