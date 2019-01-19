#include "irgen.h"
#include <llvm/Support/Path.h>
#include "../support/utility.h"

using namespace delta;

llvm::Value* IRGenerator::codegenVarExpr(const VarExpr& expr) {
    auto* value = findValue(expr.getIdentifier(), expr.getDecl());

    if (llvm::isa<llvm::AllocaInst>(value) || llvm::isa<llvm::GetElementPtrInst>(value) ||
        (llvm::isa<llvm::GlobalValue>(value) && !llvm::isa<llvm::Function>(value))) {
        value = load(value);
    }

    if (value->getType()->isFloatingPointTy()) {
        value = builder.CreateFPCast(value, toIR(expr.getType()));
    } else if (value->getType()->isIntegerTy()) {
        value = builder.CreateIntCast(value, toIR(expr.getType()), expr.getType().isSigned());
    }

    return value;
}

llvm::Value* IRGenerator::codegenLvalueVarExpr(const VarExpr& expr) {
    return findValue(expr.getIdentifier(), expr.getDecl());
}

llvm::Value* IRGenerator::codegenStringLiteralExpr(const StringLiteralExpr& expr) {
    if (expr.getType().isBasicType() && expr.getType().getName() == "StringRef") {
        ASSERT(builder.GetInsertBlock(), "CreateGlobalStringPtr requires block to insert into");
        auto* stringPtr = builder.CreateGlobalStringPtr(expr.getValue());
        auto* size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), expr.getValue().size());
        static int stringLiteralCounter = 0;
        auto* alloca = createEntryBlockAlloca(BasicType::get("StringRef", {}), structs.find("StringRef")->second.second, nullptr,
                                              "__str" + std::to_string(stringLiteralCounter++));
        // TODO: Retrieve this constructor in a nicer way.
        auto* stringRefInit = module->getOrInsertFunction("_ENM3std9StringRef4initE7pointerP4char6length3int", llvm::Type::getVoidTy(ctx),
                                                          alloca->getType(), stringPtr->getType(), size->getType());
        builder.CreateCall(stringRefInit, { alloca, stringPtr, size });
        return alloca;
    } else {
        // Passing as C-string, i.e. char pointer.
        ASSERT(expr.getType().removeOptional().isPointerType() && expr.getType().removeOptional().getPointee().isChar());
        return builder.CreateGlobalStringPtr(expr.getValue());
    }
}

llvm::Value* IRGenerator::codegenCharacterLiteralExpr(const CharacterLiteralExpr& expr) {
    return llvm::ConstantInt::get(toIR(expr.getType()), expr.getValue());
}

llvm::Value* IRGenerator::codegenIntLiteralExpr(const IntLiteralExpr& expr) {
    auto type = toIR(expr.getType());
    // Integer literals may be typed as floating-point when used in a context
    // that requires a floating-point value. It might make sense to combine
    // IntLiteralExpr and FloatLiteralExpr into a single class.
    if (expr.getType().isFloatingPoint()) {
        return llvm::ConstantFP::get(type, expr.getValue().roundToDouble());
    }
    return llvm::ConstantInt::get(type, expr.getValue().extOrTrunc(type->getIntegerBitWidth()));
}

llvm::Value* IRGenerator::codegenFloatLiteralExpr(const FloatLiteralExpr& expr) {
    return llvm::ConstantFP::get(toIR(expr.getType()), expr.getValue());
}

llvm::Value* IRGenerator::codegenBoolLiteralExpr(const BoolLiteralExpr& expr) {
    return expr.getValue() ? llvm::ConstantInt::getTrue(ctx) : llvm::ConstantInt::getFalse(ctx);
}

llvm::Value* IRGenerator::codegenNullLiteralExpr(const NullLiteralExpr& expr) {
    auto pointeeType = expr.getType().getWrappedType().getPointee();

    if (pointeeType.isArrayWithUnknownSize()) {
        auto* pointerType = toIR(pointeeType.getElementType())->getPointerTo();
        return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(pointerType));
    }

    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(toIR(expr.getType())));
}

llvm::Value* IRGenerator::codegenUndefinedLiteralExpr(const UndefinedLiteralExpr& expr) {
    return llvm::UndefValue::get(toIR(expr.getType()));
}

