#include "expr.h"
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "token.h"
#include "mangle.h"
#include "type-resolver.h"

using namespace delta;

bool Expr::isLvalue() const {
    switch (getKind()) {
        case ExprKind::VarExpr: case ExprKind::StringLiteralExpr: case ExprKind::ArrayLiteralExpr:
        case ExprKind::MemberExpr: case ExprKind::SubscriptExpr:
            return true;
        case ExprKind::IntLiteralExpr: case ExprKind::FloatLiteralExpr:
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

llvm::StringRef CallExpr::getFunctionName(const TypeResolver* resolver) const {
    switch (getCallee().getKind()) {
        case ExprKind::VarExpr: {
            auto identifier = llvm::cast<VarExpr>(getCallee()).getIdentifier();

            if (resolver) {
                if (Type type = resolver->resolveTypePlaceholder(identifier)) {
                    if (type.isBasicType()) {
                        return type.getName();
                    } else {
                        llvm_unreachable("unimplemented");
                    }
                }
            }

            return identifier;
        }
        case ExprKind::MemberExpr: return llvm::cast<MemberExpr>(getCallee()).getMemberName();
        default: return "(anonymous function)";
    }
}

std::string CallExpr::getMangledFunctionName(const TypeResolver& resolver) const {
    std::string receiverTypeName;

    if (getCallee().isMemberExpr()) {
        receiverTypeName = resolver.resolve(getReceiver()->getType()).removePointer().getName();
    }

    return mangleFunctionDecl(receiverTypeName, getFunctionName(), genericArgs);
}

bool CallExpr::isInitCall() const {
    return getCalleeDecl()->isInitDecl();
}

Expr* CallExpr::getReceiver() const {
    if (!isMethodCall()) return nullptr;
    return llvm::cast<MemberExpr>(getCallee()).getBaseExpr();
}

bool BinaryExpr::isBuiltinOp(const TypeResolver& resolver) const {
    if (op == DOTDOT || op == DOTDOTDOT) return false;
    return resolver.resolve(getLHS().getType()).isBuiltinType()
        && resolver.resolve(getRHS().getType()).isBuiltinType();
}
