#include "expr.h"
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "token.h"
#include "mangle.h"

using namespace delta;

bool Expr::isLvalue() const {
    switch (getKind()) {
        case ExprKind::VarExpr: case ExprKind::StringLiteralExpr:
        case ExprKind::CharacterLiteralExpr: case ExprKind::ArrayLiteralExpr:
        case ExprKind::MemberExpr: case ExprKind::SubscriptExpr:
            return true;
        case ExprKind::IntLiteralExpr: case ExprKind::FloatLiteralExpr: case ExprKind::SizeofExpr:
        case ExprKind::BoolLiteralExpr: case ExprKind::CastExpr: case ExprKind::UnwrapExpr:
        case ExprKind::NullLiteralExpr: case ExprKind::BinaryExpr: case ExprKind::CallExpr:
            return false;
        case ExprKind::PrefixExpr:
            return llvm::cast<PrefixExpr>(this)->getOperator() == STAR;
    }
    llvm_unreachable("all cases handled");
}

void Expr::markAsMoved() {
    if (auto* varExpr = llvm::dyn_cast<VarExpr>(this)) {
        varExpr->getDecl()->markAsMoved();
    }
}

std::unique_ptr<Expr> Expr::instantiate(const llvm::StringMap<Type>& genericArgs) const {
    switch (getKind()) {
        case ExprKind::VarExpr: {
            auto* varExpr = llvm::cast<VarExpr>(this);
            std::string identifier;

            auto it = genericArgs.find(varExpr->getIdentifier());
            if (it != genericArgs.end()) {
                identifier = it->second.getName();
            } else {
                identifier = varExpr->getIdentifier();
            }

            return llvm::make_unique<VarExpr>(std::move(identifier), varExpr->getLocation());
        }
        case ExprKind::StringLiteralExpr: {
            auto* stringLiteralExpr = llvm::cast<StringLiteralExpr>(this);
            return llvm::make_unique<StringLiteralExpr>(stringLiteralExpr->getValue(),
                                                        stringLiteralExpr->getLocation());
        }
        case ExprKind::CharacterLiteralExpr: {
            auto* characterLiteralExpr = llvm::cast<CharacterLiteralExpr>(this);
            return llvm::make_unique<CharacterLiteralExpr>(characterLiteralExpr->getValue(),
                                                           characterLiteralExpr->getLocation());
        }
        case ExprKind::IntLiteralExpr: {
            auto* intLiteralExpr = llvm::cast<IntLiteralExpr>(this);
            return llvm::make_unique<IntLiteralExpr>(intLiteralExpr->getValue(),
                                                     intLiteralExpr->getLocation());
        }
        case ExprKind::FloatLiteralExpr: {
            auto* floatLiteralExpr = llvm::cast<FloatLiteralExpr>(this);
            return llvm::make_unique<FloatLiteralExpr>(floatLiteralExpr->getValue(),
                                                       floatLiteralExpr->getLocation());
        }
        case ExprKind::BoolLiteralExpr: {
            auto* boolLiteralExpr = llvm::cast<BoolLiteralExpr>(this);
            return llvm::make_unique<BoolLiteralExpr>(boolLiteralExpr->getValue(),
                                                      boolLiteralExpr->getLocation());
        }
        case ExprKind::NullLiteralExpr: {
            auto* nullLiteralExpr = llvm::cast<NullLiteralExpr>(this);
            return llvm::make_unique<NullLiteralExpr>(nullLiteralExpr->getLocation());
        }
        case ExprKind::ArrayLiteralExpr: {
            auto* arrayLiteralExpr = llvm::cast<ArrayLiteralExpr>(this);
            auto elements = ::instantiate(arrayLiteralExpr->getElements(), genericArgs);
            return llvm::make_unique<ArrayLiteralExpr>(std::move(elements),
                                                       arrayLiteralExpr->getLocation());
        }
        case ExprKind::PrefixExpr: {
            auto* prefixExpr = llvm::cast<PrefixExpr>(this);
            auto operand = prefixExpr->getOperand().instantiate(genericArgs);
            return llvm::make_unique<PrefixExpr>(prefixExpr->getOperator(), std::move(operand),
                                                 prefixExpr->getLocation());
        }
        case ExprKind::BinaryExpr: {
            auto* binaryExpr = llvm::cast<BinaryExpr>(this);
            auto lhs = binaryExpr->getLHS().instantiate(genericArgs);
            auto rhs = binaryExpr->getRHS().instantiate(genericArgs);
            return llvm::make_unique<BinaryExpr>(binaryExpr->getOperator(), std::move(lhs),
                                                 std::move(rhs), binaryExpr->getLocation());
        }
        case ExprKind::CallExpr: {
            auto* callExpr = llvm::cast<CallExpr>(this);
            auto callee = callExpr->getCallee().instantiate(genericArgs);
            auto args = map(callExpr->getArgs(), [&](const Argument& arg) {
                return Argument(arg.getName(), arg.getValue()->instantiate(genericArgs));
            });
            auto callGenericArgs = map(callExpr->getGenericArgs(), [&](Type type) {
                return type.resolve(genericArgs);
            });
            return llvm::make_unique<CallExpr>(std::move(callee), std::move(args),
                                               std::move(callGenericArgs), callExpr->getLocation());
        }
        case ExprKind::CastExpr: {
            auto* castExpr = llvm::cast<CastExpr>(this);
            auto targetType = castExpr->getTargetType().resolve(genericArgs);
            auto expr = castExpr->getExpr().instantiate(genericArgs);
            return llvm::make_unique<CastExpr>(targetType, std::move(expr), castExpr->getLocation());
        }
        case ExprKind::SizeofExpr: {
            auto* sizeofExpr = llvm::cast<SizeofExpr>(this);
            auto type = sizeofExpr->getType().resolve(genericArgs);
            return llvm::make_unique<SizeofExpr>(type, sizeofExpr->getLocation());
        }
        case ExprKind::MemberExpr: {
            auto* memberExpr = llvm::cast<MemberExpr>(this);
            auto base = memberExpr->getBaseExpr()->instantiate(genericArgs);
            return llvm::make_unique<MemberExpr>(std::move(base), memberExpr->getMemberName(),
                                                 memberExpr->getLocation());
        }
        case ExprKind::SubscriptExpr: {
            auto* subscriptExpr = llvm::cast<SubscriptExpr>(this);
            auto base = subscriptExpr->getBaseExpr()->instantiate(genericArgs);
            auto index = subscriptExpr->getIndexExpr()->instantiate(genericArgs);
            return llvm::make_unique<SubscriptExpr>(std::move(base), std::move(index),
                                                    subscriptExpr->getLocation());
        }
        case ExprKind::UnwrapExpr: {
            auto* unwrapExpr = llvm::cast<UnwrapExpr>(this);
            auto operand = unwrapExpr->getOperand().instantiate(genericArgs);
            return llvm::make_unique<UnwrapExpr>(std::move(operand), unwrapExpr->getLocation());
        }
    }
    llvm_unreachable("all cases handled");
}

llvm::StringRef CallExpr::getFunctionName() const {
    switch (getCallee().getKind()) {
        case ExprKind::VarExpr: return llvm::cast<VarExpr>(getCallee()).getIdentifier();
        case ExprKind::MemberExpr: return llvm::cast<MemberExpr>(getCallee()).getMemberName();
        default: return "(anonymous function)";
    }
}

std::string CallExpr::getMangledFunctionName() const {
    Type receiverType;

    if (getCallee().isMemberExpr()) {
        receiverType = getReceiver()->getType().removePointer();
    }

    return mangleFunctionDecl(receiverType, getFunctionName());
}

bool CallExpr::isInitCall() const {
    return getCalleeDecl()->isInitDecl();
}

Expr* CallExpr::getReceiver() const {
    if (!isMethodCall()) return nullptr;
    return llvm::cast<MemberExpr>(getCallee()).getBaseExpr();
}

bool BinaryExpr::isBuiltinOp() const {
    if (op == DOTDOT || op == DOTDOTDOT) return false;
    return getLHS().getType().isBuiltinType() && getRHS().getType().isBuiltinType();
}
