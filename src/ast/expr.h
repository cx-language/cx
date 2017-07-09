#pragma once

#include <string>
#include <memory>
#include <vector>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Casting.h>
#include "type.h"
#include "srcloc.h"
#include "token.h"

namespace delta {

class Decl;

enum class ExprKind {
    VarExpr,
    StrLiteralExpr,
    IntLiteralExpr,
    FloatLiteralExpr,
    BoolLiteralExpr,
    NullLiteralExpr,
    ArrayLiteralExpr,
    PrefixExpr,
    BinaryExpr,
    CallExpr,
    CastExpr,
    MemberExpr,
    SubscriptExpr,
    UnwrapExpr,
};

class Expr {
public:
    virtual ~Expr() = 0;

#define DEFINE_EXPR_IS_AND_GET(KIND) \
    bool is##KIND() const { return getKind() == ExprKind::KIND; } \
    class KIND& get##KIND() { return llvm::cast<class  KIND>(*this); } \
    const class KIND& get##KIND() const { return llvm::cast<class KIND>(*this); }

    DEFINE_EXPR_IS_AND_GET(VarExpr)
    DEFINE_EXPR_IS_AND_GET(StrLiteralExpr)
    DEFINE_EXPR_IS_AND_GET(IntLiteralExpr)
    DEFINE_EXPR_IS_AND_GET(FloatLiteralExpr)
    DEFINE_EXPR_IS_AND_GET(BoolLiteralExpr)
    DEFINE_EXPR_IS_AND_GET(NullLiteralExpr)
    DEFINE_EXPR_IS_AND_GET(ArrayLiteralExpr)
    DEFINE_EXPR_IS_AND_GET(PrefixExpr)
    DEFINE_EXPR_IS_AND_GET(BinaryExpr)
    DEFINE_EXPR_IS_AND_GET(CallExpr)
    DEFINE_EXPR_IS_AND_GET(CastExpr)
    DEFINE_EXPR_IS_AND_GET(MemberExpr)
    DEFINE_EXPR_IS_AND_GET(SubscriptExpr)
    DEFINE_EXPR_IS_AND_GET(UnwrapExpr)
#undef DEFINE_EXPR_IS_AND_GET

    ExprKind getKind() const { return kind; }
    Type getType() const { return type; }
    void setType(Type t) { type = t; }
    bool isLvalue() const;
    bool isRvalue() const { return !isLvalue(); }
    SrcLoc getSrcLoc() const { return srcLoc; }

protected:
    Expr(ExprKind kind, SrcLoc srcLoc) : kind(kind), type(nullptr), srcLoc(srcLoc) { }

private:
    const ExprKind kind;
    Type type;

public:
    const SrcLoc srcLoc;
};

inline Expr::~Expr() { }

class VarExpr : public Expr {
    Decl* decl;
public:
    std::string identifier;

    VarExpr(std::string&& identifier, SrcLoc srcLoc)
    : Expr(ExprKind::VarExpr, srcLoc), decl(nullptr), identifier(std::move(identifier)) { }
    Decl* getDecl() const { return decl; }
    void setDecl(Decl* newDecl) { decl = newDecl; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::VarExpr; }
};

class StrLiteralExpr : public Expr {
public:
    std::string value;

    StrLiteralExpr(std::string&& value, SrcLoc srcLoc)
    : Expr(ExprKind::StrLiteralExpr, srcLoc), value(std::move(value)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::StrLiteralExpr; }
};

class IntLiteralExpr : public Expr {
public:
    int64_t value;

    IntLiteralExpr(int64_t value, SrcLoc srcLoc)
    : Expr(ExprKind::IntLiteralExpr, srcLoc), value(value) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::IntLiteralExpr; }
};

class FloatLiteralExpr : public Expr {
public:
    long double value;

    FloatLiteralExpr(long double value, SrcLoc srcLoc)
    : Expr(ExprKind::FloatLiteralExpr, srcLoc), value(value) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::FloatLiteralExpr; }
};

class BoolLiteralExpr : public Expr {
public:
    bool value;

    BoolLiteralExpr(bool value, SrcLoc srcLoc)
    : Expr(ExprKind::BoolLiteralExpr, srcLoc), value(value) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::BoolLiteralExpr; }
};

class NullLiteralExpr : public Expr {
public:
    NullLiteralExpr(SrcLoc srcLoc)
    : Expr(ExprKind::NullLiteralExpr, srcLoc) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::NullLiteralExpr; }
};

class ArrayLiteralExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>>&& elements, SrcLoc srcLoc)
    : Expr(ExprKind::ArrayLiteralExpr, srcLoc), elements(std::move(elements)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::ArrayLiteralExpr; }
};

class Arg {
public:
    std::string name; // Empty if no name specified.
    std::unique_ptr<Expr> value;
    SrcLoc srcLoc;
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> func;
    std::vector<Arg> args;
    std::vector<Type> genericArgs;

