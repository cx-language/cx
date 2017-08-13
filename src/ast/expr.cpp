#include "expr.h"
#include <llvm/Support/ErrorHandling.h>
#include "decl.h"
#include "token.h"
#include "mangle.h"

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

llvm::StringRef CallExpr::getFunctionName() const {
    switch (getCallee().getKind()) {
        case ExprKind::VarExpr: return llvm::cast<VarExpr>(getCallee()).getIdentifier();
        case ExprKind::MemberExpr: return llvm::cast<MemberExpr>(getCallee()).getMemberName();
        default: return "(anonymous function)";
    }
}

std::string CallExpr::getMangledFunctionName() const {
    if (getCallee().isMemberExpr()) {
        Type receiverType = getReceiver()->getType();
        if (receiverType.isPointerType()) receiverType = receiverType.getPointee();
        return mangleFunctionDecl(receiverType.getName(), getFunctionName(), genericArgs);
    }
    if (isInitCall()) return mangleInitDecl(getFunctionName());
    return mangleFunctionDecl("", getFunctionName(), genericArgs);
}

bool CallExpr::isInitCall() const {
    return getCalleeDecl()->isInitDecl();
}

Expr* CallExpr::getReceiver() const {
    if (!isMethodCall()) return nullptr;
    return llvm::cast<MemberExpr>(getCallee()).getBaseExpr();
}
