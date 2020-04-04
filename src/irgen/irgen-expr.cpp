#include "irgen.h"
#include <llvm/Support/Path.h>
#include "../ast/module.h"
#include "../support/utility.h"

using namespace delta;

llvm::Value* IRGenerator::codegenVarExpr(const VarExpr& expr) {
    return getValue(expr.getDecl());
}

llvm::Value* IRGenerator::codegenStringLiteralExpr(const StringLiteralExpr& expr) {
    if (expr.getType().isBasicType() && expr.getType().getName() == "string") {
        ASSERT(builder.GetInsertBlock(), "CreateGlobalStringPtr requires block to insert into");
        auto* stringPtr = builder.CreateGlobalStringPtr(expr.getValue());
        auto* size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), expr.getValue().size());
        static int stringLiteralCounter = 0;
        auto* type = getLLVMType(BasicType::get("string", {}));
        auto* alloca = createEntryBlockAlloca(type, nullptr, "__str" + std::to_string(stringLiteralCounter++));
        llvm::Function* stringConstructor = nullptr;

        for (auto* decl : Module::getStdlibModule()->getSymbolTable().find("string.init")) {
            auto params = llvm::cast<ConstructorDecl>(decl)->getParams();
            if (params.size() == 2 && params[0].getType().isPointerType() && params[1].getType().isInt()) {
                stringConstructor = getFunctionProto(*llvm::cast<ConstructorDecl>(decl));
                break;
            }
        }

        ASSERT(stringConstructor);
        builder.CreateCall(stringConstructor, { alloca, stringPtr, size });
        return alloca;
    } else {
        // Passing as C-string, i.e. char pointer.
        ASSERT(expr.getType().removeOptional().isPointerType() && expr.getType().removeOptional().getPointee().isChar());
        return builder.CreateGlobalStringPtr(expr.getValue());
    }
}

llvm::Value* IRGenerator::codegenCharacterLiteralExpr(const CharacterLiteralExpr& expr) {
    return llvm::ConstantInt::get(getLLVMType(expr.getType()), expr.getValue());
}

llvm::Value* IRGenerator::codegenIntLiteralExpr(const IntLiteralExpr& expr) {
    auto type = getLLVMType(expr.getType());
    // Integer literals may be typed as floating-point when used in a context
    // that requires a floating-point value. It might make sense to combine
    // IntLiteralExpr and FloatLiteralExpr into a single class.
    if (expr.getType().isFloatingPoint()) {
        return llvm::ConstantFP::get(type, expr.getValue().roundToDouble());
    }
    return llvm::ConstantInt::get(type, expr.getValue().extOrTrunc(type->getIntegerBitWidth()));
}

llvm::Value* IRGenerator::codegenFloatLiteralExpr(const FloatLiteralExpr& expr) {
    return llvm::ConstantFP::get(getLLVMType(expr.getType()), expr.getValue());
}

llvm::Value* IRGenerator::codegenBoolLiteralExpr(const BoolLiteralExpr& expr) {
    return expr.getValue() ? llvm::ConstantInt::getTrue(ctx) : llvm::ConstantInt::getFalse(ctx);
}

llvm::Value* IRGenerator::codegenNullLiteralExpr(const NullLiteralExpr& expr) {
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(getLLVMType(expr.getType())));
}

llvm::Value* IRGenerator::codegenUndefinedLiteralExpr(const UndefinedLiteralExpr& expr) {
    return llvm::UndefValue::get(getLLVMType(expr.getType()));
}

llvm::Value* IRGenerator::codegenArrayLiteralExpr(const ArrayLiteralExpr& expr) {
    auto* arrayType = llvm::ArrayType::get(getLLVMType(expr.getElements()[0]->getType()), expr.getElements().size());
    llvm::Value* array = llvm::UndefValue::get(arrayType);
    unsigned index = 0;
    for (auto& element : expr.getElements()) {
        auto* value = codegenExpr(*element);
        array = builder.CreateInsertValue(array, value, index++);
    }
    return array;
}

llvm::Value* IRGenerator::codegenTupleExpr(const TupleExpr& expr) {
    llvm::Value* tuple = llvm::UndefValue::get(getLLVMType(expr.getType()));
    unsigned index = 0;
    for (auto& element : expr.getElements()) {
        tuple = builder.CreateInsertValue(tuple, codegenExpr(*element.getValue()), index++);
    }
    return tuple;
}

