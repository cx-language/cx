#include "expr.h"
#pragma warning(push, 0)
#include <llvm/Support/ErrorHandling.h>
#pragma warning(pop)
#include "arena.h"
#include "ast.h"
#include "decl.h"
#include "token.h"

using namespace cx;

bool Expr::isAssignment() const {
    auto* binaryExpr = llvm::dyn_cast<BinaryExpr>(this);
    return binaryExpr && isAssignmentOperator(binaryExpr->op);
}

bool Expr::isReferenceExpr() const {
    auto* unaryExpr = llvm::dyn_cast<UnaryExpr>(this);
    return unaryExpr && unaryExpr->op == Token::And;
}

bool Expr::isConstant() const {
    switch (kind) {
    case ExprKind::VarExpr: {
        auto* decl = llvm::cast<VarExpr>(this)->decl;

        if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
            if (!varDecl->type.isMutable() && varDecl->initializer) {
                return varDecl->initializer->isConstant();
            }
        }

        return false;
    }
    case ExprKind::MemberExpr: {
        auto* decl = llvm::cast<MemberExpr>(this)->decl;
        auto* varDecl = decl ? llvm::dyn_cast<VarDecl>(decl) : nullptr;

        if (varDecl) {
            if (!varDecl->type.isMutable() && varDecl->initializer) {
                return varDecl->initializer->isConstant();
            }
        }

        return false;
    }

    case ExprKind::StringLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
    case ExprKind::IntLiteralExpr:
    case ExprKind::FloatLiteralExpr:
    case ExprKind::BoolLiteralExpr:
    case ExprKind::NullLiteralExpr:
        return true;

    case ExprKind::ArrayLiteralExpr:
        for (auto& element : llvm::cast<ArrayLiteralExpr>(this)->elements) {
            if (!element->isConstant()) {
                return false;
            }
        }
        return true;

    case ExprKind::AnonymousStructExpr:
        for (auto& element : llvm::cast<AnonymousStructExpr>(this)->elements) {
            if (!element.value->isConstant()) {
                return false;
            }
        }
        return true;

    case ExprKind::UnaryExpr: {
        auto unaryExpr = llvm::cast<UnaryExpr>(this);
        switch (unaryExpr->op) {
        case Token::Plus:
        case Token::Minus:
        case Token::Tilde:
            return unaryExpr->getOperand().isConstant();
        default:
            return false;
        }
    }
    case ExprKind::BinaryExpr: {
        auto binaryExpr = llvm::cast<BinaryExpr>(this);
        // Like IfExpr, `??` always emits branches and is never folded.
        if (binaryExpr->op == Token::QuestionQuestion) return false;
        // Lowered anonymous struct comparisons keep anonymous struct operands, which the getConstant* accessors can't evaluate.
        if (binaryExpr->anonymousStructComparisonLowering) return false;
        return binaryExpr->op != Token::Assignment && binaryExpr->getLHS().isConstant() && binaryExpr->getRHS().isConstant();
    }

    case ExprKind::SizeofExpr:
        return llvm::cast<SizeofExpr>(this)->operandType.getSizeInBytes().has_value();

    case ExprKind::CallExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr:
    case ExprKind::UnwrapExpr:
    case ExprKind::LambdaExpr:
    case ExprKind::UndefinedLiteralExpr:
    case ExprKind::VarDeclExpr:
        return false;

    case ExprKind::ImplicitCastExpr: {
        // User conversions run arbitrary code; every other cast preserves its operand's value.
        auto* cast = llvm::cast<ImplicitCastExpr>(this);
        return cast->castKind != ImplicitCastExpr::UserConversion && cast->operand->isConstant();
    }

    case ExprKind::IfExpr:
        return llvm::cast<IfExpr>(this)->condition->isConstant() && llvm::cast<IfExpr>(this)->thenExpr->isConstant()
            && llvm::cast<IfExpr>(this)->elseExpr->isConstant();
    case ExprKind::SwitchExpr:
        // Not folded even when constant; codegen always emits branches.
        return false;
    }

    llvm_unreachable("all cases handled");
}

