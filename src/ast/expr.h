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
class FunctionDecl;
class Module;

enum class ExprKind {
    VarExpr,
    StringLiteralExpr,
    CharacterLiteralExpr,
    IntLiteralExpr,
    FloatLiteralExpr,
    BoolLiteralExpr,
    NullLiteralExpr,
    ArrayLiteralExpr,
    TupleExpr,
    PrefixExpr,
    BinaryExpr,
    CallExpr,
    CastExpr,
    SizeofExpr,
    MemberExpr,
    SubscriptExpr,
    UnwrapExpr,
    LambdaExpr,
    IfExpr
};

class Expr {
public:
    virtual ~Expr() = 0;

    bool isVarExpr() const { return getKind() == ExprKind::VarExpr; }
    bool isStringLiteralExpr() const { return getKind() == ExprKind::StringLiteralExpr; }
    bool isCharacterLiteralExpr() const { return getKind() == ExprKind::CharacterLiteralExpr; }
    bool isIntLiteralExpr() const { return getKind() == ExprKind::IntLiteralExpr; }
    bool isFloatLiteralExpr() const { return getKind() == ExprKind::FloatLiteralExpr; }
    bool isBoolLiteralExpr() const { return getKind() == ExprKind::BoolLiteralExpr; }
    bool isNullLiteralExpr() const { return getKind() == ExprKind::NullLiteralExpr; }
    bool isArrayLiteralExpr() const { return getKind() == ExprKind::ArrayLiteralExpr; }
    bool isTupleExpr() const { return getKind() == ExprKind::TupleExpr; }
    bool isPrefixExpr() const { return getKind() == ExprKind::PrefixExpr; }
    bool isBinaryExpr() const { return getKind() == ExprKind::BinaryExpr; }
    bool isCallExpr() const { return getKind() == ExprKind::CallExpr; }
    bool isCastExpr() const { return getKind() == ExprKind::CastExpr; }
    bool isSizeofExpr() const { return getKind() == ExprKind::SizeofExpr; }
    bool isMemberExpr() const { return getKind() == ExprKind::MemberExpr; }
    bool isSubscriptExpr() const { return getKind() == ExprKind::SubscriptExpr; }
    bool isUnwrapExpr() const { return getKind() == ExprKind::UnwrapExpr; }
    bool isLambdaExpr() const { return getKind() == ExprKind::LambdaExpr; }
    bool isIfExpr() const { return getKind() == ExprKind::IfExpr; }

    ExprKind getKind() const { return kind; }
    bool hasType() const { return type.get() != nullptr; }
    Type getType() const { ASSERT(type); return type; }
    void setType(Type type) { ASSERT(type); this->type = type; }
    bool isLvalue() const;
    bool isRvalue() const { return !isLvalue(); }
    SourceLocation getLocation() const { return location; }
    void setMoved(bool moved);
    std::unique_ptr<Expr> instantiate(const llvm::StringMap<Type>& genericArgs) const;

protected:
    Expr(ExprKind kind, SourceLocation location) : kind(kind), type(nullptr), location(location) {}

private:
    const ExprKind kind;
    Type type;

public:
    const SourceLocation location;
};

inline Expr::~Expr() {}

class VarExpr : public Expr {
public:
    VarExpr(std::string&& identifier, SourceLocation location)
    : Expr(ExprKind::VarExpr, location), decl(nullptr), identifier(std::move(identifier)) {}
    Decl* getDecl() const { return decl; }
    void setDecl(Decl* newDecl) { decl = newDecl; }
    llvm::StringRef getIdentifier() const { return identifier; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::VarExpr; }

private:
    Decl* decl;
    std::string identifier;
};

class StringLiteralExpr : public Expr {
public:
    StringLiteralExpr(std::string&& value, SourceLocation location)
    : Expr(ExprKind::StringLiteralExpr, location), value(std::move(value)) {}
    llvm::StringRef getValue() const { return value; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::StringLiteralExpr; }

private:
    std::string value;
};

