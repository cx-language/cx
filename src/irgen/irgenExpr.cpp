#include "irgen.h"
#include "../support/utility.h"

using namespace delta;

namespace delta {
extern llvm::LLVMContext ctx;
}

llvm::Value* IRGenerator::codegenVarExpr(const VarExpr& expr) {
    auto* value = findValue(expr.getIdentifier(), expr.getDecl());

    if (llvm::isa<llvm::AllocaInst>(value) || llvm::isa<llvm::GlobalValue>(value) ||
        llvm::isa<llvm::GetElementPtrInst>(value)) {
        return builder.CreateLoad(value, expr.getIdentifier());
    } else {
        return value;
    }
}

llvm::Value* IRGenerator::codegenLvalueVarExpr(const VarExpr& expr) {
    return findValue(expr.getIdentifier(), expr.getDecl());
}

llvm::Value* IRGenerator::codegenStringLiteralExpr(const StringLiteralExpr& expr) {
    if (expr.getType().isString()) {
        ASSERT(builder.GetInsertBlock(), "CreateGlobalStringPtr requires block to insert into");
        auto* stringPtr = builder.CreateGlobalStringPtr(expr.getValue());
        auto* string = builder.CreateInsertValue(llvm::UndefValue::get(toIR(Type::getString())),
                                                 stringPtr, 0);
        auto* size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), expr.getValue().size());
        return builder.CreateInsertValue(string, size, 1);
    } else {
        // Passing as C-string, i.e. char pointer.
        return builder.CreateGlobalStringPtr(expr.getValue());
    }
}

llvm::Value* IRGenerator::codegenIntLiteralExpr(const IntLiteralExpr& expr) {
    // Integer literals may be typed as floating-point when used in a context
    // that requires a floating-point value. It might make sense to combine
    // IntLiteralExpr and FloatLiteralExpr into a single class.
    if (expr.getType().isFloatingPoint())
        return llvm::ConstantFP::get(toIR(expr.getType()), expr.getValue());

    return llvm::ConstantInt::getSigned(toIR(expr.getType()), expr.getValue());
}

llvm::Value* IRGenerator::codegenFloatLiteralExpr(const FloatLiteralExpr& expr) {
    return llvm::ConstantFP::get(toIR(expr.getType()), expr.getValue());
}

llvm::Value* IRGenerator::codegenBoolLiteralExpr(const BoolLiteralExpr& expr) {
    return expr.getValue() ? llvm::ConstantInt::getTrue(ctx) : llvm::ConstantInt::getFalse(ctx);
}

llvm::Value* IRGenerator::codegenNullLiteralExpr(const NullLiteralExpr& expr) {
    if (expr.getType().getPointee().isUnsizedArrayType()) {
        return llvm::ConstantStruct::getAnon({
            llvm::ConstantPointerNull::get(toIR(expr.getType().getPointee().getElementType())->getPointerTo()),
            llvm::ConstantInt::getSigned(llvm::Type::getInt32Ty(ctx), 0)
        });
    }
    return llvm::ConstantPointerNull::get(llvm::cast<llvm::PointerType>(toIR(expr.getType())));
}

llvm::Value* IRGenerator::codegenArrayLiteralExpr(const ArrayLiteralExpr& expr) {
    auto* arrayType = llvm::ArrayType::get(toIR(expr.getElements()[0]->getType()), expr.getElements().size());
    auto values = map(expr.getElements(), [&](const std::unique_ptr<Expr>& elementExpr) {
        return llvm::cast<llvm::Constant>(codegenExpr(*elementExpr));
    });
    return llvm::ConstantArray::get(arrayType, values);
}

llvm::Value* IRGenerator::codegenNot(const PrefixExpr& expr) {
    return builder.CreateNot(codegenExpr(expr.getOperand()), "");
}

llvm::Value* IRGenerator::codegenPrefixExpr(const PrefixExpr& expr) {
    switch (expr.getOperator()) {
        case PLUS: return codegenExpr(expr.getOperand());
        case MINUS:
            if (expr.getOperand().getType().isFloatingPoint()) {
                return builder.CreateFNeg(codegenExpr(expr.getOperand()));
            } else {
                return builder.CreateNeg(codegenExpr(expr.getOperand()));
            }
        case STAR: return builder.CreateLoad(codegenExpr(expr.getOperand()));
        case AND: return codegenLvalueExpr(expr.getOperand());
        case NOT: return codegenNot(expr);
        case COMPL: return codegenNot(expr);
        default: llvm_unreachable("invalid prefix operator");
    }
}