llvm::Value* IRGenerator::codegenImplicitNullComparison(llvm::Value* operand) {
    auto* pointerType = llvm::cast<llvm::PointerType>(operand->getType());
    return builder.CreateICmpNE(operand, llvm::ConstantPointerNull::get(pointerType));
}

llvm::Value* IRGenerator::codegenNot(const UnaryExpr& expr) {
    auto* operand = codegenExpr(expr.getOperand());
    if (operand->getType()->isPointerTy()) {
        operand = codegenImplicitNullComparison(operand);
    }
    return builder.CreateNot(operand);
}

llvm::Value* IRGenerator::codegenUnaryExpr(const UnaryExpr& expr) {
    switch (expr.getOperator()) {
        case Token::Plus:
            return codegenExpr(expr.getOperand());
        case Token::Minus: {
            auto operand = codegenExpr(expr.getOperand());
            return expr.getOperand().getType().isFloatingPoint() ? builder.CreateFNeg(operand) : builder.CreateNeg(operand);
        }
        case Token::Star:
            return codegenExpr(expr.getOperand());
        case Token::And:
            return codegenExprAsPointer(expr.getOperand());
        case Token::Not:
        case Token::Tilde:
            return codegenNot(expr);
        case Token::Increment:
            return codegenConstantIncrement(expr, 1);
        case Token::Decrement:
            return codegenConstantIncrement(expr, -1);
        default:
            llvm_unreachable("invalid prefix operator");
    }
}

// TODO: Lower increment and decrement statements to compound assignments so this isn't needed.
llvm::Value* IRGenerator::codegenConstantIncrement(const UnaryExpr& expr, int increment) {
    auto operandType = expr.getOperand().getType();
    auto* ptr = codegenLvalueExpr(expr.getOperand());
    if (operandType.isPointerType() && llvm::isa<llvm::AllocaInst>(ptr)) {
        ptr = createLoad(ptr);
    }
    auto* value = createLoad(ptr);
    llvm::Value* result;

    if (value->getType()->isIntegerTy()) {
        result = builder.CreateAdd(value, llvm::ConstantInt::getSigned(value->getType(), increment));
    } else if (value->getType()->isPointerTy()) {
        result = builder.CreateInBoundsGEP(value, llvm::ConstantInt::getSigned(llvm::IntegerType::getInt32Ty(ctx), increment));
    } else if (value->getType()->isFloatingPointTy()) {
        result = builder.CreateFAdd(value, llvm::ConstantFP::get(value->getType(), increment));
    } else {
        llvm_unreachable("unknown increment/decrement operand type");
    }

    createStore(result, ptr);
    return nullptr;
}