bool Expr::isFoldableIntConstant() const {
    switch (kind) {
    case ExprKind::VarExpr: {
        // Unresolved references (e.g. array bounds examined during parsing) are never constant.
        auto* decl = llvm::cast<VarExpr>(this)->decl;
        auto* varDecl = decl ? llvm::dyn_cast<VarDecl>(decl) : nullptr;
        return varDecl && !varDecl->type.isMutable() && varDecl->initializer && varDecl->initializer->isFoldableIntConstant();
    }
    case ExprKind::MemberExpr: {
        auto* decl = llvm::cast<MemberExpr>(this)->decl;
        auto* varDecl = decl ? llvm::dyn_cast<VarDecl>(decl) : nullptr;
        return varDecl && !varDecl->type.isMutable() && varDecl->initializer && varDecl->initializer->isFoldableIntConstant();
    }
    case ExprKind::IntLiteralExpr:
    case ExprKind::CharacterLiteralExpr:
        return true;
    case ExprKind::UnaryExpr: {
        auto* unaryExpr = llvm::cast<UnaryExpr>(this);
        switch (unaryExpr->op) {
        case Token::Plus:
        case Token::Minus:
        case Token::Tilde:
            return unaryExpr->getOperand().isFoldableIntConstant();
        default:
            return false;
        }
    }
    case ExprKind::BinaryExpr: {
        auto& binaryExpr = llvm::cast<BinaryExpr>(*this);
        switch (binaryExpr.op) {
        case Token::Plus:
        case Token::Minus:
        case Token::Star:
        case Token::Slash:
        case Token::Modulo:
        case Token::PositiveModulo:
        case Token::And:
        case Token::Or:
        case Token::Xor:
        case Token::LeftShift:
        case Token::RightShift:
            return binaryExpr.getLHS().isFoldableIntConstant() && binaryExpr.getRHS().isFoldableIntConstant();
        default:
            return false;
        }
    }
    case ExprKind::SizeofExpr:
        return llvm::cast<SizeofExpr>(this)->operandType.getSizeInBytes().has_value();
    case ExprKind::ImplicitCastExpr: {
        auto* cast = llvm::cast<ImplicitCastExpr>(this);
        return cast->castKind == ImplicitCastExpr::NumericWiden && cast->operand->isFoldableIntConstant();
    }
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(this);
        return ifExpr->condition->isFoldableBoolConstant() && ifExpr->thenExpr->isFoldableIntConstant() && ifExpr->elseExpr->isFoldableIntConstant();
    }
    default:
        return false;
    }
}

bool Expr::isFoldableBoolConstant() const {
    switch (kind) {
    case ExprKind::VarExpr: {
        auto* varDecl = llvm::dyn_cast<VarDecl>(llvm::cast<VarExpr>(this)->decl);
        return varDecl && !varDecl->type.isMutable() && varDecl->initializer && varDecl->initializer->isFoldableBoolConstant();
    }
    case ExprKind::MemberExpr: {
        auto* decl = llvm::cast<MemberExpr>(this)->decl;
        auto* varDecl = decl ? llvm::dyn_cast<VarDecl>(decl) : nullptr;
        return varDecl && !varDecl->type.isMutable() && varDecl->initializer && varDecl->initializer->isFoldableBoolConstant();
    }
    case ExprKind::BoolLiteralExpr:
        return true;
    case ExprKind::UnaryExpr: {
        auto* unaryExpr = llvm::cast<UnaryExpr>(this);
        return unaryExpr->op == Token::Not && unaryExpr->getOperand().isFoldableBoolConstant();
    }
    case ExprKind::BinaryExpr: {
        auto& binaryExpr = llvm::cast<BinaryExpr>(*this);
        switch (binaryExpr.op) {
        case Token::AndAnd:
        case Token::OrOr:
            return binaryExpr.getLHS().isFoldableBoolConstant() && binaryExpr.getRHS().isFoldableBoolConstant();
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual:
            return binaryExpr.getLHS().isFoldableIntConstant() && binaryExpr.getRHS().isFoldableIntConstant();
        default:
            return false;
        }
    }
    case ExprKind::ImplicitCastExpr: {
        auto* cast = llvm::cast<ImplicitCastExpr>(this);
        return cast->castKind != ImplicitCastExpr::UserConversion && cast->operand->isFoldableBoolConstant();
    }
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(this);
        return ifExpr->condition->isFoldableBoolConstant() && ifExpr->thenExpr->isFoldableBoolConstant() && ifExpr->elseExpr->isFoldableBoolConstant();
    }
    default:
        return false;
    }
}