class CharacterLiteralExpr : public Expr {
public:
    CharacterLiteralExpr(char value, SourceLocation location)
    : Expr(ExprKind::CharacterLiteralExpr, location), value(value) {}
    char getValue() const { return value; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::CharacterLiteralExpr; }

private:
    char value;
};

class IntLiteralExpr : public Expr {
public:
    IntLiteralExpr(int64_t value, SourceLocation location)
    : Expr(ExprKind::IntLiteralExpr, location), value(value) {}
    int64_t getValue() const { return value; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::IntLiteralExpr; }

private:
    int64_t value;
};

class FloatLiteralExpr : public Expr {
public:
    FloatLiteralExpr(long double value, SourceLocation location)
    : Expr(ExprKind::FloatLiteralExpr, location), value(value) {}
    long double getValue() const { return value; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::FloatLiteralExpr; }

private:
    long double value;
};

class BoolLiteralExpr : public Expr {
public:
    BoolLiteralExpr(bool value, SourceLocation location)
    : Expr(ExprKind::BoolLiteralExpr, location), value(value) {}
    bool getValue() const { return value; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::BoolLiteralExpr; }

private:
    bool value;
};

class NullLiteralExpr : public Expr {
public:
    NullLiteralExpr(SourceLocation location)
    : Expr(ExprKind::NullLiteralExpr, location) {}
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::NullLiteralExpr; }
};

class ArrayLiteralExpr : public Expr {
public:
    ArrayLiteralExpr(std::vector<std::unique_ptr<Expr>>&& elements, SourceLocation location)
    : Expr(ExprKind::ArrayLiteralExpr, location), elements(std::move(elements)) {}
    llvm::ArrayRef<std::unique_ptr<Expr>> getElements() const { return elements; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::ArrayLiteralExpr; }

private:
    std::vector<std::unique_ptr<Expr>> elements;
};

class TupleExpr : public Expr {
public:
    TupleExpr(std::vector<std::unique_ptr<Expr>>&& elements, SourceLocation location)
    : Expr(ExprKind::TupleExpr, location), elements(std::move(elements)) {}
    llvm::ArrayRef<std::unique_ptr<Expr>> getElements() const { return elements; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::TupleExpr; }

private:
    std::vector<std::unique_ptr<Expr>> elements;
};

class Argument {
public:
    Argument(std::string&& name, std::shared_ptr<Expr>&& value,
             SourceLocation location = SourceLocation::invalid())
    : name(std::move(name)), value(std::move(value)),
      location(location.isValid() ? location : this->value->getLocation()) {}
    llvm::StringRef getName() const { return name; }
    Expr* getValue() { return value.get(); }
    const Expr* getValue() const { return value.get(); }
    SourceLocation getLocation() const { return location; }

private:
    std::string name; // Empty if no name specified.
    std::shared_ptr<Expr> value;
    SourceLocation location;
};