llvm::Value* IRGenerator::codegenLogicalAnd(const Expr& left, const Expr& right) {
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "and.rhs", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "and.end", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegenExpr(left);
    builder.CreateCondBr(lhs, rhsBlock, endBlock);
    auto* lhsBlock = builder.GetInsertBlock();

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegenExpr(right);
    builder.CreateBr(endBlock);
    rhsBlock = builder.GetInsertBlock();

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "and");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* IRGenerator::codegenLogicalOr(const Expr& left, const Expr& right) {
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "or.rhs", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "or.end", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegenExpr(left);
    builder.CreateCondBr(lhs, endBlock, rhsBlock);
    auto* lhsBlock = builder.GetInsertBlock();

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegenExpr(right);
    builder.CreateBr(endBlock);
    rhsBlock = builder.GetInsertBlock();

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "or");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* IRGenerator::codegenBinaryOp(Token::Kind op, llvm::Value* lhs, llvm::Value* rhs, Type lhsType) {
    if (lhs->getType()->isPointerTy() && lhs->getType()->getPointerElementType() == rhs->getType()) {
        lhs = createLoad(lhs);
        lhsType = lhsType.getPointee();
    } else if (rhs->getType()->isPointerTy() && rhs->getType()->getPointerElementType() == lhs->getType()) {
        rhs = createLoad(rhs);
    }

    if (lhs->getType()->isFloatingPointTy()) {
        switch (op) {
            case Token::Equal:
                return builder.CreateFCmpOEQ(lhs, rhs);
            case Token::NotEqual:
                return builder.CreateFCmpONE(lhs, rhs);
            case Token::Less:
                return builder.CreateFCmpOLT(lhs, rhs);
            case Token::LessOrEqual:
                return builder.CreateFCmpOLE(lhs, rhs);
            case Token::Greater:
                return builder.CreateFCmpOGT(lhs, rhs);
            case Token::GreaterOrEqual:
                return builder.CreateFCmpOGE(lhs, rhs);
            case Token::Plus:
                return builder.CreateFAdd(lhs, rhs);
            case Token::Minus:
                return builder.CreateFSub(lhs, rhs);
            case Token::Star:
                return builder.CreateFMul(lhs, rhs);
            case Token::Slash:
                return builder.CreateFDiv(lhs, rhs);
            case Token::Modulo:
                return builder.CreateFRem(lhs, rhs);
            default:
                llvm_unreachable("all cases handled");
        }
    }

    switch (op) {
        case Token::Equal:
        case Token::PointerEqual:
            return builder.CreateICmpEQ(lhs, rhs);
        case Token::NotEqual:
        case Token::PointerNotEqual:
            return builder.CreateICmpNE(lhs, rhs);
        case Token::Plus:
            return builder.CreateAdd(lhs, rhs);
        case Token::Minus:
            return builder.CreateSub(lhs, rhs);
        case Token::Star:
            return builder.CreateMul(lhs, rhs);
        case Token::And:
            return builder.CreateAnd(lhs, rhs);
        case Token::Or:
            return builder.CreateOr(lhs, rhs);
        case Token::Xor:
            return builder.CreateXor(lhs, rhs);
        case Token::LeftShift:
            return builder.CreateShl(lhs, rhs);
        default:
            break;
    }

    bool isSigned = lhsType.isChar() ? false : lhsType.isSigned();

    switch (op) {
        case Token::Less:
            return isSigned ? builder.CreateICmpSLT(lhs, rhs) : builder.CreateICmpULT(lhs, rhs);
        case Token::LessOrEqual:
            return isSigned ? builder.CreateICmpSLE(lhs, rhs) : builder.CreateICmpULE(lhs, rhs);
        case Token::Greater:
            return isSigned ? builder.CreateICmpSGT(lhs, rhs) : builder.CreateICmpUGT(lhs, rhs);
        case Token::GreaterOrEqual:
            return isSigned ? builder.CreateICmpSGE(lhs, rhs) : builder.CreateICmpUGE(lhs, rhs);
        case Token::Slash:
            return isSigned ? builder.CreateSDiv(lhs, rhs) : builder.CreateUDiv(lhs, rhs);
        case Token::Modulo:
            return isSigned ? builder.CreateSRem(lhs, rhs) : builder.CreateURem(lhs, rhs);
        case Token::RightShift:
            return isSigned ? builder.CreateAShr(lhs, rhs) : builder.CreateLShr(lhs, rhs);
        default:
            llvm_unreachable("all cases handled");
    }
}

llvm::Value* IRGenerator::codegenShortCircuitBinaryOp(BinaryOperator op, const Expr& lhs, const Expr& rhs) {
    switch (op) {
        case Token::AndAnd:
            return codegenLogicalAnd(lhs, rhs);
        case Token::OrOr:
            return codegenLogicalOr(lhs, rhs);
        default:
            llvm_unreachable("invalid short-circuit binary operator");
    }
}

llvm::Value* IRGenerator::codegenBinaryExpr(const BinaryExpr& expr) {
    if (expr.isAssignment()) {
        codegenAssignment(expr);
        return nullptr;
    }

    if (expr.getCalleeDecl() != nullptr) {
        return codegenCallExpr(expr);
    }

    switch (expr.getOperator()) {
        case Token::AndAnd:
        case Token::OrOr:
            return codegenShortCircuitBinaryOp(expr.getOperator(), expr.getLHS(), expr.getRHS());
        default:
            auto* lhs = codegenExprOrEnumTag(expr.getLHS(), nullptr);
            auto* rhs = codegenExprOrEnumTag(expr.getRHS(), nullptr);
            return codegenBinaryOp(expr.getOperator(), lhs, rhs, expr.getLHS().getType());
    }
}

void IRGenerator::codegenAssignment(const BinaryExpr& expr) {
    if (expr.getRHS().isUndefinedLiteralExpr()) return;

    llvm::Value* lhsLvalue = codegenAssignmentLHS(expr.getLHS());
    auto rhsValue = codegenExprForPassing(expr.getRHS(), lhsLvalue->getType()->getPointerElementType());
    createStore(rhsValue, lhsLvalue);
}

bool isBuiltinArrayToArrayRefConversion(Type sourceType, llvm::Type* targetType) {
    return sourceType.removePointer().isArrayWithConstantSize() && targetType->isStructTy() &&
           targetType->getStructName().startswith("ArrayRef");
}

