#include "expr.h"
#include "../parser/token.h"
#include "../irgen/mangle.h"

using namespace delta;

bool Expr::isLvalue() const {
    switch (getKind()) {
        case ExprKind::VariableExpr: case ExprKind::StrLiteralExpr: case ExprKind::ArrayLiteralExpr:
        case ExprKind::MemberExpr: case ExprKind::SubscriptExpr:
            return true;
        case ExprKind::IntLiteralExpr: case ExprKind::FloatLiteralExpr:
        case ExprKind::BoolLiteralExpr: case ExprKind::CastExpr: case ExprKind::UnwrapExpr:
        case ExprKind::NullLiteralExpr: case ExprKind::BinaryExpr: case ExprKind::CallExpr:
            return false;
        case ExprKind::PrefixExpr:
            return getPrefixExpr().op == STAR;
    }
}

llvm::StringRef CallExpr::getFuncName() const {
    switch (func->getKind()) {
        case ExprKind::VariableExpr: return llvm::cast<VariableExpr>(*func).identifier;
        case ExprKind::MemberExpr: return llvm::cast<MemberExpr>(*func).member;
        default: return "(anonymous function)";
    }
}

std::string CallExpr::getMangledFuncName() const {
    if (func->isMemberExpr()) {
        Type receiverType = getReceiver()->getType();
        if (receiverType.isPtrType()) receiverType = receiverType.getPointee();
        return receiverType.getName().str() + "." + getFuncName().str();
    }
    if (isInitializerCall) return mangleInitDecl(getFuncName());
    return getFuncName();
}

Expr* CallExpr::getReceiver() const {
    return llvm::cast<MemberExpr>(*func).base.get();
}