    CallExpr(std::unique_ptr<Expr> func, std::vector<Arg>&& args,
             std::vector<Type>&& genericArgs, SrcLoc srcLoc)
    : Expr(ExprKind::CallExpr, srcLoc), func(std::move(func)), args(std::move(args)),
      genericArgs(std::move(genericArgs)) { }
    bool callsNamedFunc() const { return func->isVarExpr() || func->isMemberExpr(); }
    llvm::StringRef getFuncName() const;
    std::string getMangledFuncName() const;
    bool isMemberFuncCall() const { return func->isMemberExpr(); }
    bool isInitCall() const;
    bool isBuiltinConversion() const { return Type::isBuiltinScalar(getFuncName()); }
    Expr* getReceiver() const;
    Type getReceiverType() const { return receiverType; }
    void setReceiverType(Type type) { receiverType = type; }
    Decl* getCalleeDecl() const { return calleeDecl; }
    void setCalleeDecl(Decl* decl) { calleeDecl = decl; }
    llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }

    static bool classof(const Expr* e) { return e->getKind() == ExprKind::CallExpr; }

protected:
    CallExpr(ExprKind kind, std::unique_ptr<Expr> func, std::vector<Arg>&& args, SrcLoc srcLoc)
    : Expr(kind, srcLoc), func(std::move(func)), args(std::move(args)) { }

private:
    Type receiverType;
    Decl* calleeDecl;
};

inline std::vector<Arg> addArg(std::vector<Arg>&& args, std::unique_ptr<Expr> arg) {
    args.push_back({ "", std::move(arg), SrcLoc::invalid() });
    return std::move(args);
}

class PrefixExpr : public CallExpr {
public:
    PrefixOperator op;

    PrefixExpr(PrefixOperator op, std::unique_ptr<Expr> operand, SrcLoc srcLoc)
    : CallExpr(ExprKind::PrefixExpr,
               llvm::make_unique<VarExpr>(toString(op.kind), srcLoc),
               addArg({}, std::move(operand)), srcLoc), op(op) { }
    Expr& getOperand() const { return *args[0].value; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::PrefixExpr; }
};

class BinaryExpr : public CallExpr {
public:
    BinaryOperator op;

    BinaryExpr(BinaryOperator op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
               SrcLoc srcLoc)
    : CallExpr(ExprKind::BinaryExpr, llvm::make_unique<VarExpr>(toString(op.kind), srcLoc),
               addArg(addArg({}, std::move(left)), std::move(right)), srcLoc), op(op) { }
    Expr& getLHS() const { return *args[0].value; }
    Expr& getRHS() const { return *args[1].value; }
    bool isBuiltinOp() const {
        return getLHS().getType().isBuiltinType() && getRHS().getType().isBuiltinType();
    }
    Decl* getCalleeDecl() const { return calleeDecl; }
    void setCalleeDecl(Decl* decl) { calleeDecl = decl; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::BinaryExpr; }

private:
    Decl* calleeDecl;
};

/// A type cast expression using the 'cast' keyword, e.g. 'cast<type>(expr)'.
class CastExpr : public Expr {
public:
    Type type;
    std::unique_ptr<Expr> expr;

    CastExpr(Type type, std::unique_ptr<Expr> expr, SrcLoc srcLoc)
    : Expr(ExprKind::CastExpr, srcLoc), type(type), expr(std::move(expr)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::CastExpr; }
};

/// A member access expression using the dot syntax, such as 'a.b'.
class MemberExpr : public Expr {
public:
    std::unique_ptr<Expr> base;
    std::string member;

    MemberExpr(std::unique_ptr<Expr> base, std::string&& member, SrcLoc srcLoc)
    : Expr(ExprKind::MemberExpr, srcLoc), base(std::move(base)), member(std::move(member)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::MemberExpr; }
};

/// An array element access expression using the element's index in brackets, e.g. 'array[index]'.
class SubscriptExpr : public Expr {
public:
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;

    SubscriptExpr(std::unique_ptr<Expr> array, std::unique_ptr<Expr> index, SrcLoc srcLoc)
    : Expr(ExprKind::SubscriptExpr, srcLoc), array(std::move(array)), index(std::move(index)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::SubscriptExpr; }
};

/// A postfix expression that unwraps a non-null pointer, yielding a reference to its
/// pointee, e.g. 'foo!'. If the pointer is null, the operation triggers an assertion
/// error (by default), or causes undefined behavior (in unchecked mode).
class UnwrapExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;

    UnwrapExpr(std::unique_ptr<Expr> operand, SrcLoc srcLoc)
    : Expr(ExprKind::UnwrapExpr, srcLoc), operand(std::move(operand)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::UnwrapExpr; }
};

}