llvm::Value* IRGenerator::codegenArrayLiteralExpr(const ArrayLiteralExpr& expr) {
    auto* arrayType = llvm::ArrayType::get(toIR(expr.getElements()[0]->getType()), expr.getElements().size());
    llvm::Value* array = llvm::UndefValue::get(arrayType);
    unsigned index = 0;
    for (auto& element : expr.getElements()) {
        array = builder.CreateInsertValue(array, loadIfAlloca(codegenExpr(*element)), index++);
    }
    return array;
}

llvm::Value* IRGenerator::codegenTupleExpr(const TupleExpr& expr) {
    llvm::Value* tuple = llvm::UndefValue::get(toIR(expr.getType()));
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
        case Token::Minus:
            if (expr.getOperand().getType().isFloatingPoint()) {
                return builder.CreateFPCast(builder.CreateFNeg(codegenExpr(expr.getOperand())), toIR(expr.getType()));
            } else {
                return builder.CreateIntCast(builder.CreateNeg(codegenExpr(expr.getOperand())), toIR(expr.getType()), expr.getType().isSigned());
            }
        case Token::Star:
            return load(codegenExpr(expr.getOperand()));
        case Token::And:
            return codegenLvalueExpr(expr.getOperand());
        case Token::Not:
            return codegenNot(expr);
        case Token::Tilde:
            return codegenNot(expr);
        case Token::Increment:
            return codegenIncrementExpr(expr);
        case Token::Decrement:
            return codegenDecrementExpr(expr);
        default:
            llvm_unreachable("invalid prefix operator");
    }
}

llvm::Value* IRGenerator::codegenLvalueUnaryExpr(const UnaryExpr& expr) {
    switch (expr.getOperator()) {
        case Token::Star:
            return codegenExpr(expr.getOperand());
        case Token::And:
            return codegenLvalueExpr(expr.getOperand());
        default:
            llvm_unreachable("invalid lvalue prefix operator");
    }
}

llvm::Value* IRGenerator::codegenIncrementExpr(const UnaryExpr& expr) {
    auto* alloca = codegenLvalueExpr(expr.getOperand());
    auto* value = load(alloca);
    llvm::Value* result;

    if (expr.getOperand().getType().isPointerType()) {
        result = builder.CreateConstGEP1_32(value, 1);
    } else {
        result = builder.CreateAdd(value, llvm::ConstantInt::get(value->getType(), 1));
    }

    builder.CreateStore(result, alloca);
    return nullptr;
}

llvm::Value* IRGenerator::codegenDecrementExpr(const UnaryExpr& expr) {
    auto* alloca = codegenLvalueExpr(expr.getOperand());
    auto* value = load(alloca);
    llvm::Value* result;

    if (expr.getOperand().getType().isPointerType()) {
        auto* minusOne = llvm::ConstantInt::getSigned(llvm::IntegerType::getInt32Ty(ctx), -1);
        result = builder.CreateGEP(value, minusOne);
    } else {
        result = builder.CreateSub(value, llvm::ConstantInt::get(value->getType(), 1));
    }

    builder.CreateStore(result, alloca);
    return nullptr;
}

llvm::Value* IRGenerator::codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate0 create) {
    return (builder.*create)(lhs, rhs, "");
}

llvm::Value* IRGenerator::codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate1 create) {
    return (builder.*create)(lhs, rhs, "", false);
}

