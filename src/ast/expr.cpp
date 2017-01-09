#include "expr.h"
#include "type.h"

Type Expr::getType() const {
    switch (getKind()) {
        case ExprKind::VariableExpr:   return Type("int"); // TODO: Symbol table.
        case ExprKind::IntLiteralExpr: return Type("int");
        case ExprKind::PrefixExpr:     return getPrefixExpr().operand->getType();
        case ExprKind::BinaryExpr:     return getBinaryExpr().left->getType();
    }
}
