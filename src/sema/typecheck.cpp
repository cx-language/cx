#include <iostream>
#include <limits>
#include <unordered_map>
#include <boost/utility/string_ref.hpp>
#include <boost/optional.hpp>
#include "typecheck.h"
#include "../ast/type.h"
#include "../ast/expr.h"
#include "../ast/decl.h"

static std::unordered_map<std::string, Type> symbolTable;
static const Type* funcReturnType = nullptr;

template<typename... Args>
[[noreturn]] static void error(Args&&... args) {
    std::cout << "error: ";
    using expander = int[];
    (void)expander{0, (void(std::cout << std::forward<Args>(args)), 0)...};
    std::cout << '\n';
    exit(1);
}

const Type& typecheck(Expr& expr);
void typecheck(Stmt& stmt);

Type typecheck(VariableExpr& expr) {
    auto it = symbolTable.find(expr.identifier);
    if (it == symbolTable.end()) {
        error("unknown identifier '", expr.identifier, "'");
    }
    return it->second;
}

Type typecheck(StrLiteralExpr& expr) {
    return Type(PtrType{std::unique_ptr<Type>(new Type(BasicType{"char"}))});
}

Type typecheck(IntLiteralExpr& expr) {
    if (expr.value >= std::numeric_limits<int32_t>::min()
    &&  expr.value <= std::numeric_limits<int32_t>::max())
        return Type(BasicType{"int"});
    else
    if (expr.value >= std::numeric_limits<int64_t>::min()
    &&  expr.value <= std::numeric_limits<int64_t>::max())
        return Type(BasicType{"int64"});
    else
        error("integer literal is too large");
}

Type typecheck(BoolLiteralExpr&) {
    return Type(BasicType{"bool"});
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
    return expr.op.isComparisonOperator() ? Type(BasicType{"bool"}) : leftType;
}

Type typecheck(CallExpr& expr) {
    auto it = symbolTable.find(expr.funcName);
    if (it == symbolTable.end()) {
        error("unknown function '", expr.funcName, "'");
    }
    if (it->second.getKind() != TypeKind::FuncType) {
        error("'", expr.funcName, "' is not a function");
    }
    const auto& params = it->second.getFuncType().paramTypes;
    if (expr.args.size() < params.size()) {
        error("too few arguments to '", expr.funcName, "', expected ", params.size());
    }
    if (expr.args.size() > params.size()) {
        error("too many arguments to '", expr.funcName, "', expected ", params.size());
    }
    for (int i = 0; i < params.size(); ++i) {
        auto argType = typecheck(expr.args[i]);
        if (argType != params[i]) {
            error("invalid argument #", i + 1, " type '", argType, "' to '",
                expr.funcName, "', expected '", params[i], "'");
        }
    }
    return Type(TupleType{it->second.getFuncType().returnTypes});
}

const Type& typecheck(Expr& expr) {
    boost::optional<Type> type;
    switch (expr.getKind()) {
        case ExprKind::VariableExpr:    type = typecheck(expr.getVariableExpr()); break;
        case ExprKind::StrLiteralExpr:  type = typecheck(expr.getStrLiteralExpr()); break;
        case ExprKind::IntLiteralExpr:  type = typecheck(expr.getIntLiteralExpr()); break;
        case ExprKind::BoolLiteralExpr: type = typecheck(expr.getBoolLiteralExpr()); break;
        case ExprKind::PrefixExpr:      type = typecheck(expr.getPrefixExpr()); break;
        case ExprKind::BinaryExpr:      type = typecheck(expr.getBinaryExpr()); break;
        case ExprKind::CallExpr:        type = typecheck(expr.getCallExpr()); break;
    }
    expr.setType(std::move(*type));
    return expr.getType();
}

void typecheck(ReturnStmt& stmt) {
    std::vector<Type> returnValueTypes;
    for (Expr& expr : stmt.values) {
        returnValueTypes.push_back(typecheck(expr));
    }
    Type returnType = returnValueTypes.size() > 1
        ? Type(TupleType{std::move(returnValueTypes)}) : std::move(returnValueTypes[0]);
    if (returnType != *funcReturnType) {
        error("mismatching return type '", returnType, "', expected '", *funcReturnType, "'");
    }
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

void typecheck(IfStmt& stmt) {
    const Type& conditionType = typecheck(stmt.condition);
    if (conditionType != Type(BasicType{"bool"})) {
        error("'if' condition must have type 'bool'");
    }
    for (Stmt& stmt : stmt.thenBody) typecheck(stmt);
    for (Stmt& stmt : stmt.elseBody) typecheck(stmt);
}

void typecheck(WhileStmt& stmt) {
    const Type& conditionType = typecheck(stmt.condition);
    if (conditionType != Type(BasicType{"bool"})) {
        error("'while' condition must have type 'bool'");
    }
    for (Stmt& stmt : stmt.body) typecheck(stmt);
}

void typecheck(Stmt& stmt) {
    switch (stmt.getKind()) {
        case StmtKind::ReturnStmt:    typecheck(stmt.getReturnStmt()); break;
        case StmtKind::VariableStmt:  typecheck(stmt.getVariableStmt()); break;
        case StmtKind::IncrementStmt: typecheck(stmt.getIncrementStmt()); break;
        case StmtKind::DecrementStmt: typecheck(stmt.getDecrementStmt()); break;
        case StmtKind::CallStmt:      typecheck(stmt.getCallStmt().expr); break;
        case StmtKind::IfStmt:        typecheck(stmt.getIfStmt()); break;
        case StmtKind::WhileStmt:     typecheck(stmt.getWhileStmt()); break;
    }
}

void typecheck(ParamDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    symbolTable.insert({decl.name, Type(decl.type)});
}

static std::vector<Type> mapToTypes(const std::vector<ParamDecl>& params) {
    std::vector<Type> paramTypes;
    paramTypes.reserve(params.size());
    for (const auto& param : params) paramTypes.emplace_back(param.type);
    return paramTypes;
}

void addToSymbolTable(const FuncDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    auto returnTypes = decl.returnType.getKind() == TypeKind::TupleType
        ? decl.returnType.getTupleType().subtypes : std::vector<Type>{decl.returnType};
    symbolTable.insert({decl.name, Type(FuncType{returnTypes, mapToTypes(decl.params)})});
}

void typecheck(FuncDecl& decl) {
    if (decl.isExtern()) return;
    auto symbolTableBackup = symbolTable;
    for (ParamDecl& param : decl.params) typecheck(param);
    funcReturnType = &decl.returnType;
    for (Stmt& stmt : *decl.body) typecheck(stmt);
    funcReturnType = nullptr;
    symbolTable = std::move(symbolTableBackup);
}

void typecheck(VarDecl& decl) {
    if (symbolTable.count(decl.name) > 0) {
        error("redefinition of '", decl.name, "'");
    }
    auto initType = typecheck(decl.initializer);
    if (decl.type && *decl.type != initType) {
        error("cannot initialize variable of type '", *decl.type, "' with '", initType, "'");
    }
    symbolTable.insert({decl.name, std::move(initType)});
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