llvm::Value* IRGenerator::codegenExprForPassing(const Expr& expr, llvm::Type* targetType) {
    if (!targetType) {
        return codegenExpr(expr);
    }

    // TODO: Handle implicit conversions in a separate function.

    if (isBuiltinArrayToArrayRefConversion(expr.getType(), targetType)) {
        ASSERT(expr.getType().removePointer().isArrayWithConstantSize());
        auto* value = codegenExprAsPointer(expr);
        auto* elementPtr = builder.CreateConstGEP2_32(nullptr, value, 0, 0);
        auto* arrayRef = builder.CreateInsertValue(llvm::UndefValue::get(targetType), elementPtr, 0);
        auto size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), expr.getType().removePointer().getArraySize());
        return builder.CreateInsertValue(arrayRef, size, 1);
    }

    // Handle implicit conversions to type 'T[*]'.
    if (expr.getType().removePointer().isArrayWithConstantSize() && targetType->isPointerTy() &&
        !targetType->getPointerElementType()->isArrayTy()) {
        return builder.CreateBitOrPointerCast(codegenLvalueExpr(expr), targetType);
    }

    // Handle implicit conversions to void pointer.
    if (((expr.getType().isPointerType() && !expr.getType().getPointee().isVoid()) || expr.getType().isArrayWithUnknownSize()) &&
        targetType->isPointerTy() && targetType->getPointerElementType()->isIntegerTy(8)) {
        return builder.CreateBitCast(codegenExpr(expr), targetType);
    }

    // TODO: Refactor the following.
    auto* value = codegenLvalueExpr(expr);

    if (targetType->isPointerTy() && value->getType() == targetType->getPointerElementType()) {
        return createTempAlloca(value);
    } else if (value->getType()->isPointerTy() && targetType != value->getType()) {
        value = createLoad(value);
        if (value->getType()->isPointerTy() && targetType != value->getType()) {
            value = createLoad(value);
        }
        return value;
    } else {
        return value;
    }
}

llvm::Value* IRGenerator::codegenBuiltinConversion(const Expr& expr, Type type) {
    auto value = codegenExpr(expr);

    if (expr.getType().isUnsignedInteger() && type.isInteger()) {
        return builder.CreateZExtOrTrunc(value, getLLVMType(type));
    }
    if (expr.getType().isSignedInteger() && type.isInteger()) {
        return builder.CreateSExtOrTrunc(value, getLLVMType(type));
    }
    if ((expr.getType().isInteger() || expr.getType().isChar() || expr.getType().isBool()) && (type.isInteger() || type.isChar())) {
        return builder.CreateIntCast(value, getLLVMType(type), expr.getType().isSignedInteger());
    }
    if (expr.getType().isFloatingPoint()) {
        if (type.isSignedInteger()) return builder.CreateFPToSI(value, getLLVMType(type));
        if (type.isUnsignedInteger()) return builder.CreateFPToUI(value, getLLVMType(type));
        if (type.isFloatingPoint()) return builder.CreateFPCast(value, getLLVMType(type));
    }
    if (type.isFloatingPoint()) {
        if (expr.getType().isSignedInteger()) return builder.CreateSIToFP(value, getLLVMType(type));
        if (expr.getType().isUnsignedInteger()) return builder.CreateUIToFP(value, getLLVMType(type));
    }

    REPORT_ERROR(expr.getLocation(), "conversion from '" << expr.getType() << "' to '" << type << "' not supported");
    exit(1);
}

void IRGenerator::codegenAssert(llvm::Value* condition, SourceLocation location, llvm::StringRef message) {
    condition = builder.CreateIsNull(condition, "assert.condition");
    auto* function = builder.GetInsertBlock()->getParent();
    auto* failBlock = llvm::BasicBlock::Create(ctx, "assert.fail", function);
    auto* successBlock = llvm::BasicBlock::Create(ctx, "assert.success", function);
    auto* assertFail = getFunctionProto(*llvm::cast<FunctionDecl>(Module::getStdlibModule()->getSymbolTable().findOne("assertFail")));
    builder.CreateCondBr(condition, failBlock, successBlock);
    builder.SetInsertPoint(failBlock);
    auto messageAndLocation = llvm::join_items("", message, " at ", llvm::sys::path::filename(location.file), ":",
                                               std::to_string(location.line), ":", std::to_string(location.column), "\n");
    builder.CreateCall(assertFail, builder.CreateGlobalStringPtr(messageAndLocation));
    builder.CreateUnreachable();
    builder.SetInsertPoint(successBlock);
}

