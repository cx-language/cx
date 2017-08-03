#pragma once

#include <string>
#include <memory>
#include <vector>
#include <llvm/ADT/STLExtras.h>
#include <llvm/Support/Casting.h>
#include "type.h"
#include "location.h"
#include "token.h"

namespace delta {

class Decl;

enum class ExprKind {
    VarExpr,
    StringLiteralExpr,
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

    bool isVarExpr() const { return getKind() == ExprKind::VarExpr; }
    bool isStringLiteralExpr() const { return getKind() == ExprKind::StringLiteralExpr; }
    bool isIntLiteralExpr() const { return getKind() == ExprKind::IntLiteralExpr; }
    bool isFloatLiteralExpr() const { return getKind() == ExprKind::FloatLiteralExpr; }
    bool isBoolLiteralExpr() const { return getKind() == ExprKind::BoolLiteralExpr; }
    bool isNullLiteralExpr() const { return getKind() == ExprKind::NullLiteralExpr; }
    bool isArrayLiteralExpr() const { return getKind() == ExprKind::ArrayLiteralExpr; }
    bool isPrefixExpr() const { return getKind() == ExprKind::PrefixExpr; }
    bool isBinaryExpr() const { return getKind() == ExprKind::BinaryExpr; }
    bool isCallExpr() const { return getKind() == ExprKind::CallExpr; }
    bool isCastExpr() const { return getKind() == ExprKind::CastExpr; }
    bool isMemberExpr() const { return getKind() == ExprKind::MemberExpr; }
    bool isSubscriptExpr() const { return getKind() == ExprKind::SubscriptExpr; }
    bool isUnwrapExpr() const { return getKind() == ExprKind::UnwrapExpr; }

    ExprKind getKind() const { return kind; }
    Type getType() const { return type; }
    void setType(Type t) { type = t; }
    bool isLvalue() const;
    bool isRvalue() const { return !isLvalue(); }
    SourceLocation getLocation() const { return location; }

protected:
    Expr(ExprKind kind, SourceLocation location) : kind(kind), type(nullptr), location(location) { }

private:
    const ExprKind kind;
    Type type;

public:
    const SourceLocation location;
};

inline Expr::~Expr() { }

class VarExpr : public Expr {
    Decl* decl;
public:
    std::string identifier;

    VarExpr(std::string&& identifier, SourceLocation location)
    : Expr(ExprKind::VarExpr, location), decl(nullptr), identifier(std::move(identifier)) { }
    Decl* getDecl() const { return decl; }
    void setDecl(Decl* newDecl) { decl = newDecl; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::VarExpr; }
};

class StringLiteralExpr : public Expr {
public:
    std::string value;

    StringLiteralExpr(std::string&& value, SourceLocation location)
    : Expr(ExprKind::StringLiteralExpr, location), value(std::move(value)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::StringLiteralExpr; }
};

class IntLiteralExpr : public Expr {
public:
    int64_t value;

    IntLiteralExpr(int64_t value, SourceLocation location)
    : Expr(ExprKind::IntLiteralExpr, location), value(value) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::IntLiteralExpr; }
};

class FloatLiteralExpr : public Expr {
public:
    long double value;

    FloatLiteralExpr(long double value, SourceLocation location)
    : Expr(ExprKind::FloatLiteralExpr, location), value(value) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::FloatLiteralExpr; }
};

class BoolLiteralExpr : public Expr {
public:
    bool value;

    BoolLiteralExpr(bool value, SourceLocation location)
    : Expr(ExprKind::BoolLiteralExpr, location), value(value) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::BoolLiteralExpr; }
};

class NullLiteralExpr : public Expr {
public:
    NullLiteralExpr(SourceLocation location)
    : Expr(ExprKind::NullLiteralExpr, location) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::NullLiteralExpr; }
};

class ArrayLiteralExpr : public Expr {
public:
    std::vector<std::unique_ptr<Expr>> elements;

    ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>>&& elements, SourceLocation location)
    : Expr(ExprKind::ArrayLiteralExpr, location), elements(std::move(elements)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::ArrayLiteralExpr; }
};
    class TypeChecker;

class Argument {
public:
    std::string name; // Empty if no name specified.
    std::unique_ptr<Expr> value;
    SourceLocation location;

    Argument(std::string&& name, std::unique_ptr<Expr> value, SourceLocation location)
    : name(std::move(name)), value(std::move(value)), location(location) { }
    llvm::StringRef getName() const { return name; }
    Expr* getValue() const { return value.get(); }
    SourceLocation getLocation() const { return location; }
};

