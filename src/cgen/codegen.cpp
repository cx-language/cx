#include <iostream>
#include <fstream>
#include <functional>
#include "codegen.h"

static std::ostream* out = nullptr;

/// Points to the function whose body is currently being generated.
static const FuncDecl* currentFunc = nullptr;

std::string toString(const Type& type) {
    if (type.isTuple()) {
        std::string string = "struct{";
        int index = 0;
        for (const Type& subtype : type.getNames()) {
            string += toString(subtype) + " _" + std::to_string(index++) + ";";
        }
        return string + "}";
    } else {
        return type.getName();
    }
}

void codegen(const Expr& expr);

void codegen(const VariableExpr& expr) {
    *out << expr.identifier;
}

void codegen(const IntLiteralExpr& expr) {
    *out << expr.value;
}

void codegen(const PrefixExpr& expr) {
    *out << "(" << expr.op;
    codegen(*expr.operand);
    *out << ")";
}

void codegen(const BinaryExpr& expr) {
    *out << "(";
    codegen(*expr.left);
    *out << expr.op;
    codegen(*expr.right);
    *out << ")";
}

void codegen(const Expr& expr) {
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:   codegen(expr.getVariableExpr()); break;
        case ExprKind::IntLiteralExpr: codegen(expr.getIntLiteralExpr()); break;
        case ExprKind::PrefixExpr:     codegen(expr.getPrefixExpr()); break;
        case ExprKind::BinaryExpr:     codegen(expr.getBinaryExpr()); break;
    }
}

void codegen(const ReturnStmt& stmt) {
    *out << "return ";
    if (stmt.values.size() == 1) {
        codegen(stmt.values[0]);
    } else if (stmt.values.size() > 1) {
        *out << "(__typeof__(" << currentFunc->name << "())){";
        for (const Expr& expr : stmt.values) {
            codegen(stmt.values[0]);
            if (&expr != &stmt.values.back()) *out << ",";
        }
        *out << "}";
    }
    *out << ";";
}

void codegen(const VarDecl& decl);

void codegen(const VariableStmt& stmt) {
    codegen(*stmt.decl);
}

void codegen(const IncrementStmt& stmt) {
    codegen(stmt.operand);
    *out << "++;";
}

void codegen(const DecrementStmt& stmt) {
    codegen(stmt.operand);
    *out << "--;";
}

void codegen(const Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    codegen(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt:  codegen(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: codegen(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: codegen(stmt.getDecrementStmt()); break;
    }
}

void codegen(const ParamDecl& decl) {
    *out << decl.type << " " << decl.name;
}

void codegen(const FuncDecl& decl) {
    *out << toString(decl.returnType) << " " << decl.name << "(";
    for (const ParamDecl& param : decl.params) {
        codegen(param);
        if (&param != &decl.params.back()) *out << ",";
    }
    *out << "){";
    auto currentFuncBackup = currentFunc;
    currentFunc = &decl;
    for (const Stmt& stmt : decl.body) {
        codegen(stmt);
    }
    currentFunc = currentFuncBackup;
    *out << "}";
}

void codegen(const VarDecl& decl) {
    *out << toString(decl.initializer.getType()) << " " << decl.name << "=";
    codegen(decl.initializer);
    *out << ";";
}

void codegen(const Decl& decl) {
    switch (decl.getKind()) {
        case DeclKind::ParamDecl: codegen(decl.getParamDecl()); break;
        case DeclKind::FuncDecl:  codegen(decl.getFuncDecl()); break;
        case DeclKind::VarDecl:   codegen(decl.getVarDecl()); break;
    }
}

void cgen::compile(const std::vector<Decl>& decls, boost::string_ref outputPath) {
    std::ofstream outputFile;
    if (outputPath == "stdout") out = &std::cout;
    else {
        outputFile.open(outputPath.data());
        out = &outputFile;
    }

    *out << "#include <stdbool.h>\n";
    for (const Decl& decl : decls) {
        codegen(decl);
    }
}
