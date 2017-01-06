#pragma once

#include <vector>
#include <memory>
#include <boost/variant.hpp>
#include "expr.h"

class VarDecl;

enum class StmtType {
    ReturnStmt,
    VariableStmt,
    IncrementStmt,
    DecrementStmt,
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

class Stmt {
public:
#define DEFINE_STMT_TYPE_GETTER_AND_CONSTRUCTOR(type, member) \
    Stmt(type&& value) : data(std::move(value)) { } \
    \
    type& get##type() { \
        assert(getType() == StmtType::type); \
        return boost::get<type>(data); \
    } \
    const type& get##type() const { \
        assert(getType() == StmtType::type); \
        return boost::get<type>(data); \
    }
    DEFINE_STMT_TYPE_GETTER_AND_CONSTRUCTOR(ReturnStmt, returnStmt)
    DEFINE_STMT_TYPE_GETTER_AND_CONSTRUCTOR(VariableStmt, variableStmt)
    DEFINE_STMT_TYPE_GETTER_AND_CONSTRUCTOR(IncrementStmt, incrementStmt)
    DEFINE_STMT_TYPE_GETTER_AND_CONSTRUCTOR(DecrementStmt, decrementStmt)
#undef DEFINE_STMT_TYPE_GETTER_AND_CONSTRUCTOR

    Stmt(Stmt&& stmt) : data(std::move(stmt.data)) { }
    StmtType getType() const { return static_cast<StmtType>(data.which()); }

private:
    boost::variant<ReturnStmt, VariableStmt, IncrementStmt, DecrementStmt> data;
};