class CallExpr : public Expr {
public:
    std::unique_ptr<Expr> callee;
    std::vector<Argument> args;
    std::vector<Type> genericArgs;

    CallExpr(std::unique_ptr<Expr> callee, std::vector<Argument>&& args,
             std::vector<Type>&& genericArgs, SourceLocation location)
    : Expr(ExprKind::CallExpr, location), callee(std::move(callee)), args(std::move(args)),
      genericArgs(std::move(genericArgs)) { }
    bool callsNamedFunction() const { return callee->isVarExpr() || callee->isMemberExpr(); }
    llvm::StringRef getFunctionName() const;
    std::string getMangledFunctionName() const;
    bool isMethodCall() const { return callee->isMemberExpr(); }
    bool isInitCall() const;
    bool isBuiltinConversion() const { return Type::isBuiltinScalar(getFunctionName()); }
    Expr* getReceiver() const;
    Type getReceiverType() const { return receiverType; }
    void setReceiverType(Type type) { receiverType = type; }
    Decl* getCalleeDecl() const { return calleeDecl; }
    void setCalleeDecl(Decl* decl) { calleeDecl = decl; }
    const Expr& getCallee() const { return *callee; }
    llvm::ArrayRef<Argument> getArgs() const { return args; }
    llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }
    void setGenericArgs(std::vector<Type>&& types) { genericArgs = std::move(types); }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::CallExpr; }

protected:
    CallExpr(ExprKind kind, std::unique_ptr<Expr> callee, std::vector<Argument>&& args,
             SourceLocation location)
    : Expr(kind, location), callee(std::move(callee)), args(std::move(args)) { }

private:
    Type receiverType;
    Decl* calleeDecl;
};

inline std::vector<Argument> addArg(std::vector<Argument>&& args, std::unique_ptr<Expr> arg) {
    args.push_back({ "", std::move(arg), SourceLocation::invalid() });
    return std::move(args);
}

class PrefixExpr : public CallExpr {
public:
    PrefixOperator op;

    PrefixExpr(PrefixOperator op, std::unique_ptr<Expr> operand, SourceLocation location)
    : CallExpr(ExprKind::PrefixExpr,
               llvm::make_unique<VarExpr>(toString(op.kind), location),
               addArg({}, std::move(operand)), location), op(op) { }
    Expr& getOperand() const { return *getArgs()[0].getValue(); }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::PrefixExpr; }
};

class BinaryExpr : public CallExpr {
public:
    BinaryOperator op;

    BinaryExpr(BinaryOperator op, std::unique_ptr<Expr> left, std::unique_ptr<Expr> right,
               SourceLocation location)
    : CallExpr(ExprKind::BinaryExpr, llvm::make_unique<VarExpr>(toString(op.kind), location),
               addArg(addArg({}, std::move(left)), std::move(right)), location), op(op) { }
    Expr& getLHS() const { return *getArgs()[0].getValue(); }
    Expr& getRHS() const { return *getArgs()[1].getValue(); }
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

    CastExpr(Type type, std::unique_ptr<Expr> expr, SourceLocation location)
    : Expr(ExprKind::CastExpr, location), type(type), expr(std::move(expr)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::CastExpr; }
};

/// A member access expression using the dot syntax, such as 'a.b'.
class MemberExpr : public Expr {
public:
    std::unique_ptr<Expr> base;
    std::string member;

    MemberExpr(std::unique_ptr<Expr> base, std::string&& member, SourceLocation location)
    : Expr(ExprKind::MemberExpr, location), base(std::move(base)), member(std::move(member)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::MemberExpr; }
};

/// An array element access expression using the element's index in brackets, e.g. 'array[index]'.
class SubscriptExpr : public Expr {
public:
    std::unique_ptr<Expr> array;
    std::unique_ptr<Expr> index;

    SubscriptExpr(std::unique_ptr<Expr> array, std::unique_ptr<Expr> index, SourceLocation location)
    : Expr(ExprKind::SubscriptExpr, location), array(std::move(array)), index(std::move(index)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::SubscriptExpr; }
};

/// A postfix expression that unwraps a non-null pointer, yielding a reference to its
/// pointee, e.g. 'foo!'. If the pointer is null, the operation triggers an assertion
/// error (by default), or causes undefined behavior (in unchecked mode).
class UnwrapExpr : public Expr {
public:
    std::unique_ptr<Expr> operand;

    UnwrapExpr(std::unique_ptr<Expr> operand, SourceLocation location)
    : Expr(ExprKind::UnwrapExpr, location), operand(std::move(operand)) { }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::UnwrapExpr; }
};

}
