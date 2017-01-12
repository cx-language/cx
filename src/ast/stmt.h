#pragma once

#include <vector>
#include <memory>
#include <boost/variant.hpp>
#include "expr.h"

struct VarDecl;
class Stmt;

enum class StmtKind {
    ReturnStmt,
    VariableStmt,
    IncrementStmt,
    DecrementStmt,
    CallStmt,
    IfStmt,
    WhileStmt,
    AssignStmt,
};

struct ReturnStmt {
    std::vector<Expr> values;
};

struct VariableStmt {
    VarDecl* decl; // FIXME: decl is owned.
};

struct IncrementStmt {
    Expr operand;
};

struct DecrementStmt {
    Expr operand;
};

struct CallStmt {
    CallExpr expr;
};

struct IfStmt {
    Expr condition;
    std::vector<Stmt> thenBody;
    std::vector<Stmt> elseBody;
};

struct WhileStmt {
    Expr condition;
    std::vector<Stmt> body;
};

struct AssignStmt {
    Expr lhs;
    Expr rhs;
};

class Stmt {
public:
#define DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(KIND) \
    Stmt(KIND&& value) : data(std::move(value)) { } \
    \
    KIND& get##KIND() { \
        assert(getKind() == StmtKind::KIND); \
        return boost::get<KIND>(data); \
    } \
    const KIND& get##KIND() const { \
        assert(getKind() == StmtKind::KIND); \
        return boost::get<KIND>(data); \
    }
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(ReturnStmt)
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(VariableStmt)
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(IncrementStmt)
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(DecrementStmt)
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(CallStmt)
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(IfStmt)
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(WhileStmt)
    DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR(AssignStmt)
#undef DEFINE_STMTKIND_GETTER_AND_CONSTRUCTOR

    Stmt(Stmt&& stmt) : data(std::move(stmt.data)) { }
    StmtKind getKind() const { return static_cast<StmtKind>(data.which()); }

private:
    boost::variant<ReturnStmt, VariableStmt, IncrementStmt, DecrementStmt,
        CallStmt, IfStmt, WhileStmt, AssignStmt> data;
};