llvm::Value* IRGenerator::codegenEnumCase(const EnumCase& enumCase, llvm::ArrayRef<NamedValue> associatedValueElements) {
    auto enumDecl = enumCase.getEnumDecl();
    auto tag = codegenExpr(*enumCase.getValue());
    if (!enumDecl->hasAssociatedValues()) return tag;

    // TODO: Could reuse variable alloca instead of always creating a new one here.
    auto* enumValue = createEntryBlockAlloca(getLLVMType(enumDecl->getType()), nullptr, "enum");
    createStore(tag, builder.CreateStructGEP(enumValue, 0, "tag"));

    if (!associatedValueElements.empty()) {
        // TODO: This is duplicated in codegenTupleExpr.
        llvm::Value* associatedValue = llvm::UndefValue::get(getLLVMType(enumCase.getAssociatedType()));
        int index = 0;
        for (auto& element : associatedValueElements) {
            associatedValue = builder.CreateInsertValue(associatedValue, codegenExpr(*element.getValue()), index++);
        }
        auto* associatedValuePtr = builder.CreatePointerCast(builder.CreateStructGEP(enumValue, 1, "associatedValue"),
                                                             associatedValue->getType()->getPointerTo());
        createStore(associatedValue, associatedValuePtr);
    }

    return enumValue;
}

llvm::Value* IRGenerator::codegenCallExpr(const CallExpr& expr, llvm::AllocaInst* thisAllocaForInit) {
    if (expr.isBuiltinConversion()) {
        return codegenBuiltinConversion(*expr.getArgs().front().getValue(), expr.getType());
    }

    if (expr.isBuiltinCast()) {
        return codegenBuiltinCast(expr);
    }

    if (expr.getFunctionName() == "assert") {
        codegenAssert(codegenExpr(*expr.getArgs().front().getValue()), expr.getCallee().getLocation());
        return nullptr;
    }

    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.getCalleeDecl())) {
        return codegenEnumCase(*enumCase, expr.getArgs());
    }

    if (expr.getReceiver() && expr.getReceiverType().removePointer().isArrayType()) {
        if (expr.getFunctionName() == "size") {
            return getArrayLength(*expr.getReceiver(), expr.getReceiverType().removePointer());
        }
        if (expr.getFunctionName() == "iterator") {
            return getArrayIterator(*expr.getReceiver(), expr.getReceiverType().removePointer());
        }
        llvm_unreachable("unknown array member function");
    }

    if (expr.isMoveInit()) {
        auto* receiverValue = codegenExpr(*expr.getReceiver());
        auto* argumentValue = codegenExpr(*expr.getArgs()[0].getValue());
        createStore(argumentValue, receiverValue);
        return nullptr;
    }

    llvm::Value* calleeValue = getFunctionForCall(expr);

    if (!calleeValue) {
        return nullptr;
    }

    llvm::FunctionType* functionType;

    if (auto* function = llvm::dyn_cast<llvm::Function>(calleeValue)) {
        functionType = function->getFunctionType();
    } else {
        if (!llvm::isa<llvm::FunctionType>(calleeValue->getType()->getPointerElementType())) {
            calleeValue = createLoad(calleeValue);
        }
        functionType = llvm::cast<llvm::FunctionType>(calleeValue->getType()->getPointerElementType());
    }

    auto param = functionType->param_begin();
    auto paramEnd = functionType->param_end();

    llvm::SmallVector<llvm::Value*, 16> args;

    auto* calleeDecl = expr.getCalleeDecl();

    if (calleeDecl->isMethodDecl()) {
        if (auto* constructorDecl = llvm::dyn_cast<ConstructorDecl>(calleeDecl)) {
            if (thisAllocaForInit) {
                args.emplace_back(thisAllocaForInit);
            } else if (currentDecl->isConstructorDecl() && expr.getFunctionName() == "init") {
                args.emplace_back(getThis(*param));
            } else {
                args.emplace_back(createEntryBlockAlloca(getLLVMType(constructorDecl->getTypeDecl()->getType())));
            }
        } else if (expr.getReceiver()) {
            args.emplace_back(codegenExprForPassing(*expr.getReceiver(), *param));
        } else {
            args.emplace_back(getThis());
        }
        ++param;
    }

    for (const auto& arg : expr.getArgs()) {
        auto* paramType = param != paramEnd ? *param++ : nullptr;
        args.push_back(codegenExprForPassing(*arg.getValue(), paramType));
    }

    if (calleeDecl->isConstructorDecl()) {
        builder.CreateCall(calleeValue, args);
        return args[0];
    } else {
        return builder.CreateCall(calleeValue, args);
    }
}