llvm::APSInt Expr::getConstantIntegerValue() const {
    switch (kind) {
    case ExprKind::VarExpr:
        if (auto* varDecl = llvm::dyn_cast<VarDecl>(llvm::cast<VarExpr>(this)->decl)) {
            if (!varDecl->type.isMutable() && varDecl->initializer) {
                return varDecl->initializer->getConstantIntegerValue();
            }
        }
        llvm_unreachable("not a constant integer");
    case ExprKind::MemberExpr:
        if (auto* decl = llvm::cast<MemberExpr>(this)->decl) {
            if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
                if (!varDecl->type.isMutable() && varDecl->initializer) {
                    return varDecl->initializer->getConstantIntegerValue();
                }
            }
        }
        llvm_unreachable("not a constant integer");
    case ExprKind::CharacterLiteralExpr:
        return llvm::APSInt::get(llvm::cast<CharacterLiteralExpr>(this)->value);
    case ExprKind::IntLiteralExpr:
        return llvm::cast<IntLiteralExpr>(this)->value;
    case ExprKind::UnaryExpr:
        return llvm::cast<UnaryExpr>(this)->getConstantIntegerValue();
    case ExprKind::BinaryExpr:
        return llvm::cast<BinaryExpr>(this)->getConstantIntegerValue();
    case ExprKind::ImplicitCastExpr: {
        // Only widening casts reach here; they preserve the value.
        auto value = llvm::cast<ImplicitCastExpr>(this)->operand->getConstantIntegerValue();
        value = value.extOrTrunc(type.getIntegerBitWidth());
        value.setIsSigned(type.isSigned());
        return value;
    }
    case ExprKind::SizeofExpr: {
        // Same shape as lexer-produced integer literals (unsigned 64-bit) so
        // mixed constant folding never sees mismatched APSInt widths.
        llvm::APSInt value(64, false);
        value = *llvm::cast<SizeofExpr>(this)->operandType.getSizeInBytes();
        return value;
    }
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(this);
        if (ifExpr->condition->getConstantBoolValue()) {
            return ifExpr->thenExpr->getConstantIntegerValue();
        }
        return ifExpr->elseExpr->getConstantIntegerValue();
    }
    case ExprKind::SwitchExpr:
        // Not folded even when constant; codegen always emits branches.
        llvm_unreachable("not a constant integer");
    default:
        llvm_unreachable("not a constant integer");
    }
}