llvm::Value* IRGenerator::codegenBinaryOp(llvm::Value* lhs, llvm::Value* rhs, BinaryCreate2 create) {
    return (builder.*create)(lhs, rhs, "", false, false);
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
    auto* lhsBlock = builder.GetInsertBlock();
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "or.rhs", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "or.end", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegenExpr(left);
    builder.CreateCondBr(lhs, endBlock, rhsBlock);
    lhsBlock = builder.GetInsertBlock();

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
        lhs = load(lhs);
        lhsType = lhsType.getPointee();
    } else if (rhs->getType()->isPointerTy() && rhs->getType()->getPointerElementType() == lhs->getType()) {
        rhs = load(rhs);
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
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpEQ);
        case Token::NotEqual:
        case Token::PointerNotEqual:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpNE);
        case Token::Less:
            if (lhsType.isSigned()) {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpSLT);
            } else {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpULT);
            }
        case Token::LessOrEqual:
            if (lhsType.isSigned()) {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpSLE);
            } else {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpULE);
            }
        case Token::Greater:
            if (lhsType.isSigned()) {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpSGT);
            } else {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpUGT);
            }
        case Token::GreaterOrEqual:
            if (lhsType.isSigned()) {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpSGE);
            } else {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpUGE);
            }
        case Token::Plus:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateAdd);
        case Token::Minus:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateSub);
        case Token::Star:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateMul);
        case Token::Slash:
            if (lhsType.isSigned()) {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateSDiv);
            } else {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateUDiv);
            }
        case Token::Modulo:
            if (lhsType.isSigned()) {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateSRem);
            } else {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateURem);
            }
        case Token::And:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateAnd);
        case Token::Or:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateOr);
        case Token::Xor:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateXor);
        case Token::LeftShift:
            return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateShl);
        case Token::RightShift:
            if (lhsType.isSigned()) {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateAShr);
            } else {
                return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateLShr);
            }
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

    if (expr.isPointerOffset()) {
        return codegenPointerOffset(expr);
    }

    switch (expr.getOperator()) {
        case Token::AndAnd:
        case Token::OrOr:
            return codegenShortCircuitBinaryOp(expr.getOperator(), expr.getLHS(), expr.getRHS());
        default:
            llvm::Value* lhs = codegenExpr(expr.getLHS());
            llvm::Value* rhs = codegenExpr(expr.getRHS());
            return codegenBinaryOp(expr.getOperator(), lhs, rhs, expr.getLHS().getType());
    }
}

void IRGenerator::codegenAssignment(const BinaryExpr& expr) {
    llvm::Value* lhsLvalue = codegenAssignmentLHS(&expr.getLHS(), &expr.getRHS());
    if (!lhsLvalue) return;
    llvm::Value* rhsValue;

    if (isCompoundAssignmentOperator(expr.getOperator())) {
        auto nonCompoundOp = withoutCompoundEqSuffix(expr.getOperator());

        if (!isBuiltinOp(nonCompoundOp, expr.getLHS().getType(), expr.getRHS().getType())) {
            builder.CreateStore(codegenCallExpr(expr), lhsLvalue);
            return;
        }

        switch (nonCompoundOp) {
            case Token::AndAnd:
                error(expr.getLocation(), "'&&=' not implemented yet");
            case Token::OrOr:
                error(expr.getLocation(), "'||=' not implemented yet");
            default:
                break;
        }

        if (expr.getLHS().getType().isPointerType() && expr.getRHS().getType().isInteger()) {
            rhsValue = codegenPointerOffset(expr);
        } else {
            auto* lhsValue = load(lhsLvalue);
            rhsValue = codegenBinaryOp(nonCompoundOp, lhsValue, codegenExpr(expr.getRHS()), expr.getLHS().getType());
        }
    } else {
        rhsValue = codegenExprForPassing(expr.getRHS(), lhsLvalue->getType()->getPointerElementType());
    }

    builder.CreateStore(rhsValue, lhsLvalue);
}

bool isBuiltinArrayToArrayRefConversion(Type sourceType, llvm::Type* targetType) {
    return sourceType.removePointer().isArrayWithConstantSize() && targetType->isStructTy() &&
           targetType->getStructName().startswith("ArrayRef");
}

