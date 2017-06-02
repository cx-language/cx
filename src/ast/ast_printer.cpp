#include <iostream>
#include <string>
#include <llvm/Support/ErrorHandling.h>
#include "ast_printer.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"
#include "../ast/module.h"
#include "../driver/utility.h"

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

std::ostream& operator<<(std::ostream& out, const FloatLiteralExpr& expr) {
    return out << expr.value;
}

std::ostream& operator<<(std::ostream& out, const BoolLiteralExpr& expr) {
    return out << (expr.value ? "true" : "false");
}

std::ostream& operator<<(std::ostream& out, const NullLiteralExpr&) {
    return out << "null";
}

std::ostream& operator<<(std::ostream& out, const ArrayLiteralExpr& expr) {
    out << "(array-literal";
    for (auto& e : expr.elements) out << " " << *e;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const PrefixExpr& expr) {
    return out << "(" << expr.op << expr.getOperand() << ")";
}

std::ostream& operator<<(std::ostream& out, const BinaryExpr& expr) {
    return out << "(" << expr.getLHS() << " " << expr.op << " " << expr.getRHS() << ")";
}

std::ostream& operator<<(std::ostream& out, const CallExpr& expr) {
    out << "(call " << *expr.func;
    for (const Arg& arg : expr.args) {
        out << " " << *arg.value;
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

std::ostream& operator<<(std::ostream& out, const UnwrapExpr& expr) {
    return out << "(unwrap " << *expr.operand << ")";
}

std::ostream& operator<<(std::ostream& out, const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:   return out << expr.getVariableExpr();
        case ExprKind::StrLiteralExpr: return out << expr.getStrLiteralExpr();
        case ExprKind::IntLiteralExpr: return out << expr.getIntLiteralExpr();
        case ExprKind::FloatLiteralExpr:return out << expr.getFloatLiteralExpr();
        case ExprKind::BoolLiteralExpr:return out << expr.getBoolLiteralExpr();
        case ExprKind::NullLiteralExpr:return out << expr.getNullLiteralExpr();
        case ExprKind::ArrayLiteralExpr:return out<< expr.getArrayLiteralExpr();
        case ExprKind::PrefixExpr:     return out << expr.getPrefixExpr();
        case ExprKind::BinaryExpr:     return out << expr.getBinaryExpr();
        case ExprKind::CallExpr:       return out << expr.getCallExpr();
        case ExprKind::CastExpr:       return out << expr.getCastExpr();
        case ExprKind::MemberExpr:     return out << expr.getMemberExpr();
        case ExprKind::SubscriptExpr:  return out << expr.getSubscriptExpr();
        case ExprKind::UnwrapExpr:     return out << expr.getUnwrapExpr();
    }
    llvm_unreachable("all cases handled");
}

std::ostream& operator<<(std::ostream& out, llvm::ArrayRef<std::unique_ptr<Stmt>> block) {
    indentLevel++;
    for (const auto& stmt : block) {
        out << *stmt;
    }
    indentLevel--;
    return out;
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
    out << br << "(var-stmt " << stmt.decl->name << " ";
    if (stmt.decl->initializer) out << *stmt.decl->initializer;
    else out << "uninitialized";
    return out << ")";
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
    out << br << "(then" << stmt.thenBody << ")";
    if (!stmt.elseBody.empty()) {
        out << br << "(else";
        indentLevel++;
        for (const auto& substmt : stmt.elseBody) {
            if (!substmt->isIfStmt()) out << br;
            out << *substmt;
        }
        indentLevel--;
        out << ")";
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const SwitchStmt& stmt) {
    out << br << "(switch-stmt " << *stmt.condition;
    indentLevel++;
    for (const SwitchCase& switchCase : stmt.cases) {
        out << br << "(case " << *switchCase.value << switchCase.stmts << ")";
    }
    indentLevel--;
    return out << ")";
}

std::ostream& operator<<(std::ostream& out, const WhileStmt& stmt) {
    return out << br << "(while-stmt " << *stmt.condition << stmt.body << ")";
}

std::ostream& operator<<(std::ostream& out, const ForStmt& stmt) {
    return out << br << "(for-stmt " << stmt.id << " " << *stmt.range << stmt.body << ")";
}

std::ostream& operator<<(std::ostream& out, const BreakStmt&) {
    return out << br << "(break-stmt)";
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
        case StmtKind::ExprStmt:      return out << br << *stmt.getExprStmt().expr;
        case StmtKind::DeferStmt:     return out << stmt.getDeferStmt();
        case StmtKind::IfStmt:        return out << stmt.getIfStmt();
        case StmtKind::SwitchStmt:    return out << stmt.getSwitchStmt();
        case StmtKind::WhileStmt:     return out << stmt.getWhileStmt();
        case StmtKind::ForStmt:       return out << stmt.getForStmt();
        case StmtKind::BreakStmt:     return out << stmt.getBreakStmt();
        case StmtKind::AssignStmt:    return out << stmt.getAssignStmt();
        case StmtKind::AugAssignStmt: return out << stmt.getAugAssignStmt();
    }
    llvm_unreachable("all cases handled");
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
    if (!decl.isExtern()) out << *decl.body;
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
    return out << ")" << *decl.body << ")";
}

std::ostream& operator<<(std::ostream& out, const DeinitDecl& decl) {
    return out << br << "(deinit-decl " << decl.getTypeDecl().name << *decl.body << ")";
}

std::ostream& operator<<(std::ostream& out, const FieldDecl& decl) {
    return out << br << "(field-decl " << decl.type << " " << decl.name << ")";
}

std::ostream& operator<<(std::ostream& out, const TypeDecl& decl) {
    out << br << "(type-decl ";
    switch (decl.tag) {
        case TypeTag::Struct: out << "struct "; break;
        case TypeTag::Class: out << "class "; break;
        case TypeTag::Interface: out << "interface "; break;
        case TypeTag::Union: out << "union "; break;
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
        case DeclKind::GenericParamDecl: llvm_unreachable("handled via GenericFuncDecl");
        case DeclKind::GenericFuncDecl: return out << decl.getGenericFuncDecl();
        case DeclKind::InitDecl:  return out << decl.getInitDecl();
        case DeclKind::DeinitDecl:return out << decl.getDeinitDecl();
        case DeclKind::TypeDecl:  return out << decl.getTypeDecl();
        case DeclKind::VarDecl:   return out << decl.getVarDecl();
        case DeclKind::FieldDecl: return out << decl.getFieldDecl();
        case DeclKind::ImportDecl:return out << decl.getImportDecl();
    }
    llvm_unreachable("all cases handled");
}

} // anonymous namespace

std::ostream& delta::operator<<(std::ostream& out, const Module& module) {
    for (const auto& fileUnit : module.getFileUnits()) {
        out << "(source-file " << fileUnit.getFilePath();
        indentLevel++;
        for (const auto& decl : fileUnit.getTopLevelDecls()) {
            ::operator<<(out, *decl);
        }
        indentLevel--;
        out << ")\n";
    }
    return out;
}