class CallExpr : public Expr {
public:
    CallExpr(std::unique_ptr<Expr> callee, std::vector<Argument>&& args,
             std::vector<Type>&& genericArgs, SourceLocation location)
    : Expr(ExprKind::CallExpr, location), callee(std::move(callee)), args(std::move(args)),
      genericArgs(std::move(genericArgs)), calleeDecl(nullptr) {}
    bool callsNamedFunction() const { return callee->isVarExpr() || callee->isMemberExpr(); }
    llvm::StringRef getFunctionName() const;
    std::string getMangledFunctionName() const;
    bool isMethodCall() const { return callee->isMemberExpr(); }
    bool isBuiltinConversion() const { return Type::isBuiltinScalar(getFunctionName()); }
    const Expr* getReceiver() const;
    Expr* getReceiver();
    Type getReceiverType() const { return receiverType; }
    void setReceiverType(Type type) { receiverType = type; }
    Decl* getCalleeDecl() const { return calleeDecl; }
    void setCalleeDecl(Decl* callee) { ASSERT(callee); calleeDecl = callee; }
    const Expr& getCallee() const { return *callee; }
    Expr& getCallee() { return *callee; }
    llvm::ArrayRef<Argument> getArgs() const { return args; }
    llvm::MutableArrayRef<Argument> getArgs() { return args; }
    llvm::ArrayRef<Type> getGenericArgs() const { return genericArgs; }
    void setGenericArgs(std::vector<Type>&& types) { genericArgs = std::move(types); }
    static bool classof(const Expr* e) {
        switch (e->getKind()) {
            case ExprKind::CallExpr:
            case ExprKind::PrefixExpr:
            case ExprKind::BinaryExpr:
            case ExprKind::SubscriptExpr:
                return true;
            default:
                return false;
        }
    }

protected:
    CallExpr(ExprKind kind, std::unique_ptr<Expr> callee, std::vector<Argument>&& args,
             SourceLocation location)
    : Expr(kind, location), callee(std::move(callee)), args(std::move(args)), calleeDecl(nullptr) {}

private:
    std::unique_ptr<Expr> callee;
    std::vector<Argument> args;
    std::vector<Type> genericArgs;
    Type receiverType;
    Decl* calleeDecl;
};

inline std::vector<Argument> addArg(std::vector<Argument>&& args, std::shared_ptr<Expr>&& arg) {
    args.push_back({ "", std::move(arg), SourceLocation::invalid() });
    return std::move(args);
}

class PrefixExpr : public CallExpr {
public:
    PrefixExpr(PrefixOperator op, std::unique_ptr<Expr> operand, SourceLocation location)
    : CallExpr(ExprKind::PrefixExpr,
               llvm::make_unique<VarExpr>(toString(op.getKind()), location),
               addArg({}, std::move(operand)), location), op(op) {}
    PrefixOperator getOperator() const { return op; }
    Expr& getOperand() { return *getArgs()[0].getValue(); }
    const Expr& getOperand() const { return *getArgs()[0].getValue(); }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::PrefixExpr; }

private:
    PrefixOperator op;
};

class BinaryExpr : public CallExpr {
public:
    BinaryExpr(BinaryOperator op, std::shared_ptr<Expr>&& left, std::unique_ptr<Expr> right,
               SourceLocation location)
    : CallExpr(ExprKind::BinaryExpr, llvm::make_unique<VarExpr>(op.getFunctionName(), location),
               addArg(addArg({}, std::move(left)), std::move(right)), location), op(op) {}
    BinaryOperator getOperator() const { return op; }
    const Expr& getLHS() const { return *getArgs()[0].getValue(); }
    const Expr& getRHS() const { return *getArgs()[1].getValue(); }
    Expr& getLHS() { return *getArgs()[0].getValue(); }
    Expr& getRHS() { return *getArgs()[1].getValue(); }
    bool isBuiltinOp() const;
    Decl* getCalleeDecl() const { return calleeDecl; }
    void setCalleeDecl(Decl* decl) { calleeDecl = decl; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::BinaryExpr; }

private:
    BinaryOperator op;
    Decl* calleeDecl;
};

/// A type cast expression using the 'cast' keyword, e.g. 'cast<type>(expr)'.
class CastExpr : public Expr {
public:
    CastExpr(Type type, std::unique_ptr<Expr> expr, SourceLocation location)
    : Expr(ExprKind::CastExpr, location), type(type), expr(std::move(expr)) {}
    Type getTargetType() const { return type; }
    Expr& getExpr() const { return *expr; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::CastExpr; }

private:
    Type type;
    std::unique_ptr<Expr> expr;
};