llvm::Value* IRGenerator::codegenLvaluePrefixExpr(const PrefixExpr& expr) {
    switch (expr.getOperator()) {
        case STAR: return codegenExpr(expr.getOperand());
        default: llvm_unreachable("invalid lvalue prefix operator");
    }
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
    auto* lhsBlock = builder.GetInsertBlock();
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "andRHS", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "andEnd", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegenExpr(left);
    builder.CreateCondBr(lhs, rhsBlock, endBlock);

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegenExpr(right);
    builder.CreateBr(endBlock);

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "and");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* IRGenerator::codegenLogicalOr(const Expr& left, const Expr& right) {
    auto* lhsBlock = builder.GetInsertBlock();
    auto* rhsBlock = llvm::BasicBlock::Create(ctx, "orRHS", builder.GetInsertBlock()->getParent());
    auto* endBlock = llvm::BasicBlock::Create(ctx, "orEnd", builder.GetInsertBlock()->getParent());

    llvm::Value* lhs = codegenExpr(left);
    builder.CreateCondBr(lhs, endBlock, rhsBlock);

    builder.SetInsertPoint(rhsBlock);
    llvm::Value* rhs = codegenExpr(right);
    builder.CreateBr(endBlock);

    builder.SetInsertPoint(endBlock);
    llvm::PHINode* phi = builder.CreatePHI(llvm::IntegerType::getInt1Ty(ctx), 2, "or");
    phi->addIncoming(lhs, lhsBlock);
    phi->addIncoming(rhs, rhsBlock);
    return phi;
}

llvm::Value* IRGenerator::codegenBinaryOp(BinaryOperator op, llvm::Value* lhs, llvm::Value* rhs, const Expr& leftExpr) {
    if (lhs->getType()->isFloatingPointTy()) {
        switch (op) {
            case EQ:    return builder.CreateFCmpOEQ(lhs, rhs);
            case NE:    return builder.CreateFCmpONE(lhs, rhs);
            case LT:    return builder.CreateFCmpOLT(lhs, rhs);
            case LE:    return builder.CreateFCmpOLE(lhs, rhs);
            case GT:    return builder.CreateFCmpOGT(lhs, rhs);
            case GE:    return builder.CreateFCmpOGE(lhs, rhs);
            case PLUS:  return builder.CreateFAdd(lhs, rhs);
            case MINUS: return builder.CreateFSub(lhs, rhs);
            case STAR:  return builder.CreateFMul(lhs, rhs);
            case SLASH: return builder.CreateFDiv(lhs, rhs);
            case MOD:   return builder.CreateFRem(lhs, rhs);
            default:    llvm_unreachable("all cases handled");
        }
    }

    switch (op) {
        case EQ:    return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpEQ);
        case NE:    return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateICmpNE);
        case LT:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSLT :
                                           &llvm::IRBuilder<>::CreateICmpULT);
        case LE:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSLE :
                                           &llvm::IRBuilder<>::CreateICmpULE);
        case GT:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSGT :
                                           &llvm::IRBuilder<>::CreateICmpUGT);
        case GE:    return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateICmpSGE :
                                           &llvm::IRBuilder<>::CreateICmpUGE);
        case PLUS:  return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateAdd);
        case MINUS: return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateSub);
        case STAR:  return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateMul);
        case SLASH: return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateSDiv :
                                           &llvm::IRBuilder<>::CreateUDiv);
        case MOD:   return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                           &llvm::IRBuilder<>::CreateSRem :
                                           &llvm::IRBuilder<>::CreateURem);
        case AND:   return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateAnd);
        case OR:    return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateOr);
        case XOR:   return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateXor);
        case LSHIFT: return codegenBinaryOp(lhs, rhs, &llvm::IRBuilder<>::CreateShl);
        case RSHIFT: return codegenBinaryOp(lhs, rhs, leftExpr.getType().isSigned() ?
                                            (BinaryCreate1) &llvm::IRBuilder<>::CreateAShr :
                                            (BinaryCreate1) &llvm::IRBuilder<>::CreateLShr);
        default:    llvm_unreachable("all cases handled");
    }
}

llvm::Value* IRGenerator::codegenShortCircuitBinaryOp(BinaryOperator op, const Expr& lhs, const Expr& rhs) {
    switch (op) {
        case AND_AND: return codegenLogicalAnd(lhs, rhs);
        case OR_OR:   return codegenLogicalOr(lhs, rhs);
        default:      llvm_unreachable("invalid short-circuit binary operator");
    }
}

