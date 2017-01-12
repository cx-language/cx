#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include "type.h"
#include "../parser/operators.h"

class Expr;

enum class ExprKind {
    VariableExpr,
    StrLiteralExpr,
    IntLiteralExpr,
    BoolLiteralExpr,
    PrefixExpr,
    BinaryExpr,
    CallExpr,
    MemberExpr,
};

struct VariableExpr {
    std::string identifier;
};

struct StrLiteralExpr {
    std::string value;
};

struct IntLiteralExpr {
    int64_t value;
};

struct BoolLiteralExpr {
    bool value;
};

struct PrefixExpr {
    PrefixOperator op;
    std::unique_ptr<Expr> operand;
};

struct BinaryExpr {
    BinaryOperator op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
};

struct CallExpr {
    std::string funcName;
    std::vector<Expr> args;
    bool isInitializerCall;
};

/// A member access expression using the dot syntax, such as 'a.b'.
struct MemberExpr {
    std::string base;
    std::string member;
};

class Expr {
public:
#define DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(KIND) \
    Expr(KIND&& value) : data(std::move(value)) { } \
    \
    KIND& get##KIND() { \
        assert(getKind() == ExprKind::KIND); \
        return boost::get<KIND>(data); \
    } \
    const KIND& get##KIND() const { \
        assert(getKind() == ExprKind::KIND); \
        return boost::get<KIND>(data); \
    }
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(VariableExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(StrLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(IntLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(BoolLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(PrefixExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(BinaryExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(CallExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(MemberExpr)
#undef DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR

    Expr(Expr&& expr) : data(std::move(expr.data)) { }
    ExprKind getKind() const { return static_cast<ExprKind>(data.which()); }
    const Type& getType() const { return *type; }
    void setType(Type t) { type = std::move(t); }

private:
    boost::variant<VariableExpr, StrLiteralExpr, IntLiteralExpr, BoolLiteralExpr,
        PrefixExpr, BinaryExpr, CallExpr, MemberExpr> data;
    boost::optional<Type> type;
};