llvm::Value* IRGenerator::codegenExprForPassing(const Expr& expr, llvm::Type* targetType) {
    if (targetType && isBuiltinArrayToArrayRefConversion(expr.getType(), targetType)) {
        ASSERT(expr.getType().removePointer().isArrayWithConstantSize());
        auto* value = codegenLvalueExpr(expr);

        if (!value->getType()->isPointerTy()) {
            static int temporaryArrayCounter = 0;
            auto name = "__temporaryArray" + std::to_string(temporaryArrayCounter++);
            auto* alloca = createEntryBlockAlloca(expr.getType().removePointer(), nullptr, nullptr, name);
            builder.CreateStore(value, alloca);
            value = alloca;
        }

        auto* elementPtr = builder.CreateConstGEP2_32(nullptr, value, 0, 0);
        auto* arrayRef = builder.CreateInsertValue(llvm::UndefValue::get(targetType), elementPtr, 0);
        auto size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), expr.getType().removePointer().getArraySize());
        return builder.CreateInsertValue(arrayRef, size, 1);
    }

    // Handle implicit conversions to type 'T[?]*'.
    if (targetType && expr.getType().removePointer().isArrayWithConstantSize() && targetType->isPointerTy() &&
        !targetType->getPointerElementType()->isArrayTy()) {
        return builder.CreateBitOrPointerCast(codegenLvalueExpr(expr), targetType);
    }

    // Handle implicit conversions to void pointer.
    if (targetType && expr.getType().isPointerType() && !expr.getType().getPointee().isVoid() && targetType->isPointerTy() &&
        targetType->getPointerElementType()->isIntegerTy(8)) {
        return builder.CreateBitCast(codegenExpr(expr), targetType);
    }

    Type exprType = expr.getType();
    if (exprType.isPointerType()) exprType = exprType.getPointee();

    if (!targetType || expr.isRvalue() || (!exprType.isBasicType() && !exprType.isTupleType())) {
        if (expr.getType().isArrayWithConstantSize() && targetType->isPointerTy()) {
            return codegenLvalueExpr(expr);
        }

        auto* value = codegenExpr(expr);

        if (targetType && targetType->isPointerTy() && !value->getType()->isPointerTy()) {
            auto* alloca = createEntryBlockAlloca(expr.getType(), nullptr);
            builder.CreateStore(value, alloca);
            return alloca;
        } else if (targetType && value->getType()->isPointerTy() && !targetType->isPointerTy()) {
            return load(value);
        } else {
            return value;
        }
    }

    if (!targetType->isPointerTy()) {
        if (expr.getType().isPointerType()) {
            return load(codegenExpr(expr));
        }
    } else if (!expr.getType().isPointerType()) {
        auto* value = codegenLvalueExpr(expr);

        if (value->getType()->isPointerTy()) {
            return value;
        } else {
            auto* alloca = createEntryBlockAlloca(expr.getType(), nullptr);
            builder.CreateStore(value, alloca);
            return alloca;
        }
    }

    auto* value = codegenExpr(expr);
    if (value->getType()->isPointerTy() && value->getType()->getPointerElementType() == targetType) {
        value = load(value);
    }
    return value;
}

llvm::Value* IRGenerator::codegenBuiltinConversion(const Expr& expr, Type type) {
    if (expr.getType().isUnsigned() && type.isInteger()) {
        return builder.CreateZExtOrTrunc(codegenExpr(expr), toIR(type));
    } else if (expr.getType().isSigned() && type.isInteger()) {
        return builder.CreateSExtOrTrunc(codegenExpr(expr), toIR(type));
    } else if ((expr.getType().isInteger() || expr.getType().isChar() || expr.getType().isBool()) && (type.isInteger() || type.isChar())) {
        return builder.CreateIntCast(codegenExpr(expr), toIR(type), expr.getType().isSigned());
    } else if (expr.getType().isFloatingPoint()) {
        if (type.isSigned()) return builder.CreateFPToSI(codegenExpr(expr), toIR(type));
        if (type.isUnsigned()) return builder.CreateFPToUI(codegenExpr(expr), toIR(type));
        if (type.isFloatingPoint()) return builder.CreateFPCast(codegenExpr(expr), toIR(type));
    } else if (type.isFloatingPoint()) {
        if (expr.getType().isSigned()) return builder.CreateSIToFP(codegenExpr(expr), toIR(type));
        if (expr.getType().isUnsigned()) return builder.CreateUIToFP(codegenExpr(expr), toIR(type));
    }
    error(expr.getLocation(), "conversion from '", expr.getType(), "' to '", type, "' not supported");
}

