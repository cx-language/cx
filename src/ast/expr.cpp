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

    case ExprKind::TupleExpr:
        for (auto& element : llvm::cast<TupleExpr>(this)->elements) {
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
        return binaryExpr->op != Token::Assignment && binaryExpr->getLHS().isConstant() && binaryExpr->getRHS().isConstant();
    }

    case ExprKind::CallExpr:
    case ExprKind::SizeofExpr: // TODO: sizeof should be a constant expression.
    case ExprKind::MemberExpr:
    case ExprKind::IndexExpr:
    case ExprKind::IndexAssignmentExpr:
    case ExprKind::UnwrapExpr:
    case ExprKind::LambdaExpr:
    case ExprKind::UndefinedLiteralExpr:
    case ExprKind::VarDeclExpr:
        return false;

    case ExprKind::ImplicitCastExpr:
        return llvm::cast<ImplicitCastExpr>(this)->operand->isConstant();

    case ExprKind::IfExpr:
        return llvm::cast<IfExpr>(this)->condition->isConstant() && llvm::cast<IfExpr>(this)->thenExpr->isConstant()
            && llvm::cast<IfExpr>(this)->elseExpr->isConstant();
    }

    llvm_unreachable("all cases handled");
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
    case ExprKind::SizeofExpr:
    case ExprKind::IfExpr:
        llvm_unreachable("unimplemented");
    default:
        llvm_unreachable("not a constant integer");
    }
}

bool Expr::isLvalue() const {
    switch (kind) {
    case ExprKind::VarExpr:
    case ExprKind::MemberExpr:
    case ExprKind::IndexExpr:
        return true;
    case ExprKind::UnaryExpr:
        return llvm::cast<UnaryExpr>(this)->op == Token::Star;
    default:
        return false;
    }
}

