#include <iostream>
#include <limits>
#include <unordered_map>
#include <boost/utility/string_ref.hpp>
#include "typecheck.h"
#include "../ast/type.h"
#include "../ast/expr.h"
#include "../ast/decl.h"

static std::unordered_map<std::string, Type> symbolTable;

template<typename... Args>
[[noreturn]] static void error(Args&&... args) {
    std::cout << "error: ";
    using expander = int[];
    (void)expander{0, (void(std::cout << std::forward<Args>(args)), 0)...};
    std::cout << '\n';
    exit(1);
}

Type typecheck(Expr& expr);

Type typecheck(VariableExpr& expr) {
    auto it = symbolTable.find(expr.identifier);
    if (it == symbolTable.end()) {
        error("unknown identifier '", expr.identifier, "'");
    }
    return it->second;
}

Type typecheck(IntLiteralExpr& expr) {
    if (expr.value >= std::numeric_limits<int32_t>::min()
    &&  expr.value <= std::numeric_limits<int32_t>::max())
        return Type("int");
    else
    if (expr.value >= std::numeric_limits<int64_t>::min()
    &&  expr.value <= std::numeric_limits<int64_t>::max())
        return Type("int64");
    else
        error("integer literal is too large");
}

Type typecheck(BoolLiteralExpr&) {
    return Type("bool");
}

Type typecheck(PrefixExpr& expr) {
    return typecheck(*expr.operand);
}

Type typecheck(BinaryExpr& expr) {
    Type leftType = typecheck(*expr.left);
    Type rightType = typecheck(*expr.right);
    if (leftType != rightType) {
        error("operands to binary expression must have same type");
    }
    return leftType;
}

Type typecheck(Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:   return typecheck(expr.getVariableExpr()); break;
        case ExprKind::IntLiteralExpr: return typecheck(expr.getIntLiteralExpr()); break;
        case ExprKind::BoolLiteralExpr:return typecheck(expr.getBoolLiteralExpr()); break;
        case ExprKind::PrefixExpr:     return typecheck(expr.getPrefixExpr()); break;
        case ExprKind::BinaryExpr:     return typecheck(expr.getBinaryExpr()); break;
    }
}

void typecheck(ReturnStmt& stmt) {
    for (Expr& expr : stmt.values) typecheck(expr);
}

void typecheck(VarDecl& decl);

void typecheck(VariableStmt& stmt) {
    typecheck(*stmt.decl);
}

void typecheck(IncrementStmt& stmt) {
    typecheck(stmt.operand);
    // TODO: check that operand supports increment operation.
}

void typecheck(DecrementStmt& stmt) {
    typecheck(stmt.operand);
    // TODO: check that operand supports decrement operation.
}

void typecheck(Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    typecheck(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt:  typecheck(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: typecheck(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: typecheck(stmt.getDecrementStmt()); break;
    }
}

void typecheck(ParamDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, Type(decl.type)});
}

void typecheck(FuncDecl& decl) {
    // TODO: decl.returnType, decl.name
    auto symbolTableBackup = symbolTable;
    for (ParamDecl& param : decl.params) typecheck(param);
    for (Stmt& stmt : decl.body) typecheck(stmt);
    symbolTable = symbolTableBackup;
}

void typecheck(VarDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, typecheck(decl.initializer)});
}

void typecheck(Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: typecheck(decl.getParamDecl()); break;
        case DeclKind::FuncDecl:  typecheck(decl.getFuncDecl()); break;
        case DeclKind::VarDecl:   typecheck(decl.getVarDecl()); break;
    }
}

void typecheck(std::vector<Decl>& decls) {
    for (Decl& decl : decls) typecheck(decl);
}
