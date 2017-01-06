#include <iostream>
#include <string>
#include "ast_printer.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"

static int indentLevel = 0;

/// Inserts a line break followed by appropriate indentation.
static std::ostream& br(std::ostream& out) {
    return out << '\n' << std::string(indentLevel * 4, ' ');
}

std::ostream& operator<<(std::ostream& out, const Expr& expr);

std::ostream& operator<<(std::ostream& out, const VariableExpr& expr) {
    return out << expr.identifier;
}

std::ostream& operator<<(std::ostream& out, const IntLiteralExpr& expr) {
    return out << expr.value;
}

std::ostream& operator<<(std::ostream& out, const PrefixExpr& expr) {
    return out << "(" << expr.op << *expr.operand << ")";
}

std::ostream& operator<<(std::ostream& out, const BinaryExpr& expr) {
    return out << "(" << *expr.left << " " << expr.op << " " << *expr.right << ")";
}

std::ostream& operator<<(std::ostream& out, const Expr& expr) {
    switch (expr.getType()) {
        case ExprType::VariableExpr:   return out << expr.getVariableExpr();
        case ExprType::IntLiteralExpr: return out << expr.getIntLiteralExpr();
        case ExprType::PrefixExpr:     return out << expr.getPrefixExpr();
        case ExprType::BinaryExpr:     return out << expr.getBinaryExpr();
    }
}

std::ostream& operator<<(std::ostream& out, const ReturnStmt& stmt) {
    out << br << "(return-stmt ";
    for (const Expr& value : stmt.values) {
        out << value;
        if (&value != &stmt.values.back()) out << " ";
    }
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const VariableStmt& stmt) {
    return out << br << "(var-stmt " << stmt.decl->name << " " << stmt.decl->initializer << ")";
}

std::ostream& operator<<(std::ostream& out, const IncrementStmt& stmt) {
    return out << br << "(inc-stmt " << stmt.operand << ")";
}

std::ostream& operator<<(std::ostream& out, const DecrementStmt& stmt) {
    return out << br << "(dec-stmt " << stmt.operand << ")";
}

std::ostream& operator<<(std::ostream& out, const Stmt& stmt) {
    switch (stmt.getType()) {
        case StmtType::ReturnStmt:    return out << stmt.getReturnStmt();
        case StmtType::VariableStmt:  return out << stmt.getVariableStmt();
        case StmtType::IncrementStmt: return out << stmt.getIncrementStmt();
        case StmtType::DecrementStmt: return out << stmt.getDecrementStmt();
    }
}

std::ostream& operator<<(std::ostream& out, const Type& type) {
    if (type.isTuple()) {
        out << "(";
        for (const Type& memberType : type.getNames()) {
            out << memberType;
            if (&memberType != &type.getNames().back()) out << " ";
        }
        out << ")";
    } else {
        out << type.getName();
    }
    return out;
}

std::ostream& operator<<(std::ostream& out, const ParamDecl& decl) {
    return out << "(" << decl.type << " " << decl.name << ")";
}

std::ostream& operator<<(std::ostream& out, const FuncDecl& decl) {
    out << br << "(func-decl " << decl.name << " (";
    for (const ParamDecl& param : decl.params) {
        out << param;
        if (&param != &decl.params.back()) out << " ";
    }
    out << ") " << decl.returnType;
    indentLevel++;
    for (const Stmt& stmt : decl.body) {
        out << stmt;
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const VarDecl& decl) {
    return out << br << "(var-decl " << decl.name << " " << decl.initializer << ")";
}

std::ostream& operator<<(std::ostream& out, const Decl& decl) {
    switch (decl.getType()) {
        case DeclType::ParamDecl: return out << decl.getParamDecl();
        case DeclType::FuncDecl:  return out << decl.getFuncDecl();
        case DeclType::VarDecl:   return out << decl.getVarDecl();
    }
}

std::ostream& operator<<(std::ostream& out, const AST& decls) {
    out << "(source-file";
    indentLevel++;
    for (const Decl& decl : decls) {
        out << decl;
    }
    indentLevel--;
    return out << ")\n";
}
