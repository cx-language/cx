#include <iostream>
#include <string>
#include <llvm/Support/ErrorHandling.h>
#include "ast_printer.h"
#include "../ast/decl.h"
#include "../ast/stmt.h"
#include "../ast/module.h"
#include "../support/utility.h"

using namespace delta;

namespace {

int indentLevel = 0;

/// Inserts a line break followed by appropriate indentation.
std::ostream& br(std::ostream& out) {
    return out << '\n' << std::string(indentLevel * 4, ' ');
}

std::ostream& operator<<(std::ostream& out, const Expr& expr);
std::ostream& operator<<(std::ostream& out, const Stmt& stmt);

std::ostream& operator<<(std::ostream& out, const VarExpr& expr) {
    return out << expr.identifier;
}

std::ostream& operator<<(std::ostream& out, const StringLiteralExpr& expr) {
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
    out << "(call " << expr.getCallee();
    for (const Argument& arg : expr.getArgs()) {
        out << " " << *arg.getValue();
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
        case ExprKind::VarExpr: return out << llvm::cast<VarExpr>(expr);
        case ExprKind::StringLiteralExpr: return out << llvm::cast<StringLiteralExpr>(expr);
        case ExprKind::IntLiteralExpr: return out << llvm::cast<IntLiteralExpr>(expr);
        case ExprKind::FloatLiteralExpr: return out << llvm::cast<FloatLiteralExpr>(expr);
        case ExprKind::BoolLiteralExpr: return out << llvm::cast<BoolLiteralExpr>(expr);
        case ExprKind::NullLiteralExpr: return out << llvm::cast<NullLiteralExpr>(expr);
        case ExprKind::ArrayLiteralExpr: return out << llvm::cast<ArrayLiteralExpr>(expr);
        case ExprKind::PrefixExpr: return out << llvm::cast<PrefixExpr>(expr);
        case ExprKind::BinaryExpr: return out << llvm::cast<BinaryExpr>(expr);
        case ExprKind::CallExpr: return out << llvm::cast<CallExpr>(expr);
        case ExprKind::CastExpr: return out << llvm::cast<CastExpr>(expr);
        case ExprKind::MemberExpr: return out << llvm::cast<MemberExpr>(expr);
        case ExprKind::SubscriptExpr: return out << llvm::cast<SubscriptExpr>(expr);
        case ExprKind::UnwrapExpr: return out << llvm::cast<UnwrapExpr>(expr);
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

std::ostream& operator<<(std::ostream& out, const VarStmt& stmt) {
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
        case StmtKind::ReturnStmt: return out << llvm::cast<ReturnStmt>(stmt);
        case StmtKind::VarStmt: return out << llvm::cast<VarStmt>(stmt);
        case StmtKind::IncrementStmt: return out << llvm::cast<IncrementStmt>(stmt);
        case StmtKind::DecrementStmt: return out << llvm::cast<DecrementStmt>(stmt);
        case StmtKind::ExprStmt: return out << br << *llvm::cast<ExprStmt>(stmt).expr;
        case StmtKind::DeferStmt: return out << llvm::cast<DeferStmt>(stmt);
        case StmtKind::IfStmt: return out << llvm::cast<IfStmt>(stmt);
        case StmtKind::SwitchStmt: return out << llvm::cast<SwitchStmt>(stmt);
        case StmtKind::WhileStmt: return out << llvm::cast<WhileStmt>(stmt);
        case StmtKind::ForStmt: return out << llvm::cast<ForStmt>(stmt);
        case StmtKind::BreakStmt: return out << llvm::cast<BreakStmt>(stmt);
        case StmtKind::AssignStmt: return out << llvm::cast<AssignStmt>(stmt);
        case StmtKind::AugAssignStmt: return out << llvm::cast<AugAssignStmt>(stmt);
    }
    llvm_unreachable("all cases handled");
}

std::ostream& operator<<(std::ostream& out, const ParamDecl& decl) {
    return out << "(" << decl.type << " " << decl.name << ")";
}

std::ostream& operator<<(std::ostream& out, const FunctionDecl& decl) {
    out << br << (decl.isExtern() ? "(extern-function-decl " : "(function-decl ");
    out << decl.name;

    if (!decl.genericParams.empty()) {
        out << " (generic-params ";
        for (const GenericParamDecl& genericParam : decl.genericParams) {
            out << genericParam.name;
            if (&genericParam != &decl.genericParams.back()) out << " ";
        }
        out << ")";
    }

    out << " (";
    for (const ParamDecl& param : decl.params) {
        out << param;
        if (&param != &decl.params.back()) out << " ";
    }
    out << ") " << decl.returnType;

    if (!decl.isExtern()) out << *decl.body;
    return out << ")";
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
        case DeclKind::ParamDecl: return out << llvm::cast<ParamDecl>(decl);
        case DeclKind::FunctionDecl: return out << llvm::cast<FunctionDecl>(decl);
        case DeclKind::GenericParamDecl: llvm_unreachable("handled via FunctionDecl");
        case DeclKind::InitDecl: return out << llvm::cast<InitDecl>(decl);
        case DeclKind::DeinitDecl: return out << llvm::cast<DeinitDecl>(decl);
        case DeclKind::TypeDecl: return out << llvm::cast<TypeDecl>(decl);
        case DeclKind::VarDecl: return out << llvm::cast<VarDecl>(decl);
        case DeclKind::FieldDecl: return out << llvm::cast<FieldDecl>(decl);
        case DeclKind::ImportDecl: return out << llvm::cast<ImportDecl>(decl);
    }
    llvm_unreachable("all cases handled");
}

} // anonymous namespace

std::ostream& delta::operator<<(std::ostream& out, const Module& module) {
    for (const auto& sourceFile : module.getSourceFiles()) {
        out << "(source-file " << sourceFile.getFilePath();
        indentLevel++;
        for (const auto& decl : sourceFile.getTopLevelDecls()) {
            ::operator<<(out, *decl);
        }
        indentLevel--;
        out << ")\n";
    }
    return out;
}
