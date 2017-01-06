#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <boost/variant.hpp>

class Expr;

enum class ExprType {
    VariableExpr,
    IntLiteralExpr,
    PrefixExpr,
    BinaryExpr,
};

struct VariableExpr {
    std::string identifier;
};

struct IntLiteralExpr {
    int64_t value;
};

struct PrefixExpr {
    char op;
    std::unique_ptr<Expr> operand;
};

struct BinaryExpr {
    char op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

class Expr {
public:
#define DEFINE_EXPR_TYPE_GETTER_AND_CONSTRUCTOR(type, member) \
    Expr(type&& value) : data(std::move(value)) { } \
    \
    type& get##type() { \
        assert(getType() == ExprType::type); \
        return boost::get<type>(data); \
    } \
    const type& get##type() const { \
        assert(getType() == ExprType::type); \
        return boost::get<type>(data); \
    }
    DEFINE_EXPR_TYPE_GETTER_AND_CONSTRUCTOR(VariableExpr, variableExpr)
    DEFINE_EXPR_TYPE_GETTER_AND_CONSTRUCTOR(IntLiteralExpr, intLiteralExpr)
    DEFINE_EXPR_TYPE_GETTER_AND_CONSTRUCTOR(PrefixExpr, prefixExpr)
    DEFINE_EXPR_TYPE_GETTER_AND_CONSTRUCTOR(BinaryExpr, binaryExpr)
#undef DEFINE_EXPR_TYPE_GETTER_AND_CONSTRUCTOR

    Expr(Expr&& expr) : data(std::move(expr.data)) { }
    ExprType getType() const { return static_cast<ExprType>(data.which()); }

private:
    boost::variant<VariableExpr, IntLiteralExpr, PrefixExpr, BinaryExpr> data;
};