Expr* Expr::instantiate(const llvm::StringMap<Type>& genericArgs) const {
    switch (kind) {
    case ExprKind::VarExpr: {
        auto* varExpr = llvm::cast<VarExpr>(this);
        auto it = genericArgs.find(varExpr->identifier);
        auto identifier = it != genericArgs.end() ? it->second.getName().str() : varExpr->identifier;
        return makeAST<VarExpr>(std::move(identifier), varExpr->location);
    }
    case ExprKind::StringLiteralExpr: {
        auto* stringLiteralExpr = llvm::cast<StringLiteralExpr>(this);
        return makeAST<StringLiteralExpr>(std::string(stringLiteralExpr->value), stringLiteralExpr->location);
    }
    case ExprKind::CharacterLiteralExpr: {
        auto* characterLiteralExpr = llvm::cast<CharacterLiteralExpr>(this);
        return makeAST<CharacterLiteralExpr>(characterLiteralExpr->value, characterLiteralExpr->location);
    }
    case ExprKind::IntLiteralExpr: {
        auto* intLiteralExpr = llvm::cast<IntLiteralExpr>(this);
        return makeAST<IntLiteralExpr>(intLiteralExpr->value, intLiteralExpr->location);
    }
    case ExprKind::FloatLiteralExpr: {
        auto* floatLiteralExpr = llvm::cast<FloatLiteralExpr>(this);
        return makeAST<FloatLiteralExpr>(floatLiteralExpr->value, floatLiteralExpr->location);
    }
    case ExprKind::BoolLiteralExpr: {
        auto* boolLiteralExpr = llvm::cast<BoolLiteralExpr>(this);
        return makeAST<BoolLiteralExpr>(boolLiteralExpr->value, boolLiteralExpr->location);
    }
    case ExprKind::NullLiteralExpr: {
        auto* nullLiteralExpr = llvm::cast<NullLiteralExpr>(this);
        return makeAST<NullLiteralExpr>(nullLiteralExpr->location);
    }
    case ExprKind::UndefinedLiteralExpr: {
        auto* undefinedLiteralExpr = llvm::cast<UndefinedLiteralExpr>(this);
        return makeAST<UndefinedLiteralExpr>(undefinedLiteralExpr->location);
    }
    case ExprKind::ArrayLiteralExpr: {
        auto* arrayLiteralExpr = llvm::cast<ArrayLiteralExpr>(this);
        auto elements = ::instantiate(arrayLiteralExpr->elements, genericArgs);
        return makeAST<ArrayLiteralExpr>(std::move(elements), arrayLiteralExpr->location);
    }
    case ExprKind::TupleExpr: {
        auto* tupleExpr = llvm::cast<TupleExpr>(this);
        auto elements =
            map(tupleExpr->elements, [&](const NamedValue& element) { return NamedValue(std::string(element.name), element.value->instantiate(genericArgs)); });
        return makeAST<TupleExpr>(std::move(elements), tupleExpr->location);
    }
    case ExprKind::UnaryExpr: {
        auto* unaryExpr = llvm::cast<UnaryExpr>(this);
        auto operand = unaryExpr->getOperand().instantiate(genericArgs);
        return makeAST<UnaryExpr>(unaryExpr->op, operand, unaryExpr->location);
    }
    case ExprKind::BinaryExpr: {
        auto* binaryExpr = llvm::cast<BinaryExpr>(this);
        auto lhs = binaryExpr->getLHS().instantiate(genericArgs);
        auto rhs = binaryExpr->getRHS().instantiate(genericArgs);
        return makeAST<BinaryExpr>(binaryExpr->op, lhs, rhs, binaryExpr->location);
    }
    case ExprKind::CallExpr: {
        auto* callExpr = llvm::cast<CallExpr>(this);
        auto callee = callExpr->callee->instantiate(genericArgs);
        auto args = map(callExpr->args, [&](auto& arg) { return NamedValue(std::string(arg.name), arg.value->instantiate(genericArgs)); });
        auto callGenericArgs = map(callExpr->genericArgs, [&](Type type) { return type.resolve(genericArgs); });
        return makeAST<CallExpr>(callee, std::move(args), std::move(callGenericArgs), callExpr->location);
    }
    case ExprKind::SizeofExpr: {
        auto* sizeofExpr = llvm::cast<SizeofExpr>(this);
        auto type = sizeofExpr->operandType.resolve(genericArgs);
        return makeAST<SizeofExpr>(type, sizeofExpr->location);
    }
    case ExprKind::MemberExpr: {
        auto* memberExpr = llvm::cast<MemberExpr>(this);
        auto base = memberExpr->base->instantiate(genericArgs);
        return makeAST<MemberExpr>(base, std::string(memberExpr->member), memberExpr->location);
    }
    case ExprKind::IndexExpr: {
        auto* indexExpr = llvm::cast<IndexExpr>(this);
        auto base = indexExpr->getBase()->instantiate(genericArgs);
        auto index = indexExpr->getIndex()->instantiate(genericArgs);
        return makeAST<IndexExpr>(base, index, indexExpr->location);
    }
    case ExprKind::IndexAssignmentExpr: {
        auto* indexAssignmentExpr = llvm::cast<IndexAssignmentExpr>(this);
        auto base = indexAssignmentExpr->getBase()->instantiate(genericArgs);
        auto index = indexAssignmentExpr->getIndex()->instantiate(genericArgs);
        auto value = indexAssignmentExpr->getValue()->instantiate(genericArgs);
        return makeAST<IndexAssignmentExpr>(base, index, value, indexAssignmentExpr->location);
    }
    case ExprKind::UnwrapExpr: {
        auto* unwrapExpr = llvm::cast<UnwrapExpr>(this);
        auto operand = unwrapExpr->operand->instantiate(genericArgs);
        return makeAST<UnwrapExpr>(operand, unwrapExpr->location);
    }
    case ExprKind::LambdaExpr: {
        auto* lambdaExpr = llvm::cast<LambdaExpr>(this);
        auto params = instantiateParams(lambdaExpr->functionDecl->getParams(), genericArgs);
        auto body = ::instantiate(*lambdaExpr->functionDecl->body, genericArgs);
        auto lambda = makeAST<LambdaExpr>(std::move(params), lambdaExpr->functionDecl->getModule(), lambdaExpr->location);
        lambda->functionDecl->body = std::move(body);
        return lambda;
    }
    case ExprKind::IfExpr: {
        auto* ifExpr = llvm::cast<IfExpr>(this);
        auto condition = ifExpr->condition->instantiate(genericArgs);
        auto thenExpr = ifExpr->thenExpr->instantiate(genericArgs);
        auto elseExpr = ifExpr->elseExpr->instantiate(genericArgs);
        return makeAST<IfExpr>(condition, thenExpr, elseExpr, ifExpr->location);
    }
    case ExprKind::ImplicitCastExpr: {
        auto implicitCastExpr = llvm::cast<ImplicitCastExpr>(this);
        return makeAST<ImplicitCastExpr>(implicitCastExpr->operand->instantiate(genericArgs), implicitCastExpr->type.resolve(genericArgs),
                                         implicitCastExpr->castKind);
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
    // TODO: Add overflow checks.
    // TODO: Handle signedness for '>>' operator;

    auto lhs = getLHS().getConstantIntegerValue();
    auto rhs = getRHS().getConstantIntegerValue();

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
    this->functionDecl = makeAST<FunctionDecl>(std::move(proto), std::vector<Type>(), AccessLevel::Private, *module, location);
}

VarDeclExpr::VarDeclExpr(VarDecl* varDecl) : Expr(ExprKind::VarDeclExpr, varDecl->getLocation()), varDecl(varDecl) {}

const Expr* TupleExpr::getElementByName(llvm::StringRef name) const {
    for (auto& element : elements) {
        if (element.name == name) {
            return element.value;
        }
    }
    return nullptr;
}
