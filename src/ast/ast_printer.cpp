#include <iostream>
#include <string>
#include "ast_printer.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"

using namespace delta;

namespace {

int indentLevel = 0;

/// Inserts a line break followed by appropriate indentation.
std::ostream& br(std::ostream& out) {
    return out << '\n' << std::string(indentLevel * 4, ' ');
}

std::ostream& operator<<(std::ostream& out, const Expr& expr);
std::ostream& operator<<(std::ostream& out, const Stmt& stmt);

std::ostream& operator<<(std::ostream& out, const VariableExpr& expr) {
    return out << expr.identifier;
}

std::ostream& operator<<(std::ostream& out, const StrLiteralExpr& expr) {
    return out << '"' << expr.value << '"';
}

std::ostream& operator<<(std::ostream& out, const IntLiteralExpr& expr) {
    return out << expr.value;
}

std::ostream& operator<<(std::ostream& out, const BoolLiteralExpr& expr) {
    return out << (expr.value ? "true" : "false");
}

std::ostream& operator<<(std::ostream& out, const NullLiteralExpr& expr) {
    return out << "null";
}

std::ostream& operator<<(std::ostream& out, const ArrayLiteralExpr& expr) {
    out << "(array-literal";
    for (auto& e : expr.elements) out << " " << *e;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const PrefixExpr& expr) {
    return out << "(" << expr.op << *expr.operand << ")";
}

std::ostream& operator<<(std::ostream& out, const BinaryExpr& expr) {
    return out << "(" << *expr.left << " " << expr.op << " " << *expr.right << ")";
}

std::ostream& operator<<(std::ostream& out, const CallExpr& expr) {
    out << "(call " << expr.funcName << " ";
    for (const Arg& arg : expr.args) {
        out << *arg.value;
        if (&arg != &expr.args.back()) out << " ";
    }
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const CastExpr& expr) {
    return out << "(cast " << *expr.expr << " " << expr.type << ")";
}

std::ostream& operator<<(std::ostream& out, const MemberExpr& expr) {
    return out << "(member-expr " << *expr.base << " " << expr.member << ")";
}

std::ostream& operator<<(std::ostream& out, const SubscriptExpr& expr) {
    return out << "(subscript " << *expr.array << " " << *expr.index << ")";
}

std::ostream& operator<<(std::ostream& out, const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:   return out << expr.getVariableExpr();
        case ExprKind::StrLiteralExpr: return out << expr.getStrLiteralExpr();
        case ExprKind::IntLiteralExpr: return out << expr.getIntLiteralExpr();
        case ExprKind::BoolLiteralExpr:return out << expr.getBoolLiteralExpr();
        case ExprKind::NullLiteralExpr:return out << expr.getNullLiteralExpr();
        case ExprKind::ArrayLiteralExpr:return out<< expr.getArrayLiteralExpr();
        case ExprKind::PrefixExpr:     return out << expr.getPrefixExpr();
        case ExprKind::BinaryExpr:     return out << expr.getBinaryExpr();
        case ExprKind::CallExpr:       return out << expr.getCallExpr();
        case ExprKind::CastExpr:       return out << expr.getCastExpr();
        case ExprKind::MemberExpr:     return out << expr.getMemberExpr();
        case ExprKind::SubscriptExpr:  return out << expr.getSubscriptExpr();
    }
}

std::ostream& operator<<(std::ostream& out, const ReturnStmt& stmt) {
    out << br << "(return-stmt ";
    for (const auto& value : stmt.values) {
        out << *value;
        if (&value != &stmt.values.back()) out << " ";
    }
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const VariableStmt& stmt) {
    return out << br << "(var-stmt " << stmt.decl->name << " " << *stmt.decl->initializer << ")";
}

std::ostream& operator<<(std::ostream& out, const IncrementStmt& stmt) {
    return out << br << "(inc-stmt " << *stmt.operand << ")";
}

std::ostream& operator<<(std::ostream& out, const DecrementStmt& stmt) {
    return out << br << "(dec-stmt " << *stmt.operand << ")";
}

std::ostream& operator<<(std::ostream& out, const DeferStmt& stmt) {
    return out << br << "(defer-stmt " << *stmt.expr << ")";
}