bool Expr::getConstantBoolValue() const {
    switch (kind) {
    case ExprKind::VarExpr:
        if (auto* varDecl = llvm::dyn_cast<VarDecl>(llvm::cast<VarExpr>(this)->decl)) {
            if (!varDecl->type.isMutable() && varDecl->initializer) {
                return varDecl->initializer->getConstantBoolValue();
            }
        }
        llvm_unreachable("not a constant bool");
    case ExprKind::MemberExpr:
        if (auto* decl = llvm::cast<MemberExpr>(this)->decl) {
            if (auto* varDecl = llvm::dyn_cast<VarDecl>(decl)) {
                if (!varDecl->type.isMutable() && varDecl->initializer) {
                    return varDecl->initializer->getConstantBoolValue();
                }
            }
        }
        llvm_unreachable("not a constant bool");
    case ExprKind::BoolLiteralExpr:
        return llvm::cast<BoolLiteralExpr>(this)->value;
    case ExprKind::UnaryExpr: {
        auto* unaryExpr = llvm::cast<UnaryExpr>(this);
        if (unaryExpr->op == Token::Not) return !unaryExpr->getOperand().getConstantBoolValue();
        llvm_unreachable("not a constant bool");
    }
    case ExprKind::ImplicitCastExpr:
        return llvm::cast<ImplicitCastExpr>(this)->operand->getConstantBoolValue();
    case ExprKind::BinaryExpr: {
        auto& binaryExpr = llvm::cast<BinaryExpr>(*this);
        switch (binaryExpr.op) {
        case Token::AndAnd:
            return binaryExpr.getLHS().getConstantBoolValue() && binaryExpr.getRHS().getConstantBoolValue();
        case Token::OrOr:
            return binaryExpr.getLHS().getConstantBoolValue() || binaryExpr.getRHS().getConstantBoolValue();
        case Token::Equal:
        case Token::NotEqual:
        case Token::Less:
        case Token::LessOrEqual:
        case Token::Greater:
        case Token::GreaterOrEqual: {
            auto comparison = llvm::APSInt::compareValues(binaryExpr.getLHS().getConstantIntegerValue(), binaryExpr.getRHS().getConstantIntegerValue());
            switch (binaryExpr.op) {
            case Token::Equal:
                return comparison == 0;
            case Token::NotEqual:
                return comparison != 0;
            case Token::Less:
                return comparison < 0;
            case Token::LessOrEqual:
                return comparison <= 0;
            case Token::Greater:
                return comparison > 0;
            default:
                return comparison >= 0;
            }
        }
        default:
            llvm_unreachable("not a constant bool");
        }
    }
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(this);
        if (ifExpr->condition->getConstantBoolValue()) {
            return ifExpr->thenExpr->getConstantBoolValue();
        }
        return ifExpr->elseExpr->getConstantBoolValue();
    }
    default:
        llvm_unreachable("not a constant bool");
    }
}

bool Expr::isLvalue() const {
    switch (kind) {
    case ExprKind::VarExpr: {
        auto* var = llvm::cast<VarExpr>(this);
        return !var->decl || !llvm::isa<EnumCase>(var->decl);
    }
    case ExprKind::MemberExpr: {
        auto& member = llvm::cast<MemberExpr>(*this);
        if (!member.decl) {
            // Swizzles leave decl null: single-char is an lvalue iff the base
            // is; multi-char is a value (direct assignment is special-cased).
            // Anonymous struct members also leave decl null and are lvalues iff the base is.
            if (!member.swizzleIndices.empty()) return member.swizzleIndices.size() == 1 && member.base->isLvalue();
            return member.base->isLvalue();
        }
        if (llvm::isa<EnumCase>(member.decl)) return false;
        if (member.base->type.removeOptional().isPointerType()) return true;
        return member.base->isLvalue();
    }
    case ExprKind::IndexExpr: {
        auto& index = llvm::cast<IndexExpr>(*this);
        if (index.calleeDecl) {
            auto* function = llvm::dyn_cast<FunctionDecl>(index.calleeDecl);
            return function && function->getReturnType().isPointerType();
        }
        auto baseType = index.getBase()->type.removeOptional();
        if (baseType.isPointerType() || baseType.isArrayPointer()) return true;
        return index.getBase()->isLvalue();
    }
    case ExprKind::UnaryExpr:
        return llvm::cast<UnaryExpr>(this)->op == Token::Star;
    case ExprKind::UnwrapExpr:
        return llvm::cast<UnwrapExpr>(this)->operand->isLvalue();
    default:
        return false;
    }
}

