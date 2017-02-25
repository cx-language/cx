#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <vector>
#include <boost/variant.hpp>
#include <boost/optional.hpp>
#include "type.h"
#include "srcloc.h"
#include "../parser/operators.h"

namespace delta {

class Expr;

enum class ExprKind {
    VariableExpr,
    StrLiteralExpr,
    IntLiteralExpr,
    BoolLiteralExpr,
    NullLiteralExpr,
    ArrayLiteralExpr,
    PrefixExpr,
    BinaryExpr,
    CallExpr,
    CastExpr,
    MemberExpr,
    SubscriptExpr,
};

struct VariableExpr {
    std::string identifier;
    SrcLoc srcLoc;
};

struct StrLiteralExpr {
    std::string value;
    SrcLoc srcLoc;
};

struct IntLiteralExpr {
    int64_t value;
    SrcLoc srcLoc;
};

struct BoolLiteralExpr {
    bool value;
    SrcLoc srcLoc;
};

struct NullLiteralExpr {
    SrcLoc srcLoc;
};

struct ArrayLiteralExpr {
    std::vector<Expr> elements;
    SrcLoc srcLoc;
};

struct PrefixExpr {
    PrefixOperator op;
    std::unique_ptr<Expr> operand;
    SrcLoc srcLoc;
};

struct BinaryExpr {
    BinaryOperator op;
    std::unique_ptr<Expr> left;
    std::unique_ptr<Expr> right;
    SrcLoc srcLoc;
};

struct Arg {
    std::string label; // Empty if no label.
    std::unique_ptr<Expr> value;
    SrcLoc srcLoc;
};

struct CallExpr {
    std::string funcName;
    std::vector<Arg> args;
    bool isInitializerCall;
    std::unique_ptr<Expr> receiver; /// Null if non-member function call.
    SrcLoc srcLoc;

    bool isMemberFuncCall() const { return receiver != nullptr; }
};

/// A type cast expression using the 'cast' keyword, e.g. 'cast<type>(expr)'.
struct CastExpr {
    Type type;
    std::unique_ptr<Expr> expr;
    SrcLoc srcLoc;
};

/// A member access expression using the dot syntax, such as 'a.b'.
struct MemberExpr {
    std::string base;
    std::string member;
    SrcLoc baseSrcLoc;
    SrcLoc memberSrcLoc;
};

/// An array element access expression using the element's index in brackets, e.g. 'array[index]'.
struct SubscriptExpr {
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;
    SrcLoc srcLoc;
};

class Expr {
public:
#define DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(KIND) \
    Expr(KIND&& value) : data(std::move(value)) { } \
    \
    bool is##KIND() const { return getKind() == ExprKind::KIND; } \
    \
    KIND& get##KIND() { \
        assert(is##KIND()); \
        return boost::get<KIND>(data); \
    } \
    const KIND& get##KIND() const { \
        assert(is##KIND()); \
        return boost::get<KIND>(data); \
    }
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(VariableExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(StrLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(IntLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(BoolLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(NullLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(ArrayLiteralExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(PrefixExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(BinaryExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(CallExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(CastExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(MemberExpr)
    DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR(SubscriptExpr)
#undef DEFINE_EXPRKIND_GETTER_AND_CONSTRUCTOR

    Expr(Expr&&) = default;
    ExprKind getKind() const { return static_cast<ExprKind>(data.which()); }
    const Type& getType() const { return *type; }
    void setType(Type t) { type = std::move(t); }
    bool isLvalue() const;
    bool isRvalue() const { return !isLvalue(); }
    SrcLoc getSrcLoc() const;

private:
    boost::variant<VariableExpr, StrLiteralExpr, IntLiteralExpr, BoolLiteralExpr, NullLiteralExpr,
        ArrayLiteralExpr, PrefixExpr, BinaryExpr, CallExpr, CastExpr, MemberExpr, SubscriptExpr> data;
    boost::optional<Type> type;
};

}