llvm::Value* IRGenerator::codegenBinaryExpr(const BinaryExpr& expr) {
    if (!expr.isBuiltinOp(*this)) {
        return codegenCallExpr((const CallExpr&) expr);
    }

    Type resolvedLHSType = resolve(expr.getLHS().getType());
    Type resolvedRHSType = resolve(expr.getRHS().getType());

    ASSERT(resolvedLHSType.isImplicitlyConvertibleTo(resolvedRHSType) ||
           resolvedRHSType.isImplicitlyConvertibleTo(resolvedLHSType));

    switch (expr.getOperator()) {
        case AND_AND: case OR_OR:
            return codegenShortCircuitBinaryOp(expr.getOperator(), expr.getLHS(), expr.getRHS());
        default:
            llvm::Value* lhs = codegenExpr(expr.getLHS());
            llvm::Value* rhs = codegenExpr(expr.getRHS());
            return codegenBinaryOp(expr.getOperator(), lhs, rhs, expr.getLHS());
    }
}

bool isSizedArrayToUnsizedArrayRefConversion(Type sourceType, llvm::Type* targetType) {
    return sourceType.isPointerType() && sourceType.getPointee().isSizedArrayType()
           && targetType->isStructTy() && targetType->getStructNumElements() == 2
           && targetType->getStructElementType(0)->isPointerTy()
           && targetType->getStructElementType(1)->isIntegerTy(32);
}

llvm::Value* IRGenerator::codegenExprForPassing(const Expr& expr, llvm::Type* targetType,
                                                bool forceByReference) {
    if (targetType && isSizedArrayToUnsizedArrayRefConversion(expr.getType(), targetType)) {
        ASSERT(expr.getType().getPointee().getArraySize() != ArrayType::unsized);
        auto* elementPtr = builder.CreateConstGEP2_32(nullptr, codegenExpr(expr), 0, 0);
        auto* arrayRef = builder.CreateInsertValue(llvm::UndefValue::get(targetType),
                                                   elementPtr, 0);
        auto size = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx),
                                           expr.getType().getPointee().getArraySize());
        return builder.CreateInsertValue(arrayRef, size, 1);
    }

    Type exprType = expr.getType();
    if (exprType.isPointerType()) exprType = exprType.getPointee();

    if (expr.isRvalue() || !exprType.isBasicType())
        return codegenExpr(expr);

    auto it = structs.find(exprType.getName());
    if ((it == structs.end() || it->second.second->passByValue()) && !forceByReference) {
        if (expr.getType().isPointerType() && targetType && !targetType->isPointerTy()) {
            return builder.CreateLoad(codegenExpr(expr));
        }
    } else if (!expr.getType().isPointerType()) {
        return codegenLvalueExpr(expr);
    }
    return codegenExpr(expr);
}