Expr* Expr::instantiate(const llvm::StringMap<GenericArg>& genericArgs) const {
    switch (kind) {
    case ExprKind::VarExpr: {
        auto* varExpr = llvm::cast<VarExpr>(this);
        auto it = genericArgs.find(varExpr->identifier);
        if (it != genericArgs.end() && it->second.isInt()) {
            // Integer generic parameters used as values (e.g. N in Array.size())
            // instantiate to literals.
            auto* newExpr = makeAST<IntLiteralExpr>(llvm::APSInt::get(it->second.getInt()), varExpr->location);
            newExpr->endLocation = varExpr->endLocation;
            return newExpr;
        }
        // A type parameter used as a value (e.g. T in T(x)) instantiates to the
        // argument's name, but only basic types have a usable name; anything else
        // (e.g. a pointer) keeps the parameter name, producing an "unknown identifier"
        // error if that method is called with such an argument instead of crashing.
        llvm::StringRef identifier = varExpr->identifier;
        if (it != genericArgs.end() && it->second.isType() && it->second.getType().isBasicType()) {
            identifier = it->second.getType().getName();
        }
        auto* newExpr = makeAST<VarExpr>(identifier, varExpr->location);
        newExpr->endLocation = varExpr->endLocation;
        return newExpr;
    }
    case ExprKind::StringLiteralExpr: {
        auto* stringLiteralExpr = llvm::cast<StringLiteralExpr>(this);
        auto* newExpr = makeAST<StringLiteralExpr>(std::string(stringLiteralExpr->value), stringLiteralExpr->location);
        newExpr->endLocation = stringLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::CharacterLiteralExpr: {
        auto* characterLiteralExpr = llvm::cast<CharacterLiteralExpr>(this);
        auto* newExpr = makeAST<CharacterLiteralExpr>(characterLiteralExpr->value, characterLiteralExpr->location);
        newExpr->endLocation = characterLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::IntLiteralExpr: {
        auto* intLiteralExpr = llvm::cast<IntLiteralExpr>(this);
        auto* newExpr = makeAST<IntLiteralExpr>(intLiteralExpr->value, intLiteralExpr->location);
        newExpr->endLocation = intLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::FloatLiteralExpr: {
        auto* floatLiteralExpr = llvm::cast<FloatLiteralExpr>(this);
        auto* newExpr = makeAST<FloatLiteralExpr>(floatLiteralExpr->value, floatLiteralExpr->location);
        newExpr->endLocation = floatLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::BoolLiteralExpr: {
        auto* boolLiteralExpr = llvm::cast<BoolLiteralExpr>(this);
        auto* newExpr = makeAST<BoolLiteralExpr>(boolLiteralExpr->value, boolLiteralExpr->location);
        newExpr->endLocation = boolLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::NullLiteralExpr: {
        auto* nullLiteralExpr = llvm::cast<NullLiteralExpr>(this);
        auto* newExpr = makeAST<NullLiteralExpr>(nullLiteralExpr->location);
        newExpr->endLocation = nullLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::UndefinedLiteralExpr: {
        auto* undefinedLiteralExpr = llvm::cast<UndefinedLiteralExpr>(this);
        auto* newExpr = makeAST<UndefinedLiteralExpr>(undefinedLiteralExpr->location);
        newExpr->endLocation = undefinedLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::ArrayLiteralExpr: {
        auto* arrayLiteralExpr = llvm::cast<ArrayLiteralExpr>(this);
        auto elements = ::instantiate(arrayLiteralExpr->elements, genericArgs);
        auto* newExpr = makeAST<ArrayLiteralExpr>(std::move(elements), arrayLiteralExpr->location);
        newExpr->endLocation = arrayLiteralExpr->endLocation;
        return newExpr;
    }
    case ExprKind::AnonymousStructExpr: {
        auto* anonymousStructExpr = llvm::cast<AnonymousStructExpr>(this);
        auto elements =
            map(anonymousStructExpr->elements, [&](const NamedValue& element) { return NamedValue(element.name, element.value->instantiate(genericArgs)); });
        auto* newExpr = makeAST<AnonymousStructExpr>(std::move(elements), anonymousStructExpr->location);
        newExpr->endLocation = anonymousStructExpr->endLocation;
        return newExpr;
    }
    case ExprKind::UnaryExpr: {
        auto* unaryExpr = llvm::cast<UnaryExpr>(this);
        auto operand = unaryExpr->getOperand().instantiate(genericArgs);
        auto* newExpr = makeAST<UnaryExpr>(unaryExpr->op, operand, unaryExpr->location);
        newExpr->endLocation = unaryExpr->endLocation;
        return newExpr;
    }
    case ExprKind::BinaryExpr: {
        auto* binaryExpr = llvm::cast<BinaryExpr>(this);
        auto lhs = binaryExpr->getLHS().instantiate(genericArgs);
        auto rhs = binaryExpr->getRHS().instantiate(genericArgs);
        auto* newExpr = makeAST<BinaryExpr>(binaryExpr->op, lhs, rhs, binaryExpr->location);
        newExpr->parenthesized = binaryExpr->parenthesized;
        newExpr->endLocation = binaryExpr->endLocation;
        return newExpr;
    }
    case ExprKind::CallExpr: {
        auto* callExpr = llvm::cast<CallExpr>(this);
        auto callee = callExpr->callee->instantiate(genericArgs);
        auto args = map(callExpr->args, [&](auto& arg) { return NamedValue(arg.name, arg.value->instantiate(genericArgs)); });
        auto callGenericArgs = map(callExpr->genericArgs, [&](GenericArg arg) { return arg.resolve(genericArgs); });
        auto* newExpr = makeAST<CallExpr>(callee, std::move(args), std::move(callGenericArgs), callExpr->location);
        newExpr->endLocation = callExpr->endLocation;
        return newExpr;
    }
    case ExprKind::SizeofExpr: {
        auto* sizeofExpr = llvm::cast<SizeofExpr>(this);
        auto type = sizeofExpr->operandType.resolve(genericArgs);
        auto* newExpr = makeAST<SizeofExpr>(type, sizeofExpr->location);
        newExpr->endLocation = sizeofExpr->endLocation;
        return newExpr;
    }
    case ExprKind::MemberExpr: {
        auto* memberExpr = llvm::cast<MemberExpr>(this);
        auto base = memberExpr->base->instantiate(genericArgs);
        auto* newExpr = makeAST<MemberExpr>(base, memberExpr->member, memberExpr->location);
        newExpr->endLocation = memberExpr->endLocation;
        return newExpr;
    }
    case ExprKind::IndexExpr: {
        auto* indexExpr = llvm::cast<IndexExpr>(this);
        auto base = indexExpr->getBase()->instantiate(genericArgs);
        auto index = indexExpr->getIndex()->instantiate(genericArgs);
        auto* newExpr = makeAST<IndexExpr>(base, index, indexExpr->location, indexExpr->fromEnd);
        newExpr->endLocation = indexExpr->endLocation;
        return newExpr;
    }
    case ExprKind::IndexAssignmentExpr: {
        auto* indexAssignmentExpr = llvm::cast<IndexAssignmentExpr>(this);
        auto base = indexAssignmentExpr->getBase()->instantiate(genericArgs);
        auto index = indexAssignmentExpr->getIndex()->instantiate(genericArgs);
        auto value = indexAssignmentExpr->getValue()->instantiate(genericArgs);
        auto* newExpr = makeAST<IndexAssignmentExpr>(base, index, value, indexAssignmentExpr->location, indexAssignmentExpr->fromEnd);
        newExpr->endLocation = indexAssignmentExpr->endLocation;
        return newExpr;
    }
    case ExprKind::UnwrapExpr: {
        auto* unwrapExpr = llvm::cast<UnwrapExpr>(this);
        auto operand = unwrapExpr->operand->instantiate(genericArgs);
        auto* newExpr = makeAST<UnwrapExpr>(operand, unwrapExpr->location);
        newExpr->endLocation = unwrapExpr->endLocation;
        return newExpr;
    }
    case ExprKind::LambdaExpr: {
        auto* lambdaExpr = llvm::cast<LambdaExpr>(this);
        auto params = instantiateParams(lambdaExpr->functionDecl->getParams(), genericArgs);
        auto body = ::instantiate(*lambdaExpr->functionDecl->body, genericArgs);
        auto lambda = makeAST<LambdaExpr>(std::move(params), lambdaExpr->functionDecl->getModule(), lambdaExpr->location);
        lambda->endLocation = lambdaExpr->endLocation;
        lambda->functionDecl->body = std::move(body);
        return lambda;
    }
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(this);
        auto condition = ifExpr->condition->instantiate(genericArgs);
        auto thenExpr = ifExpr->thenExpr->instantiate(genericArgs);
        auto elseExpr = ifExpr->elseExpr->instantiate(genericArgs);
        auto* newExpr = makeAST<IfExpr>(condition, thenExpr, elseExpr, ifExpr->location);
        newExpr->endLocation = ifExpr->endLocation;
        return newExpr;
    }
    case ExprKind::SwitchExpr: {
        auto* switchExpr = llvm::cast<SwitchExpr>(this);
        auto condition = switchExpr->condition->instantiate(genericArgs);
        std::vector<SwitchExprArm> arms;
        for (auto& arm : switchExpr->arms) {
            auto value = arm.value->instantiate(genericArgs);
            auto associatedValue = arm.associatedValue ? llvm::cast<VarDecl>(arm.associatedValue->instantiate(genericArgs, {})) : nullptr;
            auto armExpr = arm.expr->instantiate(genericArgs);
            arms.push_back(SwitchExprArm(value, associatedValue, armExpr));
        }
        auto defaultExpr = switchExpr->defaultExpr ? switchExpr->defaultExpr->instantiate(genericArgs) : nullptr;
        auto* newExpr = makeAST<SwitchExpr>(condition, std::move(arms), defaultExpr, switchExpr->location);
        newExpr->endLocation = switchExpr->endLocation;
        return newExpr;
    }
    case ExprKind::ImplicitCastExpr: {
        auto implicitCastExpr = llvm::cast<ImplicitCastExpr>(this);
        return makeAST<ImplicitCastExpr>(implicitCastExpr->operand->instantiate(genericArgs), implicitCastExpr->type.resolve(genericArgs),
                                         implicitCastExpr->castKind, implicitCastExpr->conversionDecl);
    }
    case ExprKind::VarDeclExpr: {
        auto varDeclExpr = llvm::cast<VarDeclExpr>(this);
        return makeAST<VarDeclExpr>(llvm::cast<VarDecl>(varDeclExpr->varDecl->instantiate(genericArgs, {})));
    }
    }

    llvm_unreachable("all cases handled");
}

FieldDecl* Expr::getFieldDecl() const {
    switch (kind) {
    case ExprKind::VarExpr:
        return llvm::dyn_cast<FieldDecl>(llvm::cast<VarExpr>(this)->decl);
    case ExprKind::MemberExpr:
        return llvm::dyn_cast_or_null<FieldDecl>(llvm::cast<MemberExpr>(this)->decl);
    default:
        return nullptr;
    }
}

const Expr* Expr::withoutImplicitCast() const {
    if (auto implicitCastExpr = llvm::dyn_cast<ImplicitCastExpr>(this)) {
        return implicitCastExpr->operand;
    }
    return this;
}

bool Expr::isThis() const {
    return isVarExpr() && llvm::cast<VarExpr>(this)->identifier == "this";
}

llvm::StringRef CallExpr::getFunctionName() const {
    switch (callee->kind) {
    case ExprKind::VarExpr:
        return llvm::cast<VarExpr>(*callee).identifier;
    case ExprKind::MemberExpr:
        return llvm::cast<MemberExpr>(*callee).member;
    default:
        return "(anonymous function)";
    }
}

static Type getReceiverType(const CallExpr& call) {
    if (call.callee->isMemberExpr()) {
        return call.getReceiver()->type.removeOptional().removePointer();
    }
    return Type();
}

std::string CallExpr::getQualifiedFunctionName() const {
    return ::getQualifiedFunctionName(::getReceiverType(*this), getFunctionName(), {});
}

bool CallExpr::isMoveInit() const {
    if (getFunctionName() != "init") return false;
    if (args.size() != 1) return false;

    if (Type receiverType = ::getReceiverType(*this)) {
        return args[0].value->type.equalsIgnoreTopLevelMutable(receiverType);
    }

    return false;
}

const Expr* CallExpr::getReceiver() const {
    if (!isMethodCall()) return nullptr;
    return llvm::cast<MemberExpr>(*callee).base;
}

Expr* CallExpr::getReceiver() {
    if (!isMethodCall()) return nullptr;
    return llvm::cast<MemberExpr>(*callee).base;
}

llvm::APSInt UnaryExpr::getConstantIntegerValue() const {
    auto operand = getOperand().getConstantIntegerValue();

    switch (op) {
    case Token::Plus:
        return operand;
    case Token::Minus:
        return -operand;
    case Token::Tilde:
        return ~operand;
    default:
        llvm_unreachable("invalid constant integer prefix operator");
    }
}

bool cx::isBuiltinOp(Token::Kind op, Type left, Type right) {
    left = left.removeReference();
    right = right.removeReference();

    if (op == Token::Assignment) return true;
    if (op == Token::DotDot || op == Token::DotDotDot) return false;
    // Optionals keep resolving to the stdlib operators rather than builtin tag comparisons.
    if (left.isEnumType() && !left.isOptionalType() && left.equalsIgnoreTopLevelMutable(right)) return true;
    if (left.isEnumType() && !left.isOptionalType() && right.isInteger()) return true;
    if (left.isInteger() && right.isEnumType() && !right.isOptionalType()) return true;
    if (left.isImplementedAsPointer() && right.isImplementedAsPointer()) return true;
    return left.isBuiltinType() && right.isBuiltinType();
}

llvm::APSInt BinaryExpr::getConstantIntegerValue() const {
    // Evaluate exactly: folds mix 64/65-bit literals and (via casts) signed/unsigned values,
    // so normalize to a common wide representation instead of wrapping or asserting.
    // Overflow is diagnosed against the result type by the callers that need it.
    auto lhs = getLHS().getConstantIntegerValue().extend(256);
    auto rhs = getRHS().getConstantIntegerValue().extend(256);
    lhs.setIsSigned(true);
    rhs.setIsSigned(true);

    switch (op) {
    case Token::Plus:
        return lhs + rhs;
    case Token::Minus:
        return lhs - rhs;
    case Token::Star:
        return lhs * rhs;
    case Token::Slash:
        return lhs / rhs;
    case Token::Modulo:
        return lhs % rhs;
    case Token::PositiveModulo:
        return (lhs % rhs + rhs) % rhs;
    case Token::And:
        return lhs & rhs;
    case Token::Or:
        return lhs | rhs;
    case Token::Xor:
        return lhs ^ rhs;
    case Token::LeftShift:
        return lhs << static_cast<unsigned>(rhs.getZExtValue());
    case Token::RightShift:
        return lhs >> static_cast<unsigned>(rhs.getZExtValue());
    default:
        llvm_unreachable("invalid constant integer binary operator");
    }
}

LambdaExpr::LambdaExpr(std::vector<ParamDecl>&& params, Module* module, Location location) : Expr(ExprKind::LambdaExpr, location) {
    static uint64_t nameCounter = 0;
    FunctionProto proto("__lambda" + std::to_string(nameCounter++), std::move(params), Type(), false, false);
    this->functionDecl = makeAST<FunctionDecl>(std::move(proto), std::vector<GenericArg>(), AccessLevel::Private, *module, location);
}

VarDeclExpr::VarDeclExpr(VarDecl* varDecl) : Expr(ExprKind::VarDeclExpr, varDecl->getLocation()), varDecl(varDecl) {}

const Expr* AnonymousStructExpr::getElementByName(llvm::StringRef name) const {
    for (auto& element : elements) {
        if (element.name == name) {
            return element.value;
        }
    }
    return nullptr;
}
