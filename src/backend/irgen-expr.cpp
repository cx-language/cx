#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/StringSwitch.h>
#include <llvm/Support/Path.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../driver/driver.h"
#include "../support/utility.h"

using namespace cx;

Value* IRGenerator::emitVarExpr(const VarExpr& expr) {
    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.decl)) {
        return emitEnumCase(*enumCase, {});
    }
    return getValue(expr.decl);
}

Value* IRGenerator::emitStringLiteralExpr(const StringLiteralExpr& expr) {
    if ((expr.type.removeOptional().isPointerType() && expr.type.removeOptional().getPointee().isChar())
        || (expr.type.removeOptional().isUnsizedArrayPointer() && expr.type.removeOptional().getElementType().isChar())) {
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
        arrayRef = createInsertValue(arrayRef, createConstantInt(Type::getInt(), expr.value.size()), sizeField - arrayRefFields.begin());
        return createInsertValue(createUndefined(stringType), arrayRef, charactersField - stringFields.begin());
    }

    auto* stringPtr = createGlobalStringPtr(expr.value);
    auto* size = createConstantInt(Type::getInt(), expr.value.size());
    auto* alloca = createEntryBlockAlloca(BasicType::get("string", {}), "__str");
    Function* stringConstructor = nullptr;

    for (auto* decl : Module::getStdlibModule()->symbolTable.findInTopLevelScope("string.init")) {
        auto params = llvm::cast<ConstructorDecl>(decl)->getParams();
        if (params.size() == 2 && params[0].type.isPointerType() && params[1].type.isInt()) {
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
        return createConstantFP(expr.type, expr.value.roundToDouble());
    }

    return createConstantInt(expr.type, expr.value);
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
        return emitOptionalConstruction(expr.type.getWrappedType(), nullptr);
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

Value* IRGenerator::emitOptionalConstruction(Type wrappedType, Expr* arg) {
    auto* decl = Module::getStdlibModule()->symbolTable.findOne("Optional");
    auto* enumDecl = llvm::cast<EnumDecl>(llvm::cast<TypeTemplate>(decl)->instantiate(GenericArg(wrappedType)));
    auto* enumCase = enumDecl->getCaseByName(arg ? "Some" : "None");
    ASSERT(enumCase);
    if (arg) {
        NamedValue argValue(arg);
        return emitEnumCase(*enumCase, llvm::ArrayRef<NamedValue>(&argValue, 1));
    }
    return emitEnumCase(*enumCase, {});
}

Value* IRGenerator::emitOptionalHasValueTest(Value* enumValue) {
    auto* tag = createExtractValue(enumValue, optionalTagFieldIndex);
    return createBinaryOp(Token::Equal, tag, createConstantInt(Type::getInt(), getOptionalSomeTag()), nullptr);
}

Value* IRGenerator::emitOptionalPayloadPtr(Value* enumPtr, Type wrappedType) {
    // The Some payload is a single-element anonymous struct, so the wrapped value sits at offset zero of the payload union.
    return createCast(createGEP(enumPtr, optionalPayloadFieldIndex), wrappedType.getPointerTo());
}

Value* IRGenerator::emitOptionalUnwrap(Expr& operand, const Expr& expr, const llvm::Twine& name) {
    auto* value = emitExpr(operand);
    llvm::StringRef message = "Unwrap failed";

    if (operand.type.isImplementedAsPointer()) {
        emitAssert(value, &expr, expr.location, message, name);
        return value;
    } else {
        emitAssert(emitOptionalHasValueTest(value), &expr, expr.location, message, name);
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
    case Token::Minus:
        return createNeg(emitExpr(expr.getOperand()));
    case Token::Star: {
        auto operand = emitExpr(expr.getOperand());
        if (auto load = llvm::dyn_cast<LoadInst>(operand)) {
            load->expr = &expr;
        }
        return operand;
    }
    case Token::And: {
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
        LLVM_FALLTHROUGH;
    case Token::Tilde:
        return emitNot(expr);
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
        result = createGEP(value, {createConstantInt(Type::getInt(), increment)});
    } else if (value->getType()->isFloatingPoint()) {
        result = createBinaryOp(Token::Plus, value, createConstantFP(value->getType(), increment), &expr);
    } else {
        llvm_unreachable("unknown increment/decrement operand type");
    }

    createStore(result, ptr);
    return nullptr;
}

Value* IRGenerator::emitLogicalAnd(const Expr& left, const Expr& right) {
    auto* rhsBlock = new BasicBlock("and.rhs", insertBlock->parent);
    auto* endBlock = new BasicBlock("and.end");

    Value* lhs = emitBoolConvertibleOperand(left);
    createCondBr(lhs, rhsBlock, endBlock, lhs);

    setInsertPoint(rhsBlock);
    Value* rhs = emitBoolConvertibleOperand(right);
    createBr(endBlock, rhs);

    setInsertPoint(endBlock);
    endBlock->parameter = new Parameter{ValueKind::Parameter, lhs->getType(), "and"};
    return endBlock->parameter;
}

Value* IRGenerator::emitLogicalOr(const Expr& left, const Expr& right) {
    auto* rhsBlock = new BasicBlock("or.rhs", insertBlock->parent);
    auto* endBlock = new BasicBlock("or.end");

    Value* lhs = emitBoolConvertibleOperand(left);
    createCondBr(lhs, endBlock, rhsBlock, lhs);

    setInsertPoint(rhsBlock);
    Value* rhs = emitBoolConvertibleOperand(right);
    createBr(endBlock, rhs);

    setInsertPoint(endBlock);
    endBlock->parameter = new Parameter{ValueKind::Parameter, lhs->getType(), "or"};
    return endBlock->parameter;
}

Value* IRGenerator::emitBoolConvertibleOperand(const Expr& expr) {
    auto* value = emitExpr(expr);
    if (value->getType()->isPointerType()) {
        return emitImplicitNullComparison(value);
    } else if (expr.type.isOptionalType() && !expr.type.getWrappedType().isPointerType()) {
        return emitOptionalHasValueTest(value);
    }
    return value;
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
    createBr(endBlock, emitExpr(expr.getRHS()));

    setInsertPoint(endBlock);
    endBlock->parameter = new Parameter{ValueKind::Parameter, thenValue->getType(), "coalesce"};
    return endBlock->parameter;
}

static int getIntegerBitWidth(IRType* type) {
    return llvm::StringSwitch<int>(llvm::cast<IRBasicType>(type)->name)
        .Cases({"int8", "uint8", "byte"}, 8)
        .Cases({"int16", "uint16"}, 16)
        .Cases({"int", "int32", "uint", "uint32"}, 32)
        .Cases({"int64", "uint64"}, 64)
        .Cases({"int128", "uint128"}, 128)
        .Default(0);
}

static Type getUnsignedIntegerType(int width) {
    switch (width) {
    case 8:
        return Type::getUInt8();
    case 16:
        return Type::getUInt16();
    case 32:
        return Type::getUInt();
    case 64:
        return Type::getUInt64();
    case 128:
        return Type::getUInt128();
    default:
        llvm_unreachable("invalid integer width");
    }
}

Value* IRGenerator::emitCheckedArithmetic(BinaryOperator op, Value* left, Value* right, const BinaryExpr& expr) {
    auto* type = left->getType();
    bool resultIsChar = type->isChar();
    if (resultIsChar) {
        // Chars compare as unsigned, so check them as 8-bit unsigned integers.
        auto* uint8Type = getIRType(Type::getUInt8());
        left = createCast(left, uint8Type);
        right = createCast(right, uint8Type);
        type = uint8Type;
    }

    int width = getIntegerBitWidth(type);
    ASSERT(width != 0);
    bool isSigned = type->isSignedInteger();

    Value* result;
    Value* overflowed;

    if (width < 64) {
        // The operation is exact in 64 bits, so any loss in the round trip is an overflow.
        // Widen only to 64 bits: MSVC and xcc (the playground's C compiler) don't support
        // __int128, so 64-bit operands use the in-width checks below instead of widening.
        auto* wideType = getIRType(isSigned ? Type::getInt64() : Type::getUInt64());
        auto* wideLeft = createCast(left, wideType);
        auto* wideRight = createCast(right, wideType);
        auto* wideResult = createBinaryOp(op, wideLeft, wideRight, &expr);
        result = createCast(wideResult, type);
        overflowed = createBinaryOp(Token::NotEqual, wideResult, createCast(result, wideType), &expr);
    } else if (op != Token::Star) {
        // Compute in the unsigned domain so the wrapping step isn't signed overflow in the C backend.
        auto* unsignedType = getIRType(getUnsignedIntegerType(width));
        auto* a = createCastIfNeeded(left, unsignedType);
        auto* b = createCastIfNeeded(right, unsignedType);
        auto* r = createBinaryOp(op, a, b, &expr);
        result = createCastIfNeeded(r, type);
        if (!isSigned) {
            overflowed = createBinaryOp(Token::Less, op == Token::Plus ? r : a, op == Token::Plus ? a : b, &expr);
        } else {
            // Add and subtract set the sign bit of (a^r)&(b^r) and (a^b)&(a^r) respectively on overflow.
            auto* x = createBinaryOp(Token::Xor, a, op == Token::Plus ? r : b, &expr);
            auto* y = createBinaryOp(Token::Xor, op == Token::Plus ? b : a, r, &expr);
            // Cast the 1 up from 32 bits: the C backend prints integer constants without a type,
            // so a bare 1 would shift as a C int.
            auto* one = createCast(createConstantInt(Type::getUInt(), 1), unsignedType);
            auto* signBit = createBinaryOp(Token::LeftShift, one, createConstantInt(unsignedType, width - 1), &expr);
            auto* signBitSet = createBinaryOp(Token::And, createBinaryOp(Token::And, x, y, &expr), signBit, &expr);
            overflowed = createBinaryOp(Token::NotEqual, signBitSet, createConstantInt(unsignedType, 0), &expr);
        }
    } else {
        // 64-bit multiply can't widen (MSVC and xcc lack __int128) and 128-bit multiply
        // has no wider type; check the wrapped result against division instead: with b != 0,
        // result / b != a exactly when the multiply overflowed. Operands are nonzero below,
        // and the MIN / -1 division trap is guarded, so the division is safe.
        auto* unsignedType = getIRType(getUnsignedIntegerType(width));
        auto* a = createCastIfNeeded(left, unsignedType);
        auto* b = createCastIfNeeded(right, unsignedType);
        result = createCastIfNeeded(createBinaryOp(Token::Star, a, b, &expr), type);

        auto* function = insertBlock->parent;
        auto* checkBlock = new BasicBlock("overflow.check", function);
        auto* endBlock = new BasicBlock("overflow.end");
        auto* divisorIsZero = createBinaryOp(Token::Equal, right, createConstantInt(type, 0), &expr);
        createCondBr(divisorIsZero, endBlock, checkBlock, createConstantBool(false));

        setInsertPoint(checkBlock);
        if (isSigned) {
            auto* minusOne = createConstantInt(type, -1);
            auto* one = createCast(createConstantInt(Type::getUInt(), 1), unsignedType);
            auto* minValue = createBinaryOp(Token::LeftShift, one, createConstantInt(unsignedType, width - 1), &expr);
            auto* min = createCast(minValue, type);
            auto* leftIsMinusOne = createBinaryOp(Token::Equal, left, minusOne, &expr);
            auto* rightIsMin = createBinaryOp(Token::Equal, right, min, &expr);
            auto* minCase1 = createBinaryOp(Token::And, leftIsMinusOne, rightIsMin, &expr);
            auto* leftIsMin = createBinaryOp(Token::Equal, left, min, &expr);
            auto* rightIsMinusOne = createBinaryOp(Token::Equal, right, minusOne, &expr);
            auto* minCase2 = createBinaryOp(Token::And, leftIsMin, rightIsMinusOne, &expr);
            auto* divBlock = new BasicBlock("overflow.div", function);
            createCondBr(createBinaryOp(Token::Or, minCase1, minCase2, &expr), endBlock, divBlock, createConstantBool(true));
            setInsertPoint(divBlock);
        }
        createBr(endBlock, createBinaryOp(Token::NotEqual, createBinaryOp(Token::Slash, result, right, &expr), left, &expr));

        setInsertPoint(endBlock);
        endBlock->parameter = new Parameter{ValueKind::Parameter, getIRType(Type::getBool()), "overflowed"};
        overflowed = endBlock->parameter;
    }

    emitAssert(createNot(overflowed), &expr, expr.location, "integer overflow", "overflow");
    if (resultIsChar) result = createCast(result, getIRType(Type::getChar()));
    return result;
}

Value* IRGenerator::emitBinaryExpr(const BinaryExpr& expr) {
    if (expr.isAssignment()) {
        return emitAssignment(expr);
    }

    if (expr.anonymousStructComparisonLowering) {
        // Anonymous struct comparison was lowered to elementwise comparison over
        // compiler-generated temporaries during typechecking. Evaluate each
        // side once and bind the temporaries to the values, so operands with
        // side effects run only once no matter how many elements are compared.
        // The bindings alias the values (no copies), so there is nothing to
        // destroy; they are removed right after the lowering is emitted.
        auto* lhsValue = emitExpr(expr.getLHS());
        auto* rhsValue = emitExpr(expr.getRHS());
        auto& bindings = scopes.back().valuesByDecl;
        bindings[expr.anonymousStructTempLHS] = lhsValue;
        bindings[expr.anonymousStructTempRHS] = rhsValue;
        auto* result = emitExpr(*expr.anonymousStructComparisonLowering);
        bindings.erase(expr.anonymousStructTempLHS);
        bindings.erase(expr.anonymousStructTempRHS);
        return result;
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
        if (left->getType()->isUnsignedInteger()) return createBinaryOp(Token::Modulo, left, right, &expr);
        // Positive remainder ((a % b) + b) % b. The operands are already evaluated values, so this doesn't re-evaluate them.
        auto* rem = createBinaryOp(Token::Modulo, left, right, &expr);
        auto* shifted = createBinaryOp(Token::Plus, rem, right, &expr);
        return createBinaryOp(Token::Modulo, shifted, right, &expr);
    }

    default:
        auto left = emitExprOrEnumTag(expr.getLHS(), nullptr);
        auto right = emitExprOrEnumTag(expr.getRHS(), nullptr);

        if (left->getType()->isPointerType() && left->getType()->getPointee()->equals(right->getType())) {
            left = createLoad(left);
        } else if (right->getType()->isPointerType() && right->getType()->getPointee()->equals(left->getType())) {
            right = createLoad(right);
        }

        if ((expr.op == Token::Plus || expr.op == Token::Minus || expr.op == Token::Star) && options.mode != BuildMode::ReleaseFast
            && (left->getType()->isInteger() || left->getType()->isChar())) {
            return emitCheckedArithmetic(expr.op, left, right, expr);
        }
        return createBinaryOp(expr.op, left, right, &expr);
    }
}

Value* IRGenerator::emitAssignment(const BinaryExpr& expr) {
    if (expr.getRHS().isUndefinedLiteralExpr()) return nullptr;

    auto lvalue = emitAssignmentLHS(expr.getLHS(), expr.lhsIsMoved);
    auto rvalue = emitExprForPassing(expr.getRHS(), lvalue->getType()->getPointee());
    createStore(rvalue, lvalue);
    return nullptr;
}

static bool isBuiltinArrayToSliceConversion(Type sourceType, IRType* targetType) {
    return sourceType.removePointer().isConstantArray() && targetType->isStruct() && targetType->getName().starts_with("Slice<");
}

static bool isListToSliceConversion(Type sourceType, IRType* targetType) {
    return sourceType.isBasicType() && sourceType.getName() == "List" && targetType->isStruct() && targetType->getName().starts_with("Slice<");
}

static bool isStringBufferToStringConversion(Type sourceType, IRType* targetType) {
    return sourceType.isBasicType() && sourceType.getName() == "StringBuffer" && targetType->isStruct() && targetType->getName() == "string";
}

Value* IRGenerator::emitExprForPassing(const Expr& expr, IRType* targetType) {
    if (!targetType) {
        // In variadic calls, arrays decay to pointers to their first element (as in C).
        if (expr.type.isConstantArray()) {
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
        ASSERT(expr.type.removePointer().isConstantArray());
        // Pointer-typed lvalues (e.g. spilled parameters) point at the pointer variable; load the pointer itself.
        auto* value = expr.type.isPointerType() ? emitExpr(expr) : emitExprAsPointer(expr);
        Value* elementPtr;
        if (expr.type.removePointer().getArraySize() == 0) {
            elementPtr = createConstantNull(targetType->getFields()[0].type);
        } else {
            elementPtr = createGEP(value, 0);
        }
        auto* arrayRef = createInsertValue(createUndefined(targetType), elementPtr, 0);
        auto size = createConstantInt(Type::getInt(), expr.type.removePointer().getArraySize());
        return createInsertValue(arrayRef, size, 1);
    }

    if (isListToSliceConversion(expr.type, targetType)) {
        auto* listPtr = emitExprAsPointer(expr);
        auto* buffer = createLoad(createGEP(listPtr, 0));
        auto* size = createLoad(createGEP(listPtr, 1));
        auto* arrayRef = createInsertValue(createUndefined(targetType), buffer, 0);
        return createInsertValue(arrayRef, size, 1);
    }

    if (isStringBufferToStringConversion(expr.type, targetType)) {
        auto* listPtr = createGEP(emitExprAsPointer(expr), 0);
        auto* buffer = createLoad(createGEP(listPtr, 0));
        auto* listSize = createLoad(createGEP(listPtr, 1));
        // StringBuffer stores a trailing null that the string view excludes.
        auto* size = createBinaryOp(Token::Minus, listSize, createConstantInt(listSize->getType(), 1), nullptr);
        auto* arrayRefType = targetType->getFields()[0].type;
        auto* arrayRef = createInsertValue(createUndefined(arrayRefType), buffer, 0);
        arrayRef = createInsertValue(arrayRef, size, 1);
        return createInsertValue(createUndefined(targetType), arrayRef, 0);
    }

    // Handle implicit conversions to type 'T[*]'.
    if (expr.type.removePointer().isConstantArray() && targetType->isPointerType() && !targetType->getPointee()->isArrayType()) {
        return createCast(loadThroughStorageAddress(emitLvalueExpr(expr), expr.type), targetType);
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
    auto* assertFail = getFunction(*llvm::cast<FunctionDecl>(Module::getStdlibModule()->symbolTable.findOne("assertFail")));
    auto messageAndLocation = llvm::join_items("", message, " at ", llvm::sys::path::filename(location.file), ":", std::to_string(location.line), ":",
                                               std::to_string(location.column), "\n");
    createCall(assertFail, createGlobalStringPtr(messageAndLocation), nullptr);
    createUnreachable();
}

Value* IRGenerator::emitEnumCase(const EnumCase& enumCase, llvm::ArrayRef<NamedValue> associatedValueElements) {
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
    auto* enumValue = createEntryBlockAlloca(enumDecl->getType(), "enum");
    createStore(tag, createGEP(enumValue, 0, nullptr, "tag"));

    if (!associatedValueElements.empty()) {
        Value* associatedValue = emitAggregateElements(enumCase.associatedType, associatedValueElements);
        auto* associatedValuePtr = createCast(createGEP(enumValue, 1, nullptr, "associatedValue"), associatedValue->getType()->getPointerTo());
        createStore(associatedValue, associatedValuePtr);
    }

    return enumValue;
}

Value* IRGenerator::emitEnumCaseCall(const EnumCase& enumCase, const CallExpr& expr) {
    if (expr.argParamIndices.size() != expr.args.size()) return emitEnumCase(enumCase, expr.args);
    auto enumDecl = enumCase.getEnumDecl();
    auto tag = emitExpr(*enumCase.value);
    if (!enumDecl->hasAssociatedValues()) return tag;

    auto* enumValue = createEntryBlockAlloca(enumDecl->getType(), "enum");
    createStore(tag, createGEP(enumValue, 0, nullptr, "tag"));

    if (!expr.args.empty()) {
        llvm::SmallVector<Value*, 8> writtenValues;
        for (auto& arg : expr.args)
            writtenValues.push_back(emitExpr(*arg.value));
        Value* associatedValue = createUndefined(enumCase.associatedType);
        for (size_t i = 0; i < expr.args.size(); ++i)
            associatedValue = createInsertValue(associatedValue, writtenValues[i], expr.argParamIndices[i]);
        auto* associatedValuePtr = createCast(createGEP(enumValue, 1, nullptr, "associatedValue"), associatedValue->getType()->getPointerTo());
        createStore(associatedValue, associatedValuePtr);
    }

    return enumValue;
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
    return createCall(function, args, &expr);
}

Value* IRGenerator::emitCallExpr(const CallExpr& expr, AllocaInst* thisAllocaForInit) {
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

    if (expr.getReceiver() && expr.receiverType.removePointer().isArrayType()) {
        if (expr.getFunctionName() == "size") {
            return getArrayLength(*expr.getReceiver(), expr.receiverType.removePointer());
        }
        if (expr.getFunctionName() == "data") {
            return getArrayData(*expr.getReceiver(), expr.receiverType.removePointer());
        }
        if (expr.getFunctionName() == "iterator") {
            return getArrayIterator(*expr.getReceiver(), expr.receiverType.removePointer());
        }
        llvm_unreachable("unknown array member function");
    }

    if (expr.isMoveInit()) {
        auto* receiverValue = loadThroughStorageAddress(emitExprAsPointer(*expr.getReceiver()), expr.getReceiver()->type);
        auto* argumentValue = emitExpr(*expr.args[0].value);
        createStore(argumentValue, receiverValue);
        return nullptr;
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
            } else if (currentDecl->isConstructorDecl() && expr.getFunctionName() == "init") {
                args.emplace_back(getThis(*param));
            } else {
                args.emplace_back(createEntryBlockAlloca(constructorDecl->getTypeDecl()->getType()));
            }
        } else if (expr.getReceiver()) {
            args.emplace_back(emitExprForPassing(*expr.getReceiver(), *param));
        } else {
            args.emplace_back(getThis());
        }
        ++param;
    }

    std::vector<IRType*> argParamTypes(param, params.end());
    if (expr.argParamIndices.size() == expr.args.size()) {
        llvm::SmallVector<Value*, 16> writtenValues;
        for (size_t i = 0; i < expr.args.size(); ++i) {
            int paramIndex = expr.argParamIndices[i];
            IRType* paramType = (paramIndex != -1 && size_t(paramIndex) < argParamTypes.size()) ? argParamTypes[size_t(paramIndex)] : nullptr;
            auto* argValue = emitExprForPassing(*expr.args[i].value, paramType);
            ASSERT(!paramType || argValue->getType()->equals(paramType));
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
        return createCall(calleeValue, args, &expr);
    }
}

Value* IRGenerator::emitBuiltinCast(const CallExpr& expr) {
    auto* value = emitExpr(*expr.args.front().value);
    auto type = expr.genericArgs.front().type;
    return createCastIfNeeded(value, type);
}

Value* IRGenerator::emitSizeofExpr(const SizeofExpr& expr) {
    return createSizeof(expr.operandType);
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
        auto index = baseTypeDecl->isUnion() ? 0 : baseTypeDecl->getFieldIndex(field);
        return createExtractValue(baseValue, index, field->getName());
    }
}

Value* IRGenerator::getArrayLength(const Expr&, Type objectType) {
    return createConstantInt(Type::getInt(), objectType.getArraySize());
}

Value* IRGenerator::getArrayData(const Expr& object, Type objectType) {
    if (objectType.isUnsizedArrayPointer()) {
        return emitExpr(object);
    }
    // Pointer-typed lvalues (e.g. spilled parameters) point at the pointer variable; load the pointer itself.
    auto* value = object.type.isPointerType() ? emitExpr(object) : emitExprAsPointer(object);
    ASSERT(value->getType()->getPointee()->isArrayType());
    if (objectType.getArraySize() == 0) {
        return createConstantNull(value->getType()->getPointee()->getElementType()->getPointerTo());
    }
    return createGEP(value, 0);
}

Value* IRGenerator::getArrayIterator(const Expr& object, Type objectType) {
    auto type = BasicType::get("ArrayIterator", GenericArg(objectType.getElementType()));
    if (objectType.getArraySize() == 0) {
        auto* irType = getIRType(type);
        auto fields = irType->getFields();
        ASSERT(fields.size() == 2);
        auto* iterator = createInsertValue(createUndefined(type), createConstantNull(fields[0].type), 0);
        return createInsertValue(iterator, createConstantNull(fields[1].type), 1);
    }
    // Pointer-typed lvalues (e.g. spilled parameters) point at the pointer variable; load the pointer itself.
    auto* value = object.type.isPointerType() ? emitExpr(object) : emitExprAsPointer(object);
    auto* elementPtr = createGEP(value, 0);
    auto* size = getArrayLength(object, objectType);
    auto* end = createGEP(elementPtr, {size});
    auto* iterator = createInsertValue(createUndefined(type), elementPtr, 0);
    return createInsertValue(iterator, end, 1);
}

Value* IRGenerator::emitMemberExpr(const MemberExpr& expr) {
    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.decl)) {
        return emitEnumCase(*enumCase, {});
    }

    if (auto* varDecl = llvm::dyn_cast_or_null<VarDecl>(expr.decl)) {
        return getValue(varDecl);
    }

    if (expr.base->type.removePointer().isAnonymousStructType()) {
        return emitAnonymousStructElementAccess(expr);
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

Value* IRGenerator::emitIndexedAccess(const Expr& base, const Expr& index) {
    auto* value = emitLvalueExpr(base);

    if (value->getType()->isPointerType() && value->getType()->getPointee()->isPointerType() && value->getType()->getPointee()->equals(getIRType(base.type))) {
        value = createLoad(value);
    }

    if (base.type.removeOptional().isUnsizedArrayPointer()) {
        return createGEP(value, {emitExpr(index)});
    } else {
        return createGEP(value, {createConstantInt(Type::getInt(), 0), emitExpr(index)});
    }
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
    if (!expr.operand->type.isOptionalType()) {
        return emitExpr(*expr.operand);
    }
    return emitOptionalUnwrap(*expr.operand, expr, "assert");
}

Value* IRGenerator::emitLambdaExpr(const LambdaExpr& expr) {
    auto functionDecl = expr.functionDecl;

    auto currentFunctionBackup = currentFunction;
    auto insertBlockBackup = insertBlock;
    auto scopesBackup = std::move(scopes);

    emitDecl(*functionDecl);

    currentFunction = currentFunctionBackup;
    scopes = std::move(scopesBackup);
    if (insertBlockBackup) setInsertPoint(insertBlockBackup);

    if (functionDecl->captures.empty()) {
        VarExpr varExpr(functionDecl->getName().str(), functionDecl->getLocation());
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
    return closure;
}

Value* IRGenerator::emitIfExpr(const IfExpr& expr) {
    // Integer ternaries fold in emitPlainExpr; fold boolean ones here so global initializers stay branch-free.
    if (expr.hasType() && expr.type.isBool() && expr.isFoldableBoolConstant()) {
        return createConstantBool(expr.getConstantBoolValue());
    }

    auto* condition = emitExpr(*expr.condition);
    if (condition->getType()->isPointerType()) {
        condition = emitImplicitNullComparison(condition);
    } else if (expr.condition->type.isOptionalType() && !expr.condition->type.getWrappedType().isPointerType()) {
        condition = emitOptionalHasValueTest(condition);
    }
    auto* function = currentFunction;
    auto* thenBlock = new BasicBlock("if.then", function);
    auto* elseBlock = new BasicBlock("if.else");
    auto* endIfBlock = new BasicBlock("if.end");
    createCondBr(condition, thenBlock, elseBlock);

    setInsertPoint(thenBlock);
    auto* thenValue = emitExpr(*expr.thenExpr);
    // Void branches produce no value to join; like void calls, the result is only usable in discard positions.
    bool isVoid = !thenValue || thenValue->getType()->isVoid();
    createBr(endIfBlock, isVoid ? nullptr : thenValue);

    setInsertPoint(elseBlock);
    auto* elseValue = emitExpr(*expr.elseExpr);
    createBr(endIfBlock, isVoid ? nullptr : elseValue);

    setInsertPoint(endIfBlock);
    if (isVoid) return thenValue;
    endIfBlock->parameter = new Parameter{ValueKind::Parameter, thenValue->getType(), "if.result"};
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
    auto* switchInst = createSwitch(condition, defaultBlock);

    auto casesIterator = cases.begin();
    for (auto& arm : expr.arms) {
        auto* value = casesIterator->first;
        auto* block = casesIterator->second;
        setInsertPoint(block);

        if (auto* associatedValue = arm.associatedValue) {
            auto type = associatedValue->type.getPointerTo();
            auto* associatedValuePtr = createCast(createGEP(enumValue, 1), type, associatedValue->getName());
            // The binding borrows the enum payload, so it must not run a destructor.
            setLocalValue(associatedValuePtr, associatedValue, false);
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

    setInsertPoint(end);
    end->parameter = new Parameter{ValueKind::Parameter, getIRType(expr.type), "switch.result"};
    return end->parameter;
}

Value* IRGenerator::emitImplicitCastExpr(const ImplicitCastExpr& expr) {
    switch (expr.castKind) {
    case ImplicitCastExpr::OptionalWrap:
        if (expr.type.getWrappedType().isImplementedAsPointer()) {
            return emitExpr(*expr.operand);
        } else {
            return emitOptionalConstruction(expr.operand->type, expr.operand);
        }
    case ImplicitCastExpr::OptionalUnwrap:
        return emitOptionalUnwrap(*expr.operand, expr, "__implicit_unwrap");
    case ImplicitCastExpr::AutoReference:
        return emitPlainExpr(*expr.operand);
    case ImplicitCastExpr::AutoDereference:
        return createLoad(emitPlainExpr(*expr.operand));
    case ImplicitCastExpr::NumericWiden:
        return createCastIfNeeded(emitExpr(*expr.operand), expr.type);
    }

    llvm_unreachable("all implicit cast kinds handled");
}

Value* IRGenerator::emitPlainExpr(const Expr& expr) {
    // Cyclic constants can leave expressions untyped; don't fold those.
    if (expr.hasType() && expr.type.isInteger() && expr.isFoldableIntConstant()) {
        return createConstantInt(expr.type, expr.getConstantIntegerValue());
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
    auto value = emitPlainExpr(expr);

    // Handle optionals that have been implicitly unwrapped due to data-flow analysis.
    // Pointer-implemented optionals need no access adjustment: the narrowed type is a compile-time view of the same value.
    if (expr.hasAssignableType() && expr.assignableType.isOptionalType() && !expr.assignableType.getWrappedType().isImplementedAsPointer()
        && expr.type == expr.assignableType.getWrappedType()) {
        // Temporaries are SSA values, not memory; spill to a temp so the payload access works.
        if (!value->getType()->isPointerType()) value = createTempAlloca(value);
        return emitOptionalPayloadPtr(value, expr.assignableType.getWrappedType());
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