llvm::Value* IRGenerator::codegenBuiltinConversion(const Expr& expr, Type type) {
    if (expr.getType().isUnsigned() && type.isInteger()) {
        return builder.CreateZExtOrTrunc(codegenExpr(expr), toIR(type));
    } else if (expr.getType().isSigned() && type.isInteger()) {
        return builder.CreateSExtOrTrunc(codegenExpr(expr), toIR(type));
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

llvm::Value* IRGenerator::codegenCallExpr(const CallExpr& expr) {
    if (expr.isBuiltinConversion())
        return codegenBuiltinConversion(*expr.getArgs().front().getValue(), expr.getType());

    if (expr.getFunctionName() == "sizeOf") {
        return llvm::ConstantExpr::getSizeOf(toIR(expr.getGenericArgs().front()));
    } else if (expr.getFunctionName() == "offsetUnsafely") {
        return codegenOffsetUnsafely(expr);
    }

    llvm::Function* function = getFunctionForCall(expr);
    ASSERT(function);
    auto param = function->arg_begin();
    llvm::SmallVector<llvm::Value*, 16> args;

    auto* calleeDecl = expr.getCalleeDecl();

    if ((calleeDecl && ((calleeDecl->isFunctionDecl() && llvm::cast<FunctionDecl>(calleeDecl)->isMethodDecl()) ||
                        calleeDecl->isDeinitDecl())) || (!calleeDecl && function->getName() == "offsetUnsafely")) {
        bool forceByReference = calleeDecl && calleeDecl->isFunctionDecl() && llvm::cast<FunctionDecl>(calleeDecl)->isMutating();

        if (expr.getReceiver()) {
            args.emplace_back(codegenExprForPassing(*expr.getReceiver(), param->getType(), forceByReference));
        } else {
            auto* thisValue = findValue("this", nullptr);
            if (thisValue->getType()->isPointerTy() && !param->getType()->isPointerTy()) {
                thisValue = builder.CreateLoad(thisValue, thisValue->getName());
            }
            args.emplace_back(thisValue);
        }
        ++param;
    }

    for (const auto& arg : expr.getArgs()) {
        auto* paramType = param != function->arg_end() ? param++->getType() : nullptr;
        args.emplace_back(codegenExprForPassing(*arg.getValue(), paramType));
    }

    return builder.CreateCall(function, args);
}

llvm::Value* IRGenerator::codegenCastExpr(const CastExpr& expr) {
    auto* value = codegenExpr(expr.getExpr());
    auto* type = toIR(expr.getTargetType());
    if (value->getType()->isIntegerTy() && type->isIntegerTy()) {
        return builder.CreateIntCast(value, type, expr.getExpr().getType().isSigned());
    }
    if (expr.getExpr().getType().isPointerType() && expr.getExpr().getType().getPointee().isUnsizedArrayType()) {
        value = builder.CreateExtractValue(value, 0);
    }
    return builder.CreateBitOrPointerCast(value, type);
}

llvm::Value* IRGenerator::codegenMemberAccess(llvm::Value* baseValue, Type memberType, llvm::StringRef memberName) {
    auto baseType = baseValue->getType();
    if (baseType->isPointerTy()) {
        baseType = baseType->getPointerElementType();
        if (baseType->isPointerTy()) {
            baseType = baseType->getPointerElementType();
            baseValue = builder.CreateLoad(baseValue);
        }
        auto& baseTypeDecl = *structs.find(baseType->getStructName())->second.second;
        auto index = baseTypeDecl.isUnion() ? 0 : baseTypeDecl.getFieldIndex(memberName);
        auto* gep = builder.CreateStructGEP(nullptr, baseValue, index);
        if (baseTypeDecl.isUnion()) {
            return builder.CreateBitCast(gep, toIR(memberType)->getPointerTo(), memberName);
        }
        return gep;
    } else {
        auto& baseTypeDecl = *structs.find(baseType->getStructName())->second.second;
        auto index = baseTypeDecl.isUnion() ? 0 : baseTypeDecl.getFieldIndex(memberName);
        return builder.CreateExtractValue(baseValue, index);
    }
}

llvm::Value* IRGenerator::getArrayOrStringDataPointer(const Expr& object, Type objectType) {
    if (objectType.isUnsizedArrayType() || objectType.isString()) {
        return builder.CreateExtractValue(codegenExpr(object), 0, "data");
    } else {
        llvm::Value* objectValue = codegenExpr(object);
        if (objectValue->getType()->isPointerTy()) {
            auto* zeroConstant = llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0);
            return builder.CreateGEP(objectValue, { zeroConstant, zeroConstant });
        } else {
            auto* alloca = createEntryBlockAlloca(objectType);
            builder.CreateStore(objectValue, alloca);
            return alloca;
        }
    }
}

llvm::Value* IRGenerator::getArrayOrStringLength(const Expr& object, Type objectType) {
    if (objectType.isUnsizedArrayType() || objectType.isString()) {
        return builder.CreateExtractValue(codegenExpr(object), 1, "count");
    } else {
        return llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), objectType.getArraySize());
    }
}

llvm::Value* IRGenerator::codegenOffsetUnsafely(const CallExpr& call) {
    auto* pointer = codegenExpr(*call.getReceiver());
    auto* offset = codegenExpr(*call.getArgs().front().getValue());
    return builder.CreateGEP(pointer, offset);
}

llvm::Value* IRGenerator::codegenLvalueMemberExpr(const MemberExpr& expr) {
    return codegenMemberAccess(codegenLvalueExpr(*expr.getBaseExpr()), expr.getType(), expr.getMemberName());
}

llvm::Value* IRGenerator::codegenMemberExpr(const MemberExpr& expr) {
    Type baseType = expr.getBaseExpr()->getType();
    if (baseType.isReference()) baseType = baseType.getPointee();
    if (baseType.isArrayType() || baseType.isString()) {
        if (expr.getMemberName() == "data") return getArrayOrStringDataPointer(*expr.getBaseExpr(), baseType);
        if (expr.getMemberName() == "count") return getArrayOrStringLength(*expr.getBaseExpr(), baseType);
    }
    auto* value = codegenLvalueMemberExpr(expr);
    return value->getType()->isPointerTy() ? builder.CreateLoad(value) : value;
}