std::ostream& operator<<(std::ostream& out, const IfStmt& stmt) {
    out << br << "(if-stmt " << *stmt.condition;
    indentLevel++;
    out << br << "(then";
    indentLevel++;
    for (const Stmt& substmt : stmt.thenBody) {
        out << br << substmt;
    }
    out << ")";
    indentLevel--;
    out << br << "(else";
    indentLevel++;
    for (const Stmt& substmt : stmt.elseBody) {
        if (!substmt.isIfStmt()) out << br;
        out << substmt;
    }
    indentLevel--;
    out << ")";
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const SwitchStmt& stmt) {
    out << br << "(switch-stmt " << *stmt.condition;
    indentLevel++;
    for (const SwitchCase& switchCase : stmt.cases) {
        out << br << "(case " << *switchCase.value;
        indentLevel++;
        for (const Stmt& substmt : switchCase.stmts) {
            out << br << substmt;
        }
        indentLevel--;
        out << ")";
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const WhileStmt& stmt) {
    out << br << "(while-stmt " << *stmt.condition;
    indentLevel++;
    for (const Stmt& substmt : stmt.body) {
        out << br << substmt;
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const BreakStmt&) {
    return out << "(break-stmt)";
}

std::ostream& operator<<(std::ostream& out, const AssignStmt& stmt) {
    return out << br << "(assign-stmt " << *stmt.lhs << " " << *stmt.rhs << ")";
}

std::ostream& operator<<(std::ostream& out, const AugAssignStmt& stmt) {
    return out << br << "(comp-assign-stmt " << stmt.op << " " << *stmt.lhs << " " << *stmt.rhs << ")";
}

std::ostream& operator<<(std::ostream& out, const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    return out << stmt.getReturnStmt();
        case StmtKind::VariableStmt:  return out << stmt.getVariableStmt();
        case StmtKind::IncrementStmt: return out << stmt.getIncrementStmt();
        case StmtKind::DecrementStmt: return out << stmt.getDecrementStmt();
        case StmtKind::ExprStmt:      return out << *stmt.getExprStmt().expr;
        case StmtKind::DeferStmt:     return out << stmt.getDeferStmt();
        case StmtKind::IfStmt:        return out << stmt.getIfStmt();
        case StmtKind::SwitchStmt:    return out << stmt.getSwitchStmt();
        case StmtKind::WhileStmt:     return out << stmt.getWhileStmt();
        case StmtKind::BreakStmt:     return out << stmt.getBreakStmt();
        case StmtKind::AssignStmt:    return out << stmt.getAssignStmt();
        case StmtKind::AugAssignStmt: return out << stmt.getAugAssignStmt();
    }
}

std::ostream& operator<<(std::ostream& out, const ParamDecl& decl) {
    return out << "(" << decl.type << " " << decl.name << ")";
}

std::ostream& printFuncBase(std::ostream& out, const FuncDecl& decl) {
    out << " (";
    for (const ParamDecl& param : decl.params) {
        out << param;
        if (&param != &decl.params.back()) out << " ";
    }
    out << ") " << decl.returnType;

    if (!decl.isExtern()) {
        indentLevel++;
        for (const Stmt& stmt : *decl.body) {
            out << stmt;
        }
        indentLevel--;
    }

    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const FuncDecl& decl) {
    out << br << (decl.isExtern() ? "(extern-func-decl " : "(func-decl ");
    out << decl.name;
    return printFuncBase(out, decl);
}

std::ostream& operator<<(std::ostream& out, const GenericFuncDecl& decl) {
    out << br << (decl.func->isExtern() ? "(extern-generic-func-decl " : "(generic-func-decl ");
    out << decl.func->name << " (";
    for (const GenericParamDecl& genericParam : decl.genericParams) {
        out << genericParam.name;
        if (&genericParam != &decl.genericParams.back()) out << " ";
    }
    out << ")";
    return printFuncBase(out, *decl.func);
}

std::ostream& operator<<(std::ostream& out, const InitDecl& decl) {
    out << br << "(init-decl " << decl.getTypeDecl().name << " (";
    for (const ParamDecl& param : decl.params) {
        out << param;
        if (&param != &decl.params.back()) out << " ";
    }
    out << ")";
    indentLevel++;
    for (const Stmt& stmt : *decl.body) {
        out << stmt;
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const DeinitDecl& decl) {
    out << br << "(deinit-decl " << decl.getTypeDecl().name;
    indentLevel++;
    for (const Stmt& stmt : *decl.body) {
        out << stmt;
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const FieldDecl& decl) {
    return out << br << "(field-decl " << decl.type << " " << decl.name << ")";
}

std::ostream& operator<<(std::ostream& out, const TypeDecl& decl) {
    out << br << "(type-decl ";
    switch (decl.tag) {
        case TypeTag::Struct: out << "struct "; break;
        case TypeTag::Class: out << "class "; break;
    }
    out << decl.name;
    indentLevel++;
    for (const FieldDecl& field : decl.fields) {
        out << field;
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const VarDecl& decl) {
    return out << br << "(var-decl " << decl.name << " " << *decl.initializer << ")";
}

std::ostream& operator<<(std::ostream& out, const ImportDecl& decl) {
    return out << br << "(import-decl \"" << decl.target << "\")";
}

std::ostream& operator<<(std::ostream& out, const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: return out << decl.getParamDecl();
        case DeclKind::FuncDecl:  return out << decl.getFuncDecl();
        case DeclKind::GenericParamDecl: /* handled via GenericFuncDecl */ assert(false); break;
        case DeclKind::GenericFuncDecl: return out << decl.getGenericFuncDecl();
        case DeclKind::InitDecl:  return out << decl.getInitDecl();
        case DeclKind::DeinitDecl:return out << decl.getDeinitDecl();
        case DeclKind::TypeDecl:  return out << decl.getTypeDecl();
        case DeclKind::VarDecl:   return out << decl.getVarDecl();
        case DeclKind::FieldDecl: return out << decl.getFieldDecl();
        case DeclKind::ImportDecl:return out << decl.getImportDecl();
    }
}

} // anonymous namespace

std::ostream& delta::operator<<(std::ostream& out, const AST& decls) {
    out << "(source-file";
    indentLevel++;
    for (const Decl& decl : decls) {
        ::operator<<(out, decl);
    }
    indentLevel--;
    return out << ")\n";
}
