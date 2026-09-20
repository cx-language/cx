#include "irgen.h"
#pragma warning(push, 0)
#include <llvm/ADT/StringExtras.h>
#include <llvm/Support/Path.h>
#pragma warning(pop)
#include "../ast/module.h"
#include "../support/utility.h"

using namespace cx;

Value* IRGenerator::emitVarExpr(const VarExpr& expr) {
    return getValue(expr.decl);
}

Value* IRGenerator::emitStringLiteralExpr(const StringLiteralExpr& expr) {
    if ((expr.type.removeOptional().isPointerType() && expr.type.removeOptional().getPointee().isChar())
        || (expr.type.removeOptional().isUnsizedArrayPointer() && expr.type.removeOptional().getElementType().isChar())) {
        return createGlobalStringPtr(expr.value);
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
    ASSERT(someCase && someCase->associatedType && someCase->associatedType.getTupleElements().size() == 1);
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
    auto* enumDecl = llvm::cast<EnumDecl>(llvm::cast<TypeTemplate>(decl)->instantiate(wrappedType));
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
    // The Some payload is a single-element tuple, so the wrapped value sits at offset zero of the payload union.
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
    Value* array = createUndefined(expr.type);
    auto index = 0;

    for (auto& element : expr.elements) {
        auto* value = emitExpr(*element);
        array = createInsertValue(array, value, index++);
    }

    return array;
}

Value* IRGenerator::emitTupleExpr(const TupleExpr& expr) {
    Value* tuple = createUndefined(expr.type);
    int index = 0;
    for (auto& element : expr.elements) {
        tuple = createInsertValue(tuple, emitExpr(*element.value), index++);
    }
    return tuple;
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
        // Function parameters are SSA values, not memory, so spill them to a temporary to form a real address.
        // FIXME: This is a point-in-time copy; stores through the address don't update the parameter.
        // Remove once parameters get entry-block allocas ("Codegen allocas for parameters").
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

Value* IRGenerator::emitBinaryExpr(const BinaryExpr& expr) {
    if (expr.isAssignment()) {
        emitAssignment(expr);
        return nullptr;
    }

    if (expr.calleeDecl != nullptr) {
        return emitCallExpr(expr);
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
        return emitLogicalAnd(expr.getLHS(), expr.getRHS());

    case Token::OrOr:
        return emitLogicalOr(expr.getLHS(), expr.getRHS());

    default:
        auto left = emitExprOrEnumTag(expr.getLHS(), nullptr);
        auto right = emitExprOrEnumTag(expr.getRHS(), nullptr);

        if (left->getType()->isPointerType() && left->getType()->getPointee()->equals(right->getType())) {
            left = createLoad(left);
        } else if (right->getType()->isPointerType() && right->getType()->getPointee()->equals(left->getType())) {
            right = createLoad(right);
        }

        return createBinaryOp(expr.op, left, right, &expr);
    }
}

void IRGenerator::emitAssignment(const BinaryExpr& expr) {
    if (expr.getRHS().isUndefinedLiteralExpr()) return;

    auto lvalue = emitAssignmentLHS(expr.getLHS(), expr.lhsIsMoved);
    auto rvalue = emitExprForPassing(expr.getRHS(), lvalue->getType()->getPointee());
    createStore(rvalue, lvalue);
}

static bool isBuiltinArrayToArrayRefConversion(Type sourceType, IRType* targetType) {
    return sourceType.removePointer().isConstantArray() && targetType->isStruct() && targetType->getName().starts_with("ArrayRef<");
}

Value* IRGenerator::emitExprForPassing(const Expr& expr, IRType* targetType) {
    if (!targetType) {
        // In variadic calls, arrays decay to pointers to their first element (as in C).
        if (expr.type.isConstantArray()) {
            auto* value = emitExprAsPointer(expr);
            ASSERT(value->getType()->getPointee()->isArrayType());
            return createGEP(value, 0);
        }
        return emitExpr(expr);
    }

    // TODO: Handle implicit conversions in a separate function.

    if (isBuiltinArrayToArrayRefConversion(expr.type, targetType)) {
        ASSERT(expr.type.removePointer().isConstantArray());
        auto* value = emitExprAsPointer(expr);
        auto* elementPtr = createGEP(value, 0);
        auto* arrayRef = createInsertValue(createUndefined(targetType), elementPtr, 0);
        auto size = createConstantInt(Type::getInt(), expr.type.removePointer().getArraySize());
        return createInsertValue(arrayRef, size, 1);
    }

    // Handle implicit conversions to type 'T[*]'.
    if (expr.type.removePointer().isConstantArray() && targetType->isPointerType() && !targetType->getPointee()->isArrayType()) {
        return createCast(emitLvalueExpr(expr), targetType);
    }

    // Handle implicit conversions to void pointer, and to base type pointer.
    // Skip this when the target is a pointer to the source type. AutoReference keeps the source type (see Typechecker::convert),
    // so that case is taking the address (handled by the temp alloca below), not a bitcast.
    if (expr.type.isImplementedAsPointer() && targetType->isPointerType() && !getIRType(expr.type)->equals(targetType->getPointee())) {
        return createCastIfNeeded(emitExpr(expr), targetType);
    }

    // TODO: Refactor the following.
    auto* value = emitLvalueExpr(expr);

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
    auto* assertFail = getFunction(*llvm::cast<FunctionDecl>(Module::getStdlibModule()->symbolTable.findOne("assertFail")));
    createCondBr(condition, failBlock, successBlock);
    setInsertPoint(failBlock);
    auto messageAndLocation = llvm::join_items("", message, " at ", llvm::sys::path::filename(location.file), ":", std::to_string(location.line), ":",
                                               std::to_string(location.column), "\n");
    createCall(assertFail, createGlobalStringPtr(messageAndLocation), nullptr);
    createUnreachable();
    setInsertPoint(successBlock);
}

Value* IRGenerator::emitEnumCase(const EnumCase& enumCase, llvm::ArrayRef<NamedValue> associatedValueElements) {
    auto enumDecl = enumCase.getEnumDecl();
    auto tag = emitExpr(*enumCase.value);
    if (!enumDecl->hasAssociatedValues()) return tag;

    // TODO: Could reuse variable alloca instead of always creating a new one here.
    auto* enumValue = createEntryBlockAlloca(enumDecl->getType(), "enum");
    createStore(tag, createGEP(enumValue, 0, nullptr, "tag"));

    if (!associatedValueElements.empty()) {
        // TODO: This is duplicated in emitTupleExpr.
        Value* associatedValue = createUndefined(enumCase.associatedType);
        int index = 0;
        for (auto& element : associatedValueElements) {
            associatedValue = createInsertValue(associatedValue, emitExpr(*element.value), index++);
        }
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
        auto& message = llvm::cast<StringLiteralExpr>(*expr.args[1].value).value;
        emitAssert(emitExpr(*expr.args.front().value), &expr, expr.callee->location, message);
        return nullptr;
    }

    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.calleeDecl)) {
        return emitEnumCase(*enumCase, expr.args);
    }

    if (expr.getReceiver() && expr.receiverType.removePointer().isArrayType()) {
        if (expr.getFunctionName() == "size") {
            return getArrayLength(*expr.getReceiver(), expr.receiverType.removePointer());
        }
        if (expr.getFunctionName() == "iterator") {
            return getArrayIterator(*expr.getReceiver(), expr.receiverType.removePointer());
        }
        llvm_unreachable("unknown array member function");
    }

    if (expr.isMoveInit()) {
        auto* receiverValue = emitExpr(*expr.getReceiver());
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

    for (const auto& arg : expr.args) {
        auto paramType = param != params.end() ? *param++ : nullptr;
        auto* argValue = emitExprForPassing(*arg.value, paramType);
        ASSERT(!paramType || argValue->getType()->equals(paramType));
        args.push_back(argValue);
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
    auto type = expr.genericArgs.front();
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

Value* IRGenerator::getArrayIterator(const Expr& object, Type objectType) {
    auto type = BasicType::get("ArrayIterator", objectType.getElementType());
    auto* value = emitExprAsPointer(object);
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

    if (expr.base->type.removePointer().isTupleType()) {
        return emitTupleElementAccess(expr);
    }

    return emitMemberAccess(emitLvalueExpr(*expr.base), llvm::cast<FieldDecl>(expr.decl), &expr);
}

Value* IRGenerator::emitTupleElementAccess(const MemberExpr& expr) {
    unsigned index = 0;
    for (auto& element : expr.base->type.removePointer().getTupleElements()) {
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
        return emitCallExpr(expr);
    }

    auto gep = emitIndexedAccess(*expr.getBase(), *expr.getIndex());
    createStore(emitExpr(*expr.getValue()), gep);
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
    createBr(endIfBlock, thenValue);

    setInsertPoint(elseBlock);
    auto* elseValue = emitExpr(*expr.elseExpr);
    createBr(endIfBlock, elseValue);

    setInsertPoint(endIfBlock);
    endIfBlock->parameter = new Parameter{ValueKind::Parameter, thenValue->getType(), "if.result"};
    return endIfBlock->parameter;
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
    }

    llvm_unreachable("all implicit cast kinds handled");
}

Value* IRGenerator::emitPlainExpr(const Expr& expr) {
    if (expr.isConstant() && expr.type.isInteger()) {
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
    case ExprKind::TupleExpr:
        return emitTupleExpr(llvm::cast<TupleExpr>(expr));
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
                // Aggregate-typed parameters have no address; spill to a temp so the tag load and associated-value access work.
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
        // Function parameters are SSA values, not memory; spill to a temp so the payload access works.
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