llvm::Value* IRGenerator::codegenBuiltinCast(const CallExpr& expr) {
    auto* value = codegenExpr(*expr.getArgs().front().getValue());
    auto* type = getLLVMType(expr.getGenericArgs().front());

    if (value->getType()->isIntegerTy() && type->isIntegerTy()) {
        return builder.CreateIntCast(value, type, expr.getArgs().front().getValue()->getType().isSigned());
    }

    return builder.CreateBitOrPointerCast(value, type);
}

llvm::Value* IRGenerator::codegenSizeofExpr(const SizeofExpr& expr) {
    return llvm::ConstantExpr::getSizeOf(getLLVMType(expr.getType()));
}

llvm::Value* IRGenerator::codegenAddressofExpr(const AddressofExpr& expr) {
    llvm::Value* value = codegenExpr(expr.getOperand());
    llvm::Type* uintptr = getLLVMType(Type::getUIntPtr());
    return builder.CreatePtrToInt(value, uintptr, "address");
}

llvm::Value* IRGenerator::codegenMemberAccess(llvm::Value* baseValue, Type memberType, llvm::StringRef memberName) {
    auto baseType = baseValue->getType();
    if (baseType->isPointerTy()) {
        baseType = baseType->getPointerElementType();
        if (baseType->isPointerTy()) {
            baseType = baseType->getPointerElementType();
            baseValue = createLoad(baseValue);
        }
        auto& baseTypeDecl = *structs.find(baseType->getStructName())->second.second;

        if (baseTypeDecl.isUnion()) {
            return builder.CreateBitCast(baseValue, getLLVMType(memberType)->getPointerTo(), memberName);
        } else {
            auto index = baseTypeDecl.getFieldIndex(memberName);
            if (!baseType->isSized()) {
                codegenTypeDecl(baseTypeDecl);
            }
            return builder.CreateStructGEP(nullptr, baseValue, index, memberName);
        }
    } else {
        auto& baseTypeDecl = *structs.find(baseType->getStructName())->second.second;
        auto index = baseTypeDecl.isUnion() ? 0 : baseTypeDecl.getFieldIndex(memberName);
        return builder.CreateExtractValue(baseValue, index, memberName);
    }
}

llvm::Value* IRGenerator::getArrayLength(const Expr& object, Type objectType) {
    if (objectType.isArrayWithRuntimeSize()) {
        return builder.CreateExtractValue(codegenExpr(object), 1, "size");
    } else {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), objectType.getArraySize());
    }
}

llvm::Value* IRGenerator::getArrayIterator(const Expr& object, Type objectType) {
    auto* type = getLLVMType(BasicType::get("ArrayIterator", objectType.getElementType()));
    auto* value = codegenExprAsPointer(object);
    auto* elementPtr = builder.CreateConstInBoundsGEP2_32(nullptr, value, 0, 0);
    auto* size = getArrayLength(object, objectType);
    auto* end = builder.CreateInBoundsGEP(elementPtr, size);
    auto* iterator = builder.CreateInsertValue(llvm::UndefValue::get(type), elementPtr, 0);
    return builder.CreateInsertValue(iterator, end, 1);
}

llvm::Value* IRGenerator::codegenMemberExpr(const MemberExpr& expr) {
    if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(expr.getDecl())) {
        return codegenEnumCase(*enumCase, {});
    }

    if (expr.getBaseExpr()->getType().removePointer().isTupleType()) {
        return codegenTupleElementAccess(expr);
    }

    return codegenMemberAccess(codegenLvalueExpr(*expr.getBaseExpr()), expr.getType(), expr.getMemberName());
}

llvm::Value* IRGenerator::codegenTupleElementAccess(const MemberExpr& expr) {
    unsigned index = 0;
    for (auto& element : expr.getBaseExpr()->getType().removePointer().getTupleElements()) {
        if (element.name == expr.getMemberName()) break;
        ++index;
    }

    auto* baseValue = codegenLvalueExpr(*expr.getBaseExpr());
    if (baseValue->getType()->isPointerTy() && baseValue->getType()->getPointerElementType()->isPointerTy()) {
        baseValue = createLoad(baseValue);
    }

    if (baseValue->getType()->isPointerTy()) {
        return builder.CreateStructGEP(nullptr, baseValue, index, expr.getMemberName());
    } else {
        return builder.CreateExtractValue(baseValue, index, expr.getMemberName());
    }
}

