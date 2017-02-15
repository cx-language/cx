#include "expr.h"
#include "decl.h"
#include "../parser/parser.hpp"

using namespace delta;

bool Expr::isLvalue() const {
    switch (getKind()) {
        case ExprKind::VariableExpr: case ExprKind::StrLiteralExpr: case ExprKind::MemberExpr:
        case ExprKind::SubscriptExpr:
            return true;
        case ExprKind::IntLiteralExpr: case ExprKind::BoolLiteralExpr: case ExprKind::CastExpr:
        case ExprKind::NullLiteralExpr: case ExprKind::BinaryExpr: case ExprKind::CallExpr:
            return false;
        case ExprKind::PrefixExpr:
            return getPrefixExpr().op.rawValue == STAR;
    }
}

SrcLoc Expr::getSrcLoc() const {
    switch (getKind()) {
        case ExprKind::VariableExpr:    return getVariableExpr().srcLoc;
        case ExprKind::StrLiteralExpr:  return getStrLiteralExpr().srcLoc;
        case ExprKind::IntLiteralExpr:  return getIntLiteralExpr().srcLoc;
        case ExprKind::BoolLiteralExpr: return getBoolLiteralExpr().srcLoc;
        case ExprKind::NullLiteralExpr: return getNullLiteralExpr().srcLoc;
        case ExprKind::PrefixExpr:      return getPrefixExpr().srcLoc;
        case ExprKind::BinaryExpr:      return getBinaryExpr().srcLoc;
        case ExprKind::CallExpr:        return getCallExpr().srcLoc;
        case ExprKind::CastExpr:        return getCastExpr().srcLoc;
        case ExprKind::MemberExpr:      return getMemberExpr().baseSrcLoc;
        case ExprKind::SubscriptExpr:   return getSubscriptExpr().srcLoc;
    }
}