/// A compile-time expression returning the size of a given type in bytes, e.g. 'sizeof(int)'.
class SizeofExpr : public Expr {
public:
    SizeofExpr(Type type, SourceLocation location)
    : Expr(ExprKind::SizeofExpr, location), type(type) {}
    Type getType() const { return type; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::SizeofExpr; }

private:
    Type type;
};

/// A member access expression using the dot syntax, such as 'a.b'.
class MemberExpr : public Expr {
public:
    MemberExpr(std::unique_ptr<Expr> base, std::string&& member, SourceLocation location)
    : Expr(ExprKind::MemberExpr, location), base(std::move(base)), member(std::move(member)) {}
    const Expr* getBaseExpr() const { return base.get(); }
    Expr* getBaseExpr() { return base.get(); }
    llvm::StringRef getMemberName() const { return member; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::MemberExpr; }

private:
    std::unique_ptr<Expr> base;
    std::string member;
};

/// An array element access expression using the element's index in brackets, e.g. 'array[index]'.
class SubscriptExpr : public CallExpr {
public:
    SubscriptExpr(std::unique_ptr<Expr> array, std::unique_ptr<Expr> index, SourceLocation location)
    : CallExpr(ExprKind::SubscriptExpr, llvm::make_unique<MemberExpr>(std::move(array), "[]", location),
               { Argument("", std::move(index)) }, location) {}
    const Expr* getBaseExpr() const { return getReceiver(); }
    const Expr* getIndexExpr() const { return getArgs()[0].getValue(); }
    Expr* getBaseExpr() { return getReceiver(); }
    Expr* getIndexExpr() { return getArgs()[0].getValue(); }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::SubscriptExpr; }
};

/// A postfix expression that unwraps an optional (nullable) value, yielding the value wrapped by
/// the optional, for example 'foo!'. If the optional is null, the operation triggers an assertion
/// error (by default), or causes undefined behavior (in unchecked mode).
class UnwrapExpr : public Expr {
public:
    UnwrapExpr(std::unique_ptr<Expr> operand, SourceLocation location)
    : Expr(ExprKind::UnwrapExpr, location), operand(std::move(operand)) {}
    Expr& getOperand() const { return *operand; }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::UnwrapExpr; }

private:
    std::unique_ptr<Expr> operand;
};

class LambdaExpr : public Expr {
public:
    LambdaExpr(std::vector<ParamDecl>&& params, std::unique_ptr<Expr> body,
               SourceLocation location)
    : Expr(ExprKind::LambdaExpr, location), params(std::move(params)),
      body(std::move(body)) {}
    llvm::ArrayRef<ParamDecl> getParams() const { return params; }
    llvm::MutableArrayRef<ParamDecl> getParams() { return params; }
    Expr* getBody() const { return body.get(); }
    std::unique_ptr<FunctionDecl> lower(Module& module) const;
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::LambdaExpr; }

private:
    std::vector<ParamDecl> params;
    std::unique_ptr<Expr> body;
};

class IfExpr : public Expr {
public:
    IfExpr(std::unique_ptr<Expr> condition, std::unique_ptr<Expr> thenExpr,
           std::unique_ptr<Expr> elseExpr, SourceLocation location)
    : Expr(ExprKind::IfExpr, location), condition(std::move(condition)),
      thenExpr(std::move(thenExpr)), elseExpr(std::move(elseExpr)) {}
    Expr* getCondition() { return condition.get(); }
    Expr* getThenExpr() { return thenExpr.get(); }
    Expr* getElseExpr() { return elseExpr.get(); }
    const Expr* getCondition() const { return condition.get(); }
    const Expr* getThenExpr() const { return thenExpr.get(); }
    const Expr* getElseExpr() const { return elseExpr.get(); }
    static bool classof(const Expr* e) { return e->getKind() == ExprKind::IfExpr; }

private:
    std::unique_ptr<Expr> condition;
    std::unique_ptr<Expr> thenExpr;
    std::unique_ptr<Expr> elseExpr;
};

}