void IRGenerator::codegenAssert(llvm::Value* condition, SourceLocation location, llvm::StringRef message) {
    condition = builder.CreateIsNull(condition, "assert.condition");
    auto* function = builder.GetInsertBlock()->getParent();
    auto* failBlock = llvm::BasicBlock::Create(ctx, "assert.fail", function);
    auto* successBlock = llvm::BasicBlock::Create(ctx, "assert.success", function);
    auto* puts = module->getOrInsertFunction("puts", llvm::Type::getInt32Ty(ctx), llvm::Type::getInt8PtrTy(ctx));
    builder.CreateCondBr(condition, failBlock, successBlock);
    builder.SetInsertPoint(failBlock);
    auto messageAndLocation = llvm::join_items("", message, " at ", llvm::sys::path::filename(location.file), ":",
                                               std::to_string(location.line), ":", std::to_string(location.column));
    builder.CreateCall(puts, builder.CreateGlobalStringPtr(messageAndLocation));
    builder.CreateCall(module->getOrInsertFunction("abort", llvm::Type::getVoidTy(ctx)));
    builder.CreateUnreachable();
    builder.SetInsertPoint(successBlock);
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

    if (expr.getReceiver() && expr.getReceiverType().removePointer().isArrayType()) {
        if (expr.getFunctionName() == "size") {
            return getArrayLength(*expr.getReceiver(), expr.getReceiverType().removePointer());
        }
        llvm_unreachable("unknown static array member function");
    }

    if (expr.isMoveInit()) {
        auto* receiverValue = codegenExpr(*expr.getReceiver());
        auto* argumentValue = codegenExpr(*expr.getArgs()[0].getValue());
        builder.CreateStore(argumentValue, receiverValue);
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
            calleeValue = load(calleeValue);
        }
        functionType = llvm::cast<llvm::FunctionType>(calleeValue->getType()->getPointerElementType());
    }

    auto param = functionType->param_begin();
    auto paramEnd = functionType->param_end();

    llvm::SmallVector<llvm::Value*, 16> args;

    auto* calleeDecl = expr.getCalleeDecl();

    if (calleeDecl && calleeDecl->isMethodDecl()) {
        if (auto* initDecl = llvm::dyn_cast<InitDecl>(calleeDecl)) {
            if (thisAllocaForInit) {
                args.emplace_back(thisAllocaForInit);
            } else if (currentDecl->isInitDecl() && expr.getFunctionName() == "init") {
                args.emplace_back(findValue("this", nullptr));
            } else {
                Type type = initDecl->getTypeDecl()->getType();
                auto* alloca = createEntryBlockAlloca(type, nullptr);
                args.emplace_back(alloca);
            }
        } else if (expr.getReceiver()) {
            auto* thisValue = codegenExprForPassing(*expr.getReceiver(), *param);
            args.emplace_back(thisValue);
        } else {
            auto* thisValue = findValue("this", nullptr);
            if (thisValue->getType()->isPointerTy() && !(*param)->isPointerTy()) {
                thisValue = load(thisValue);
            }
            args.emplace_back(thisValue);
        }
        ++param;
    }

    for (const auto& arg : expr.getArgs()) {
        auto* paramType = param != paramEnd ? *param++ : nullptr;
        args.emplace_back(codegenExprForPassing(*arg.getValue(), paramType));
    }

    if (calleeDecl->isInitDecl()) {
        builder.CreateCall(calleeValue, args);
        return args[0];
    } else {
        return builder.CreateCall(calleeValue, args);
    }
}

llvm::Value* IRGenerator::codegenBuiltinCast(const CallExpr& expr) {
    auto* value = codegenExpr(*expr.getArgs().front().getValue());
    auto* type = toIR(expr.getGenericArgs().front());

    if (value->getType()->isIntegerTy() && type->isIntegerTy()) {
        return builder.CreateIntCast(value, type, expr.getArgs().front().getValue()->getType().isSigned());
    }

    return builder.CreateBitOrPointerCast(value, type);
}

llvm::Value* IRGenerator::codegenSizeofExpr(const SizeofExpr& expr) {
    return llvm::ConstantExpr::getSizeOf(toIR(expr.getType()));
}

llvm::Value* IRGenerator::codegenAddressofExpr(const AddressofExpr& expr) {
    llvm::Value* value = codegenExpr(expr.getOperand());
    llvm::Type* uintptr = toIR(Type::getUIntPtr());
    return builder.CreatePtrToInt(value, uintptr, "address");
}