llvm::Value* IRGenerator::codegenIndexExpr(const IndexExpr& expr) {
    if (!expr.getBase()->getType().removePointer().isArrayType()) {
        return codegenCallExpr(expr);
    }

    auto* value = codegenLvalueExpr(*expr.getBase());
    Type lhsType = expr.getBase()->getType();

    if (lhsType.isArrayWithRuntimeSize()) {
        if (value->getType()->isPointerTy()) {
            value = createLoad(value);
        }
        auto* ptr = builder.CreateExtractValue(value, 0);
        auto* index = codegenExpr(*expr.getIndex());
        return builder.CreateGEP(ptr, index);
    }

    if (value->getType()->isPointerTy() && value->getType()->getPointerElementType()->isPointerTy() &&
        value->getType()->getPointerElementType() == getLLVMType(lhsType)) {
        value = createLoad(value);
    }

    if (lhsType.isArrayWithUnknownSize()) {
        return builder.CreateGEP(value, codegenExpr(*expr.getIndex()));
    }

    return builder.CreateGEP(value, { llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), codegenExpr(*expr.getIndex()) });
}

llvm::Value* IRGenerator::codegenUnwrapExpr(const UnwrapExpr& expr) {
    auto* value = codegenExpr(expr.getOperand());
    codegenAssert(value, expr.getLocation(), "Unwrap failed");
    return value;
}

llvm::Value* IRGenerator::codegenLambdaExpr(const LambdaExpr& expr) {
    auto functionDecl = expr.getFunctionDecl();

    auto insertBlockBackup = builder.GetInsertBlock();
    auto insertPointBackup = builder.GetInsertPoint();
    auto scopesBackup = std::move(scopes);

    codegenFunctionDecl(*functionDecl);

    scopes = std::move(scopesBackup);
    if (insertBlockBackup) builder.SetInsertPoint(insertBlockBackup, insertPointBackup);

    VarExpr varExpr(functionDecl->getName(), functionDecl->getLocation());
    varExpr.setDecl(functionDecl);
    varExpr.setType(expr.getType());
    return codegenVarExpr(varExpr);
}

llvm::Value* IRGenerator::codegenIfExpr(const IfExpr& expr) {
    auto* condition = codegenExpr(*expr.getCondition());
    if (condition->getType()->isPointerTy()) {
        condition = codegenImplicitNullComparison(condition);
    }
    auto* function = builder.GetInsertBlock()->getParent();
    auto* thenBlock = llvm::BasicBlock::Create(ctx, "if.then", function);
    auto* elseBlock = llvm::BasicBlock::Create(ctx, "if.else");
    auto* endIfBlock = llvm::BasicBlock::Create(ctx, "if.end");
    builder.CreateCondBr(condition, thenBlock, elseBlock);

    builder.SetInsertPoint(thenBlock);
    auto* thenValue = codegenExpr(*expr.getThenExpr());
    builder.CreateBr(endIfBlock);
    thenBlock = builder.GetInsertBlock();

    function->getBasicBlockList().push_back(elseBlock);
    builder.SetInsertPoint(elseBlock);
    auto* elseValue = codegenExpr(*expr.getElseExpr());
    builder.CreateBr(endIfBlock);
    elseBlock = builder.GetInsertBlock();

    function->getBasicBlockList().push_back(endIfBlock);
    builder.SetInsertPoint(endIfBlock);
    auto* phi = builder.CreatePHI(thenValue->getType(), 2, "phi");
    phi->addIncoming(thenValue, thenBlock);
    phi->addIncoming(elseValue, elseBlock);
    return phi;
}

llvm::Value* IRGenerator::codegenImplicitCastExpr(const ImplicitCastExpr& expr) {
    return codegenExprWithoutAutoCast(*expr.getOperand());
}