llvm::Value* IRGenerator::codegenLvalueSubscriptExpr(const SubscriptExpr& expr) {
    auto* value = codegenLvalueExpr(*expr.getBaseExpr());
    Type lhsType = expr.getBaseExpr()->getType();

    if (lhsType.isPointerType() && lhsType.getPointee().isUnsizedArrayType()) {
        if (value->getType()->isPointerTy()) {
            value = builder.CreateLoad(value);
        }
        return builder.CreateGEP(builder.CreateExtractValue(value, 0), codegenExpr(*expr.getIndexExpr()));
    }
    if (value->getType()->getPointerElementType()->isPointerTy()) value = builder.CreateLoad(value);

    return builder.CreateGEP(value, {
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(ctx), 0),
        codegenExpr(*expr.getIndexExpr())
    });
}

llvm::Value* IRGenerator::codegenSubscriptExpr(const SubscriptExpr& expr) {
    return builder.CreateLoad(codegenLvalueSubscriptExpr(expr));
}

llvm::Value* IRGenerator::codegenUnwrapExpr(const UnwrapExpr& expr) {
    // TODO: Assert that the operand is non-null.
    return codegenExpr(expr.getOperand());
}

llvm::Value* IRGenerator::codegenExpr(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VarExpr: return codegenVarExpr(llvm::cast<VarExpr>(expr));
        case ExprKind::StringLiteralExpr: return codegenStringLiteralExpr(llvm::cast<StringLiteralExpr>(expr));
        case ExprKind::IntLiteralExpr: return codegenIntLiteralExpr(llvm::cast<IntLiteralExpr>(expr));
        case ExprKind::FloatLiteralExpr: return codegenFloatLiteralExpr(llvm::cast<FloatLiteralExpr>(expr));
        case ExprKind::BoolLiteralExpr: return codegenBoolLiteralExpr(llvm::cast<BoolLiteralExpr>(expr));
        case ExprKind::NullLiteralExpr: return codegenNullLiteralExpr(llvm::cast<NullLiteralExpr>(expr));
        case ExprKind::ArrayLiteralExpr: return codegenArrayLiteralExpr(llvm::cast<ArrayLiteralExpr>(expr));
        case ExprKind::PrefixExpr: return codegenPrefixExpr(llvm::cast<PrefixExpr>(expr));
        case ExprKind::BinaryExpr: return codegenBinaryExpr(llvm::cast<BinaryExpr>(expr));
        case ExprKind::CallExpr: return codegenCallExpr(llvm::cast<CallExpr>(expr));
        case ExprKind::CastExpr: return codegenCastExpr(llvm::cast<CastExpr>(expr));
        case ExprKind::MemberExpr: return codegenMemberExpr(llvm::cast<MemberExpr>(expr));
        case ExprKind::SubscriptExpr: return codegenSubscriptExpr(llvm::cast<SubscriptExpr>(expr));
        case ExprKind::UnwrapExpr: return codegenUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
    }
    llvm_unreachable("all cases handled");
}

llvm::Value* IRGenerator::codegenLvalueExpr(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VarExpr: return codegenLvalueVarExpr(llvm::cast<VarExpr>(expr));
        case ExprKind::StringLiteralExpr: llvm_unreachable("no lvalue string literals");
        case ExprKind::IntLiteralExpr: llvm_unreachable("no lvalue integer literals");
        case ExprKind::FloatLiteralExpr: llvm_unreachable("no lvalue float literals");
        case ExprKind::BoolLiteralExpr: llvm_unreachable("no lvalue boolean literals");
        case ExprKind::NullLiteralExpr: llvm_unreachable("no lvalue null literals");
        case ExprKind::ArrayLiteralExpr: llvm_unreachable("no lvalue array literals");
        case ExprKind::PrefixExpr: return codegenLvaluePrefixExpr(llvm::cast<PrefixExpr>(expr));
        case ExprKind::BinaryExpr: llvm_unreachable("no lvalue binary expressions");
        case ExprKind::CallExpr: llvm_unreachable("IRGen doesn't support lvalue call expressions yet");
        case ExprKind::CastExpr: llvm_unreachable("IRGen doesn't support lvalue cast expressions yet");
        case ExprKind::MemberExpr: return codegenLvalueMemberExpr(llvm::cast<MemberExpr>(expr));
        case ExprKind::SubscriptExpr: return codegenLvalueSubscriptExpr(llvm::cast<SubscriptExpr>(expr));
        case ExprKind::UnwrapExpr: return codegenUnwrapExpr(llvm::cast<UnwrapExpr>(expr));
    }
    llvm_unreachable("all cases handled");
}