llvm::Value* IRGenerator::codegenMemberAccess(llvm::Value* baseValue, Type memberType, llvm::StringRef memberName) {
    auto baseType = baseValue->getType();
    if (baseType->isPointerTy()) {
        baseType = baseType->getPointerElementType();
        if (baseType->isPointerTy()) {
            baseType = baseType->getPointerElementType();
            baseValue = load(baseValue);
        }
        auto& baseTypeDecl = *structs.find(baseType->getStructName())->second.second;

        if (baseTypeDecl.isUnion()) {
            return builder.CreateBitCast(baseValue, toIR(memberType)->getPointerTo(), memberName);
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
    if (objectType.isArrayWithRuntimeSize() || objectType.isString()) {
        return builder.CreateExtractValue(codegenExpr(object), 1, "size");
    } else {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), objectType.getArraySize());
    }
}

llvm::Value* IRGenerator::codegenPointerOffset(const BinaryExpr& expr) {
    auto* pointer = codegenExpr(expr.getLHS());
    auto* offset = codegenExpr(expr.getRHS());

    if (expr.getOperator() == Token::Minus || expr.getOperator() == Token::MinusEqual) {
        offset = builder.CreateNeg(offset);
    }

    return builder.CreateGEP(pointer, offset);
}

llvm::Value* IRGenerator::codegenLvalueMemberExpr(const MemberExpr& expr) {
    if (expr.getBaseExpr()->getType().removePointer().isTupleType()) {
        return codegenTupleElementAccess(expr);
    }

    return codegenMemberAccess(codegenLvalueExpr(*expr.getBaseExpr()), expr.getType(), expr.getMemberName());
}

llvm::Value* IRGenerator::codegenMemberExpr(const MemberExpr& expr) {
    if (auto* basicType = llvm::dyn_cast<BasicType>(expr.getType().getBase())) {
        if (basicType->getDecl()) {
            if (auto* enumDecl = llvm::dyn_cast<EnumDecl>(basicType->getDecl())) {
                return codegenExpr(*enumDecl->getCaseByName(expr.getMemberName())->getValue());
            }
        }
    }

    auto* value = codegenLvalueMemberExpr(expr);

    if (value->getType()->isPointerTy() && value->getType()->getPointerElementType() == toIR(expr.getType())) {
        value = load(value);
    }

    return value;
}

llvm::Value* IRGenerator::codegenTupleElementAccess(const MemberExpr& expr) {
    unsigned index = 0;
    for (auto& element : expr.getBaseExpr()->getType().removePointer().getTupleElements()) {
        if (element.name == expr.getMemberName()) break;
        ++index;
    }

    auto* baseValue = codegenLvalueExpr(*expr.getBaseExpr());
    if (baseValue->getType()->isPointerTy() && baseValue->getType()->getPointerElementType()->isPointerTy()) {
        baseValue = load(baseValue);
    }

    if (baseValue->getType()->isPointerTy()) {
        return builder.CreateStructGEP(nullptr, baseValue, index);
    } else {
        return builder.CreateExtractValue(baseValue, index);
    }
}

llvm::Value* IRGenerator::codegenLvalueSubscriptExpr(const SubscriptExpr& expr) {
    if (!expr.getBaseExpr()->getType().removePointer().isArrayType()) {
        return codegenCallExpr(expr);
    }

    auto* value = codegenLvalueExpr(*expr.getBaseExpr());
    Type lhsType = expr.getBaseExpr()->getType();

    if (lhsType.isPointerType() && lhsType.getPointee().isArrayWithRuntimeSize()) {
        if (value->getType()->isPointerTy()) {
            value = load(value);
        }
        auto* ptr = builder.CreateExtractValue(value, 0);
        auto* index = codegenExpr(*expr.getIndexExpr());
        return builder.CreateGEP(ptr, index);
    }

    if (value->getType()->getPointerElementType()->isPointerTy()) value = load(value);

    if (lhsType.isPointerType() && lhsType.getPointee().isArrayWithUnknownSize()) {
        return builder.CreateGEP(value, codegenExpr(*expr.getIndexExpr()));
    }

    return builder.CreateGEP(value, { llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0), codegenExpr(*expr.getIndexExpr()) });
}