llvm::Value* IRGenerator::codegenExprWithoutAutoCast(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VarExpr:
            return codegenVarExpr(llvm::cast<VarExpr>(expr));
        case ExprKind::StringLiteralExpr:
            return codegenStringLiteralExpr(llvm::cast<StringLiteralExpr>(expr));
        case ExprKind::CharacterLiteralExpr:
            return codegenCharacterLiteralExpr(llvm::cast<CharacterLiteralExpr>(expr));
        case ExprKind::IntLiteralExpr:
            return codegenIntLiteralExpr(llvm::cast<IntLiteralExpr>(expr));
        case ExprKind::FloatLiteralExpr:
            return codegenFloatLiteralExpr(llvm::cast<FloatLiteralExpr>(expr));
        case ExprKind::BoolLiteralExpr:
            return codegenBoolLiteralExpr(llvm::cast<BoolLiteralExpr>(expr));
        case ExprKind::NullLiteralExpr:
            return codegenNullLiteralExpr(llvm::cast<NullLiteralExpr>(expr));
        case ExprKind::UndefinedLiteralExpr:
            return codegenUndefinedLiteralExpr(llvm::cast<UndefinedLiteralExpr>(expr));
        case ExprKind::ArrayLiteralExpr:
            return codegenArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr));
        case ExprKind::TupleExpr:
            return codegenTupleExpr(llvm::cast<TupleExpr>(expr));
        case ExprKind::UnaryExpr:
            return codegenUnaryExpr(llvm::cast<UnaryExpr>(expr));
        case ExprKind::BinaryExpr:
            return codegenBinaryExpr(llvm::cast<BinaryExpr>(expr));
        case ExprKind::CallExpr:
            return codegenCallExpr(llvm::cast<CallExpr>(expr));
        case ExprKind::SizeofExpr:
            return codegenSizeofExpr(llvm::cast<SizeofExpr>(expr));
        case ExprKind::AddressofExpr:
            return codegenAddressofExpr(llvm::cast<AddressofExpr>(expr));
        case ExprKind::MemberExpr:
            return codegenMemberExpr(llvm::cast<MemberExpr>(expr));
        case ExprKind::IndexExpr:
            return codegenIndexExpr(llvm::cast<IndexExpr>(expr));
        case ExprKind::UnwrapExpr:
            return codegenUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
        case ExprKind::LambdaExpr:
            return codegenLambdaExpr(llvm::cast<LambdaExpr>(expr));
        case ExprKind::IfExpr:
            return codegenIfExpr(llvm::cast<IfExpr>(expr));
        case ExprKind::ImplicitCastExpr:
            return codegenImplicitCastExpr(llvm::cast<ImplicitCastExpr>(expr));
    }
    llvm_unreachable("all cases handled");
}

llvm::Value* IRGenerator::codegenExpr(const Expr& expr) {
    auto* value = codegenLvalueExpr(expr);
    if (value && value->getType()->isPointerTy() && value->getType()->getPointerElementType() == getLLVMType(expr.getType())) {
        value = createLoad(value);
    }
    return value;
}

llvm::Value* IRGenerator::codegenLvalueExpr(const Expr& expr) {
    return codegenAutoCast(codegenExprWithoutAutoCast(expr), expr);
}

llvm::Value* IRGenerator::codegenExprAsPointer(const Expr& expr) {
    auto* value = codegenLvalueExpr(expr);
    if (!value->getType()->isPointerTy()) {
        value = createTempAlloca(value);
    }
    return value;
}

llvm::Value* IRGenerator::codegenExprOrEnumTag(const Expr& expr, llvm::Value** enumValue) {
    if (auto* memberExpr = llvm::dyn_cast<MemberExpr>(&expr)) {
        if (auto* enumCase = llvm::dyn_cast_or_null<EnumCase>(memberExpr->getDecl())) {
            return codegenExpr(*enumCase->getValue());
        }
    }

    if (auto* enumDecl = llvm::dyn_cast_or_null<EnumDecl>(expr.getType().getDecl())) {
        if (enumDecl->hasAssociatedValues()) {
            auto* value = codegenLvalueExpr(expr);
            if (enumValue) *enumValue = value;
            return createLoad(builder.CreateStructGEP(value, 0, value->getName() + ".tag"));
        }
    }

    return codegenExpr(expr);
}

llvm::Value* IRGenerator::codegenAutoCast(llvm::Value* value, const Expr& expr) {
    if (value && expr.hasType()) {
        auto type = getLLVMType(expr.getType());

        if (type != value->getType()) {
            if (value->getType()->isFloatingPointTy()) {
                return builder.CreateFPCast(value, type);
            } else if (value->getType()->isIntegerTy()) {
                if (expr.getType().isFloatingPoint()) {
                    return builder.CreateSIToFP(value, type); // TODO: Handle signed vs unsigned
                } else {
                    return builder.CreateIntCast(value, type, expr.getType().isSigned());
                }
            }
        }
    }

    return value;
}