llvm::Value* IRGenerator::codegenSubscriptExpr(const SubscriptExpr& expr) {
    if (!expr.getBaseExpr()->getType().removePointer().isArrayType()) {
        return codegenCallExpr(expr);
    }

    return load(codegenLvalueSubscriptExpr(expr));
}

llvm::Value* IRGenerator::codegenUnwrapExpr(const UnwrapExpr& expr) {
    auto* value = codegenExpr(expr.getOperand());
    codegenAssert(value, expr.getLocation(), "Unwrap failed");
    return value;
}

llvm::Value* IRGenerator::codegenLambdaExpr(const LambdaExpr& expr) {
    auto functionDecl = expr.lower(*currentDecl->getModule());

    auto insertBlockBackup = builder.GetInsertBlock();
    auto insertPointBackup = builder.GetInsertPoint();
    auto scopesBackup = std::move(scopes);

    codegenFunctionDecl(*functionDecl);

    scopes = std::move(scopesBackup);
    if (insertBlockBackup) builder.SetInsertPoint(insertBlockBackup, insertPointBackup);

    VarExpr varExpr(functionDecl->getName(), functionDecl->getLocation());
    varExpr.setDecl(functionDecl.get());
    helperDecls.push_back(std::move(functionDecl));
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

llvm::Value* IRGenerator::codegenExpr(const Expr& expr) {
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
        case ExprKind::SubscriptExpr:
            return codegenSubscriptExpr(llvm::cast<SubscriptExpr>(expr));
        case ExprKind::UnwrapExpr:
            return codegenUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
        case ExprKind::LambdaExpr:
            return codegenLambdaExpr(llvm::cast<LambdaExpr>(expr));
        case ExprKind::IfExpr:
            return codegenIfExpr(llvm::cast<IfExpr>(expr));
    }
    llvm_unreachable("all cases handled");
}

llvm::Value* IRGenerator::codegenLvalueExpr(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VarExpr:
            return codegenLvalueVarExpr(llvm::cast<VarExpr>(expr));
        case ExprKind::StringLiteralExpr:
            return codegenStringLiteralExpr(llvm::cast<StringLiteralExpr>(expr));
        case ExprKind::CharacterLiteralExpr:
            return codegenCharacterLiteralExpr(llvm::cast<CharacterLiteralExpr>(expr));
        case ExprKind::IntLiteralExpr:
            llvm_unreachable("no lvalue integer literals");
        case ExprKind::FloatLiteralExpr:
            llvm_unreachable("no lvalue float literals");
        case ExprKind::BoolLiteralExpr:
            llvm_unreachable("no lvalue boolean literals");
        case ExprKind::NullLiteralExpr:
            llvm_unreachable("no lvalue null literals");
        case ExprKind::UndefinedLiteralExpr:
            llvm_unreachable("no lvalue undefined literals");
        case ExprKind::ArrayLiteralExpr:
            return codegenArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr));
        case ExprKind::TupleExpr:
            return codegenTupleExpr(llvm::cast<TupleExpr>(expr));
        case ExprKind::UnaryExpr:
            return codegenLvalueUnaryExpr(llvm::cast<UnaryExpr>(expr));
        case ExprKind::BinaryExpr:
            llvm_unreachable("no lvalue binary expressions");
        case ExprKind::CallExpr:
            return codegenCallExpr(llvm::cast<CallExpr>(expr));
        case ExprKind::SizeofExpr:
            llvm_unreachable("no lvalue sizeof expressions");
        case ExprKind::AddressofExpr:
            llvm_unreachable("no lvalue addressof expressions");
        case ExprKind::MemberExpr:
            return codegenLvalueMemberExpr(llvm::cast<MemberExpr>(expr));
        case ExprKind::SubscriptExpr:
            return codegenLvalueSubscriptExpr(llvm::cast<SubscriptExpr>(expr));
        case ExprKind::UnwrapExpr:
            return codegenUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
        case ExprKind::LambdaExpr:
            return codegenLambdaExpr(llvm::cast<LambdaExpr>(expr));
        case ExprKind::IfExpr:
            return codegenIfExpr(llvm::cast<IfExpr>(expr));
    }
    llvm_unreachable("all cases handled");
}
